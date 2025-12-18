#include "driver/twai.h"

// --- การกำหนดขา (Wiring Config) ---
// ขาที่ต่อกับ TJA1051
#define RX_PIN 16  // ต่อกับขา RXD ของ TJA1051
#define TX_PIN 17  // ต่อกับขา TXD ของ TJA1051

// ขาสำหรับควบคุมอุปกรณ์ (Output)
#define BRAKE_PIN 26 // ต่อกับ Relay เบรค
#define LIGHT_PIN 27 // ต่อกับ Relay ไฟ

// --- กำหนด ID คำสั่ง (ต้องตรงกับตัวส่ง) ---
#define CMD_BRAKE_ID 0x100
#define CMD_LIGHT_ID 0x101

void setup() {
  Serial.begin(115200);
  
  // ตั้งค่าขา Output
  pinMode(BRAKE_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(BRAKE_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);

  Serial.println("Initializing TWAI (CAN)...");

  // 1. ตั้งค่า Config ของ TWAI
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
  
  // 2. ตั้งค่าความเร็ว (Timing) **สำคัญมาก ต้องตรงกับตัวส่ง (500Kbps)**
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  
  // 3. ตั้งค่า Filter (รับทุก ID)
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // ติดตั้ง Driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return;
  }

  // เริ่มการทำงาน
  if (twai_start() == ESP_OK) {
    Serial.println("Driver started. Waiting for commands...");
  } else {
    Serial.println("Failed to start driver");
    return;
  }
}

void loop() {
  twai_message_t message;

  // รอรับข้อความ (Timeout 1ms เพื่อไม่ให้ loop ค้างนาน)
  if (twai_receive(&message, pdMS_TO_TICKS(1)) == ESP_OK) {
    
    // --- ส่วนประมวลผลคำสั่ง ---

    // 1. ตรวจสอบคำสั่งเบรค (ID 0x100)
    if (message.identifier == CMD_BRAKE_ID) {
      if (message.data_length_code > 0) {
        if (message.data[0] == 1) {
          digitalWrite(BRAKE_PIN, HIGH);
          Serial.println("CMD: BRAKE [ON]");
        } else {
          digitalWrite(BRAKE_PIN, LOW);
          Serial.println("CMD: BRAKE [OFF]");
        }
      }
    }

    // 2. ตรวจสอบคำสั่งไฟ (ID 0x101)
    else if (message.identifier == CMD_LIGHT_ID) {
      if (message.data_length_code > 0) {
        if (message.data[0] == 1) {
          digitalWrite(LIGHT_PIN, HIGH);
          Serial.println("CMD: LIGHT [ON]");
        } else {
          digitalWrite(LIGHT_PIN, LOW);
          Serial.println("CMD: LIGHT [OFF]");
        }
      }
    }
    
    // Debug: แสดง ID ที่ไม่รู้จัก (เผื่อเช็คว่ามีอะไรส่งมาบ้าง)
    else {
      Serial.printf("Unknown ID: 0x%X DLC: %d\n", message.identifier, message.data_length_code);
    }
  }
}
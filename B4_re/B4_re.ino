#include "driver/twai.h"

// --- Config ---
// ขาที่ต่อกับ TJA1051
#define RX_PIN 26
#define TX_PIN 27

// ID ที่จะรอรับ (ต้องตรงกับ Node 3)
#define SENSOR_MSG_ID 0x789

void setup() {
  Serial.begin(115200);
  Serial.println("Node 4 (Receiver) Initializing TWAI...");

  // 1. ตั้งค่า Config
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
  // 2. ตั้งค่าความเร็ว (500Kbps ต้องตรงกับ Node 3)
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  // 3. ตั้งค่า Filter (รับทุกอย่าง)
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install Driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return;
  }

  // Start Driver
  if (twai_start() == ESP_OK) {
    Serial.println("Driver started. Waiting for Sensor Data...");
  } else {
    Serial.println("Failed to start driver");
    return;
  }
}

void loop() {
  twai_message_t message;

  // รอรับข้อมูล (Timeout 1ms)
  if (twai_receive(&message, pdMS_TO_TICKS(1)) == ESP_OK) {
    
    // ตรวจสอบว่าเป็นข้อมูลจาก Sensor Node 3 หรือไม่
    if (message.identifier == SENSOR_MSG_ID) {
      
      // แปลงข้อมูล 2 bytes กลับเป็น int (Reassemble)
      // data[0] คือ High Byte, data[1] คือ Low Byte
      int receivedValue = (message.data[0] << 8) | message.data[1];

      Serial.print("Received from Node 3 -> Sensor Value: ");
      Serial.println(receivedValue);
      
      // ตัวอย่างการนำไปใช้: ถ้าค่ามาก ให้เปิด LED
      // if (receivedValue > 2000) { ... }
    } 
  }
}
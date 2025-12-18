#include <SPI.h>
#include <mcp2515.h>
#include "driver/twai.h"

// --- ส่วนตั้งค่า MCP2515 (Output/Control Side) ---
struct can_frame mcpMsg;
const int MCP_CS_PIN = 5;  // ขา CS ของ MCP2515
MCP2515 mcp2515(MCP_CS_PIN);

// --- ส่วนตั้งค่า TWAI/Internal CAN (Input Side: TJA1051) ---
// ขาที่ต่อกับ TJA1051 (TX/RX)
#define TWAI_RX_PIN 16  // ต่อกับขา RXD ของ TJA1051
#define TWAI_TX_PIN 17  // ต่อกับขา TXD ของ TJA1051

#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26

// กำหนด ID คำสั่ง (ตัวอย่าง)
#define CMD_BRAKE_ID 0x100
#define CMD_LIGHT_ID 0x101

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // 1. ตั้งค่า MCP2515 (ฝั่งส่งไปคุมเบรค/ไฟ)
  SPI.begin();
  mcp2515.reset();
  // ตั้งความเร็ว Bus ฝั่ง MCP2515 (เช่น 500kbps, 8MHz Crystal)
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  Serial.println("MCP2515 Initialized.");

  // 2. ตั้งค่า TWAI (ฝั่งรับจาก TJA1051)
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TWAI_TX_PIN, (gpio_num_t)TWAI_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); // ตั้งความเร็วให้ตรงกับรถ/เซนเซอร์ต้นทาง
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // ติดตั้ง Driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("TWAI Driver installed");
  } else {
    Serial.println("Failed to install TWAI driver");
    return;
  }

  // เริ่มการทำงาน TWAI
  if (twai_start() == ESP_OK) {
    Serial.println("TWAI Driver started");
  } else {
    Serial.println("Failed to start TWAI driver");
    return;
  }
}

void loop() {
  // ตัวแปรสำหรับรับค่าจาก TWAI (TJA1051)
  twai_message_t rxMsg;

  // ตรวจสอบว่ามีข้อมูลเข้ามาทาง TJA1051 หรือไม่ (Timeout 1ms)
  if (twai_receive(&rxMsg, pdMS_TO_TICKS(1)) == ESP_OK) {
    
    // แสดงผลข้อมูลที่รับมา (Debug)
    Serial.print("Received from TJA1051 (ID): 0x");
    Serial.print(rxMsg.identifier, HEX);
    Serial.print(" Data: ");
    for (int i = 0; i < rxMsg.data_length_code; i++) {
      Serial.print(rxMsg.data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // --- ส่วน Logic: ส่งต่อไปยัง MCP2515 เพื่อคุมเบรค/ไฟ ---
    
    // เตรียมข้อมูลสำหรับ MCP2515
    mcpMsg.can_id = rxMsg.identifier;
    mcpMsg.can_dlc = rxMsg.data_length_code;
    for (int i = 0; i < rxMsg.data_length_code; i++) {
      mcpMsg.data[i] = rxMsg.data[i];
    }

    // ตัวอย่าง: ถ้า ID ตรงกับคำสั่งเบรค (0x100) ให้ส่งคำสั่งออกไปทาง MCP2515
    if (rxMsg.identifier == CMD_BRAKE_ID) {
        Serial.println(">> Forwarding BRAKE Command via MCP2515...");
        mcp2515.sendMessage(&mcpMsg);
    }
    
    // ตัวอย่าง: ถ้า ID ตรงกับคำสั่งไฟ (0x101) ให้ส่งคำสั่งออกไปทาง MCP2515
    else if (rxMsg.identifier == CMD_LIGHT_ID) {
        Serial.println(">> Forwarding LIGHT Command via MCP2515...");
        mcp2515.sendMessage(&mcpMsg);
    }
    
    // หรือถ้าต้องการส่งต่อ "ทุกข้อมูล" (Gateway Mode) ให้เอา if-else ออก แล้วใช้บรรทัดนี้:
    // mcp2515.sendMessage(&mcpMsg);
  }
}
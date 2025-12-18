#include <SPI.h>
#include <mcp2515.h>


#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26

// --- ตั้งค่าขา (Pin Config) ---
// ถ้าใช้ Arduino Uno/Nano: CS = 10
// ถ้าใช้ ESP32: CS = 5 (หรือตามที่ต่อจริง)
const int SPI_CS_PIN = 10; 

// ขาที่ต่อกับ Relay หรือ MOSFET เพื่อคุมอุปกรณ์
const int BRAKE_PIN = 7;   // ต่อกับรีเลย์เบรค
const int LIGHT_PIN = 8;   // ต่อกับรีเลย์ไฟ

// สร้าง Object
struct can_frame canMsg;
MCP2515 mcp2515(SPI_CS_PIN);

// ต้องตรงกับโค้ดตัวส่ง (Sender)
#define CMD_BRAKE_ID 0x100
#define CMD_LIGHT_ID 0x101

void setup() {
  Serial.begin(115200);
  
  // ตั้งค่าขา Output
  pinMode(BRAKE_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  
  // กำหนดสถานะเริ่มต้น (ปิด)
  digitalWrite(BRAKE_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);

  // เริ่มต้น SPI และ MCP2515
  SPI.begin();
  mcp2515.reset();
  
  // *** สำคัญมาก: ความเร็วต้องตรงกับตัวส่ง (500KBPS / 8MHZ) ***
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  Serial.println("Receiver Unit Ready...");
  Serial.println("Waiting for CAN Commands...");
}

void loop() {
  // ตรวจสอบว่ามีข้อมูลเข้ามาใน MCP2515 หรือไม่
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    
    // --- รับคำสั่งควบคุมเบรค (ID: 0x100) ---
    if (canMsg.can_id == CMD_BRAKE_ID) {
      // เช็คข้อมูล Byte ที่ 0 (ถ้าส่งมาเป็น 1 คือ ON, 0 คือ OFF)
      if (canMsg.data[0] == 0x01) {
        digitalWrite(BRAKE_PIN, HIGH); // สั่ง Relay ทำงาน
        Serial.println("ACTION: BRAKE [ON]");
      } else {
        digitalWrite(BRAKE_PIN, LOW);  // สั่ง Relay หยุด
        Serial.println("ACTION: BRAKE [OFF]");
      }
    }

    // --- รับคำสั่งควบคุมไฟ (ID: 0x101) ---
    else if (canMsg.can_id == CMD_LIGHT_ID) {
      if (canMsg.data[0] == 0x01) {
        digitalWrite(LIGHT_PIN, HIGH); // เปิดไฟ
        Serial.println("ACTION: LIGHT [ON]");
      } else {
        digitalWrite(LIGHT_PIN, LOW);  // ปิดไฟ
        Serial.println("ACTION: LIGHT [OFF]");
      }
    }
  }
}
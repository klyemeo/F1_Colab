#include <SPI.h>
#include <mcp2515.h>

// --- Config ---
const int SPI_CS_PIN = 5;    // ขา CS ของ MCP2515
const int SENSOR_PIN = 32;   // ขา Analog ที่ต่อกับ Sensor (เช่น Potentiometer หรือเซนเซอร์อื่นๆ)

MCP2515 mcp2515(SPI_CS_PIN);
struct can_frame canMsg;

// กำหนด ID ประจำตัวของ Node นี้
#define SENSOR_MSG_ID 0x789

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);

  SPI.begin();
  
  // เริ่มต้น MCP2515
  mcp2515.reset();
  // ตั้งค่าความเร็ว 500KBPS / 8MHZ (ต้องตรงกันทั้งระบบ)
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  Serial.println("Node 3 (Sender) Ready via MCP2515...");
}

void loop() {
  // 1. อ่านค่าจาก Sensor (ค่า 0-4095 สำหรับ ESP32)
  int sensorValue = analogRead(SENSOR_PIN);

  // 2. เตรียมแพ็คข้อมูล (ส่งค่า int ต้องแยกเป็น 2 bytes High/Low)
  canMsg.can_id = SENSOR_MSG_ID;
  canMsg.can_dlc = 2; // ส่งข้อมูล 2 bytes
  canMsg.data[0] = (sensorValue >> 8) & 0xFF; // High Byte
  canMsg.data[1] = sensorValue & 0xFF;        // Low Byte

  // 3. ส่งข้อมูลออกไป
  if (mcp2515.sendMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print("Sent Value: ");
    Serial.println(sensorValue);
  } else {
    Serial.println("Error Sending Message...");
  }

  delay(100); // ส่งทุกๆ 100ms
}
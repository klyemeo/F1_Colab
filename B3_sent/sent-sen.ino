#include <SPI.h>
#include <mcp2515.h>

#define CAN_CS 5
MCP2515 mcp2515(CAN_CS);

struct can_frame canMsg;

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, CAN_CS);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  Serial.println("MCP2515 Sender Ready");
}

void loop() {
  int sensorValue = analogRead(34);   // ตัวอย่างค่า sensor

  canMsg.can_id  = 0x789;
  canMsg.can_dlc = 2;
  canMsg.data[0] = highByte(sensorValue);
  canMsg.data[1] = lowByte(sensorValue);

  mcp2515.sendMessage(&canMsg);

  Serial.print("Send: ");
  Serial.println(sensorValue);

  delay(100);
}

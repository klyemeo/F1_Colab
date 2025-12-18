#include <SPI.h>
#include <mcp2515.h>

// Pins for Node 1
const int CS_PIN = 5;
MCP2515 mcp2515(CS_PIN);

struct can_frame canMsgSend;
struct can_frame canMsgRecv;

void setup() {
  Serial.begin(115200);
  SPI.begin();
     
  mcp2515.reset();
  // Ensure you match your module's crystal (8MHz or 16MHz)
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); 
  mcp2515.setNormalMode();
  
  Serial.println("Node 1 (MCP2515) Ready");
}

void loop() {
  // 1. Send Sensor Data
  static uint32_t lastSend = 0;
  if (millis() - lastSend > 1000) {
    float sensorValue = 12.34; // Placeholder for sensor logic
    
    canMsgSend.can_id  = 0x101; // ID of this node
    canMsgSend.can_dlc = 4;
    memcpy(canMsgSend.data, &sensorValue, 4);
    
    if (mcp2515.sendMessage(&canMsgSend) == MCP2515::ERROR_OK) {
      Serial.println("Sent: 0x101");
    }
    lastSend = millis();
  }

  // 2. Receive Data
  if (mcp2515.readMessage(&canMsgRecv) == MCP2515::ERROR_OK) {
    if (canMsgRecv.can_id == 0x102) { // ID of partner node
      float receivedVal;
      memcpy(&receivedVal, canMsgRecv.data, 4);
      Serial.printf("Received from Node 2: %.2f\n", receivedVal);
    }
  }
}

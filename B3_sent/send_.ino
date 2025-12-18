#include <SPI.h>
#include <mcp_can.h>

const int SPI_CS_PIN = 5;
MCP_CAN CAN(SPI_CS_PIN); 

void setup() {
  Serial.begin(115200);
  // Initialize MCP2515 at 500kbps and 8MHz (check your crystal on the module!)
  while (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)) {
    Serial.println("MCP2515 Init Failed, Retrying...");
    delay(100);
  }
  CAN.setMode(MCP_NORMAL);
  Serial.println("MCP2515 Initialized");
}

void loop() {
  // 1. Send Sensor Data
  static uint32_t lastSend = 0;
  if (millis() - lastSend > 1000) {
    float sensorVal = 25.5; // Replace with your sensor read
    byte data[4];
    memcpy(data, &sensorVal, 4);
    CAN.sendMsgBuf(0x101, 0, 4, data); 
    lastSend = millis();
  }

  // 2. Receive Data
  long unsigned int rxId;
  unsigned char len = 0;
  unsigned char rxBuf[8];

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&rxId, &len, rxBuf);
    if (rxId == 0x102) {
      float receivedVal;
      memcpy(&receivedVal, rxBuf, 4);
      Serial.printf("Received from Node 2: %.2f\n", receivedVal);
    }
  }
}

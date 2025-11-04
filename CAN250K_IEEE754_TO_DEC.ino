#include <SPI.h>
#include <mcp2515.h>

struct  can_frame canMsg;
MCP2515 mcp2515(10);





void setup() 
{
  Serial.begin(115200); 
  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);            
  mcp2515.setNormalMode();


  



}

void loop() 
{
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) 
  {
    if (canMsg.can_id == 0x89A10003)
    {

     uint32_t ieeeHex1 = ( (uint32_t)canMsg.data[0] << 24 ) |
                     ( (uint32_t)canMsg.data[1] << 16 ) |
                     ( (uint32_t)canMsg.data[2] << 8  ) |
                     ( (uint32_t)canMsg.data[3] );


      uint32_t ieeeHex2 = ( (uint32_t)canMsg.data[4] << 24 ) |
                     ( (uint32_t)canMsg.data[5] << 16 ) |
                     ( (uint32_t)canMsg.data[6] << 8  ) |
                     ( (uint32_t)canMsg.data[7] );

      float decimalValue1 = hexToFloat(ieeeHex1);
      float decimalValue2 = hexToFloat(ieeeHex2);

      Serial.print(" -> CURRENT: ");
      Serial.println(decimalValue1, 2);
      Serial.print(" -> VOLTAGE: ");
      Serial.println(decimalValue2, 2);

      delay (150);


    }
  } 
}


float hexToFloat(uint32_t hexValue) {
    float floatValue;
    memcpy(&floatValue, &hexValue, sizeof(float)); // Convert hex to float
    return floatValue;
}
#include "CAN_config.h"
#include "ESP32CAN.h"

// REQUIRE:
// ESP32 by Espressif
// 2.0.17

// CAN1: TJA1050 parameters
#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26
CAN_device_t CAN_cfg;            // CAN Config
const int    rx_queue_size = 10;   // Receive Queue size

// Unused variables from your original code (for sending messages)
static unsigned long previousMillis50   = 0;
static unsigned long previousMillis500  = 0;
static unsigned long previousMillis1000 = 0;

/**
 * @brief Helper function from your second code to convert 4 bytes (as a uint32_t) into a float.
 */
float hexToFloat(uint32_t hexValue) {
    float floatValue;
    memcpy(&floatValue, &hexValue, sizeof(float)); // Convert hex to float
    return floatValue;
}

void setup() {
  Serial.begin(115200);
  Serial.println("WELCOME USB CAN-MESSAGE MONITORING (with Parser)");
  Serial.println("by");
  Serial.println("DR.ANUCHA PROMWUNGKWA");
  Serial.println("USB SERIAL: 115200 bps");

  // --- CONFIGURATION ---
  // Both of your code examples used 250kbps.
  // I have fixed the print statement to match the configuration.
  CAN_cfg.speed = CAN_SPEED_250KBPS;
  Serial.println("CAN SPEED: 250 kbps"); // <-- Corrected to match config
  
  CAN_cfg.tx_pin_id = CAN_TX_PIN;
  CAN_cfg.rx_pin_id = CAN_RX_PIN;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  
  // Init CAN Module
  ESP32Can.CANInit();
}

void loop() {

  // CAN1: rx frame
  CAN_frame_t rx_frame;

  // Receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
  {
    // --- LOGIC FROM 2ND CODE ---
    // Check if this is the specific message ID we want to parse
    if (rx_frame.MsgID == 0x89A10003) 
    {
      // Check that the data length is correct (float + float = 8 bytes)
      if (rx_frame.FIR.B.DLC == 8) 
      {
        // Unpack the 8 bytes into two 32-bit values
        uint32_t ieeeHex1 = ( (uint32_t)rx_frame.data.u8[0] << 24 ) |
                            ( (uint32_t)rx_frame.data.u8[1] << 16 ) |
                            ( (uint32_t)rx_frame.data.u8[2] << 8  ) |
                            ( (uint32_t)rx_frame.data.u8[3] );

        uint32_t ieeeHex2 = ( (uint32_t)rx_frame.data.u8[4] << 24 ) |
                            ( (uint32_t)rx_frame.data.u8[5] << 16 ) |
                            ( (uint32_t)rx_frame.data.u8[6] << 8  ) |
                            ( (uint32_t)rx_frame.data.u8[7] );

        // Convert the 32-bit values into floats
        float decimalValue1 = hexToFloat(ieeeHex1);
        float decimalValue2 = hexToFloat(ieeeHex2);

        // Print the decoded values
        Serial.print(" -> CURRENT: ");
        Serial.println(decimalValue1, 2);
        Serial.print(" -> VOLTAGE: ");
        Serial.println(decimalValue2, 2);
      }
      else 
      {
        // The ID matched, but the data length was wrong.
        printf("WARN: Received 0x%08X with unexpected DLC %d\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
      }
    }
    // --- ORIGINAL LOGIC FROM 1ST CODE ---
    // If it's NOT 0x89A10003, print the raw message
    else 
    {
      if (rx_frame.FIR.B.FF == CAN_frame_std)
      {
        printf("New standard frame");
      }
      else
      {
        printf("New extended frame");
      }

      if (rx_frame.FIR.B.RTR == CAN_RTR)
      {
        printf(" RTR from 0x%08X, DLC %d\r\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
      }
      else
      {
        printf(" from 0x%08X, DLC %d, Data ", rx_frame.MsgID, rx_frame.FIR.B.DLC);
        for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
        {
          printf("0x%02X ", rx_frame.data.u8[i]);
        }
        printf("\n");
      }
    }
  }
}

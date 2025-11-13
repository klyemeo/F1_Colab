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
  CAN_frame_t rx_frame;

  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

    switch (rx_frame.MsgID) {

      case 0x09A10000: // Key event trigger
        Serial.println("[EVENT] Key event triggered (ID: 0x09A10000)");
        break;

      case 0x09A10001: // Max charging capability
      case 0x09A10002: { // Max discharging capability
        const char* label = (rx_frame.MsgID == 0x09A10001) ? "[CHARGE CAPABILITY]" : "[DISCHARGE CAPABILITY]";

        if (rx_frame.FIR.B.DLC == 8) {
          uint32_t currentHex = ((uint32_t)rx_frame.data.u8[0] << 24) |
                                ((uint32_t)rx_frame.data.u8[1] << 16) |
                                ((uint32_t)rx_frame.data.u8[2] << 8) |
                                ((uint32_t)rx_frame.data.u8[3]);

          uint32_t voltageHex = ((uint32_t)rx_frame.data.u8[4] << 24) |
                                ((uint32_t)rx_frame.data.u8[5] << 16) |
                                ((uint32_t)rx_frame.data.u8[6] << 8) |
                                ((uint32_t)rx_frame.data.u8[7]);

          float current = hexToFloat(currentHex); // หน่วย A
          float voltage = hexToFloat(voltageHex); // หน่วย V

          Serial.println(label);
          Serial.print(" -> Max Current (A): ");
          Serial.println(current, 2);
          Serial.print(" -> Voltage Limit (V): ");
          Serial.println(voltage, 2);

          if (current <= 0.0f)
            Serial.println(" ⚠️ Charging/Discharging is DISABLED!");
        }
        // break;
      }

      case 0x09A10003: { // Real-time power info
        if (rx_frame.FIR.B.DLC == 8) {
          uint32_t currentHex = ((uint32_t)rx_frame.data.u8[0] << 24) |
                                ((uint32_t)rx_frame.data.u8[1] << 16) |
                                ((uint32_t)rx_frame.data.u8[2] << 8) |
                                ((uint32_t)rx_frame.data.u8[3]);

          uint32_t voltageHex = ((uint32_t)rx_frame.data.u8[4] << 24) |
                                ((uint32_t)rx_frame.data.u8[5] << 16) |
                                ((uint32_t)rx_frame.data.u8[6] << 8) |
                                ((uint32_t)rx_frame.data.u8[7]);

          float current = hexToFloat(currentHex);
          float voltage = hexToFloat(voltageHex);

          Serial.println("🔋[REAL-TIME POWER INFO]");
          Serial.print(" -> Current (A): ");
          Serial.println(current, 2);
          Serial.print(" -> Voltage (V): ");
          Serial.println(voltage, 2);
        }
        break;
      }

      case 0x09A10004: { // Battery Pack Status
        if (rx_frame.FIR.B.DLC == 8) {
          uint16_t socStatus = ((uint16_t)rx_frame.data.u8[0] << 8) |
                               (uint16_t)rx_frame.data.u8[1];
          uint16_t socRaw = socStatus & 0x03FF;
          float socPercent = socRaw / 10.0;
          bool relayStatus = (socStatus >> 15) & 0x01;

          uint16_t highestCell_mV = ((uint16_t)rx_frame.data.u8[2] << 8) |
                                     (uint16_t)rx_frame.data.u8[3];
          uint16_t lowestCell_mV = ((uint16_t)rx_frame.data.u8[4] << 8) |
                                    (uint16_t)rx_frame.data.u8[5];
          float highestCell_V = highestCell_mV / 1000.0;
          float lowestCell_V  = lowestCell_mV / 1000.0;

          int8_t tempHigh = (int8_t)rx_frame.data.u8[6];
          int8_t tempLow  = (int8_t)rx_frame.data.u8[7];

          Serial.println("[BATTERY PACK INFO]");
          Serial.print(" -> SOC (%): ");
          Serial.println(socPercent, 1);
          Serial.print(" -> Relay Status: ");
          Serial.println(relayStatus ? "ON" : "OFF");
          Serial.print(" -> Highest Cell Voltage (V): ");
          Serial.println(highestCell_V, 2);
          Serial.print(" -> Lowest Cell Voltage (V): ");
          Serial.println(lowestCell_V, 2);
          Serial.print(" -> Highest Cell Temp (°C): ");
          Serial.println(tempHigh);
          Serial.print(" -> Lowest Cell Temp (°C): ");
          Serial.println(tempLow);
        }
        break;
      }

      case 0x09A10005: { // Controller Status
        if (rx_frame.FIR.B.DLC == 8) {
          uint8_t fault = rx_frame.data.u8[0];
          uint8_t warning = rx_frame.data.u8[1];

          int8_t bmcTemp  = (int8_t)rx_frame.data.u8[2];
          int8_t bicHigh  = (int8_t)rx_frame.data.u8[3];
          int8_t bicLow   = (int8_t)rx_frame.data.u8[4];

          uint16_t timeRaw = ((uint16_t)rx_frame.data.u8[5] << 8) |
                              rx_frame.data.u8[6];
          bool isCharge = (timeRaw >> 15) & 0x01;
          bool hasAccuracy = (timeRaw >> 14) & 0x01;
          uint16_t minutes = timeRaw & 0x3FFF;

          uint8_t voltRaw = rx_frame.data.u8[7];
          float powerVolt = (voltRaw * 0.1f) + 6.5f;

          Serial.println("[CONTROLLER STATUS]");
          Serial.print(" -> Fault Flags: 0x");
          Serial.println(fault, HEX);
          Serial.print(" -> Warning Flags: 0x");
          Serial.println(warning, HEX);

          Serial.print(" -> BMC Temp (°C): ");
          Serial.println(bmcTemp);
          Serial.print(" -> Highest BIC Temp (°C): ");
          Serial.println(bicHigh);
          Serial.print(" -> Lowest BIC Temp (°C): ");
          Serial.println(bicLow);

          Serial.print(" -> Estimated Time: ");
          Serial.print(minutes);
          Serial.print(" min (");
          Serial.print(isCharge ? "Charging" : "Discharging");
          Serial.print(", Accuracy: ");
          Serial.print(hasAccuracy ? "High" : "Low");
          Serial.println(")");

          Serial.print(" -> 12V Supply Voltage (V): ");
          Serial.println(powerVolt, 2);

          if (powerVolt > 30.0)
            Serial.println(" ⚠️ Voltage > 30V");
          else if (powerVolt < 8.0)
            Serial.println(" ⚠️ Voltage < 8V");
        }
        break;
      }

      default:
        Serial.print("Unknown CAN ID: 0x");
        Serial.println(rx_frame.MsgID, HEX);
        break;
    }
  }
}

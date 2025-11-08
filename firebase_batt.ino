#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "CAN_config.h"
#include "ESP32CAN.h"

// ------------------------- กำหนดค่าระบบ ----------------------------
#define WIFI_SSID "CMUF19"
#define WIFI_PASSWORD "ilovef19"
#define FIREBASE_HOST "test-data-f19-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "HbMfY46ykieKqMFzv4Kdl7vyim5ByDn4aOBr88nm"
#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

CAN_device_t CAN_cfg;
const int rx_queue_size = 10;

// -------------------------------------------------------------------
// แปลงข้อมูลจาก 4 byte (uint32_t) ให้เป็น float
float hexToFloat(uint32_t hexValue) {
  float floatValue;
  memcpy(&floatValue, &hexValue, sizeof(float));
  return floatValue;
}

// ----------------------------- SETUP -------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("== ESP32: CAN + Firebase ==");
  Serial.println("กำลังเริ่มต้น...");

  // ตั้งค่า CAN Bus
  CAN_cfg.speed = CAN_SPEED_250KBPS;
  CAN_cfg.tx_pin_id = CAN_TX_PIN;
  CAN_cfg.rx_pin_id = CAN_RX_PIN;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  ESP32Can.CANInit();
  Serial.println("CAN Bus พร้อมใช้งาน (250kbps)");

  // เชื่อมต่อ Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("กำลังเชื่อมต่อ Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nเชื่อมต่อแล้ว! IP คือ: " + WiFi.localIP().toString());

  // ตั้งค่า Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("เชื่อมต่อ Firebase สำเร็จ");
}

// ------------------------------ LOOP --------------------------------
void loop() {
  CAN_frame_t rx_frame;

  // รับข้อมูลจาก CAN ถ้ามี
    if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

    String path = "/ESP32_Data/Location1";

    switch (rx_frame.MsgID) {

      // ✅ Real-time Power Info
      case 0x09A10003: {
        if (rx_frame.FIR.B.DLC == 8) {
          uint32_t rawCurrent = ((uint32_t)rx_frame.data.u8[0] << 24) |
                                ((uint32_t)rx_frame.data.u8[1] << 16) |
                                ((uint32_t)rx_frame.data.u8[2] << 8) |
                                ((uint32_t)rx_frame.data.u8[3]);
          uint32_t rawVoltage = ((uint32_t)rx_frame.data.u8[4] << 24) |
                                ((uint32_t)rx_frame.data.u8[5] << 16) |
                                ((uint32_t)rx_frame.data.u8[6] << 8) |
                                ((uint32_t)rx_frame.data.u8[7]);

          float current = hexToFloat(rawCurrent);
          float voltage = hexToFloat(rawVoltage);

          Serial.printf("Real-Time >> Current: %.2f A, Voltage: %.2f V\n", current, voltage);

          Firebase.RTDB.setFloat(&fbdo, path + "/Current", current);
          Firebase.RTDB.setFloat(&fbdo, path + "/Voltage", voltage);
        }
        break;
      }

      // ✅ Battery Pack Status
      case 0x09A10004: {
        if (rx_frame.FIR.B.DLC == 8) {
          uint16_t socStatus = ((uint16_t)rx_frame.data.u8[0] << 8) |
                               rx_frame.data.u8[1];
          uint16_t socRaw = socStatus & 0x03FF;
          float socPercent = socRaw / 10.0;
          //bool relayStatus = (socStatus >> 15) & 0x01;
          float highestCell_V = (((uint16_t)rx_frame.data.u8[2] << 8) |
                                  rx_frame.data.u8[3]) / 1000.0;
          float lowestCell_V = (((uint16_t)rx_frame.data.u8[4] << 8) |
                                 rx_frame.data.u8[5]) / 1000.0;

          int8_t tempHigh = (int8_t)rx_frame.data.u8[6];
          int8_t tempLow  = (int8_t)rx_frame.data.u8[7];

          Serial.println("[BATTERY STATUS]");
          //Serial.printf("SOC: %.1f%% | Relay: %s\n", socPercent, relayStatus ? "ON" : "OFF");
          Serial.printf("SOC: %.1f%%,socPercent);
          Firebase.RTDB.setFloat(&fbdo, path + "/SOC_Percent", socPercent);
          //Firebase.RTDB.setBool(&fbdo, path + "/Relay", relayStatus);
          Firebase.RTDB.setFloat(&fbdo, path + "/Cell_Highest_V", highestCell_V);
          Firebase.RTDB.setFloat(&fbdo, path + "/Cell_Lowest_V", lowestCell_V);
          Firebase.RTDB.setInt(&fbdo, path + "/Temp_High", tempHigh);
          Firebase.RTDB.setInt(&fbdo, path + "/Temp_Low", tempLow);
        }
        break;
      }

      // ✅ Controller Status
      case 0x09A10005: {
        if (rx_frame.FIR.B.DLC == 8) {
          uint8_t fault = rx_frame.data.u8[0];
          uint8_t warning = rx_frame.data.u8[1];

          int8_t bmcTemp  = (int8_t)rx_frame.data.u8[2];
          int8_t bicHigh  = (int8_t)rx_frame.data.u8[3];
          int8_t bicLow   = (int8_t)rx_frame.data.u8[4];

          uint16_t timeRaw = ((uint16_t)rx_frame.data.u8[5] << 8) |
                              rx_frame.data.u8[6];
          bool isCharging = (timeRaw >> 15) & 0x01;
          bool hasAccuracy = (timeRaw >> 14) & 0x01;
          uint16_t minutes = timeRaw & 0x3FFF;

          float powerVolt = (rx_frame.data.u8[7] * 0.1f) + 6.5f;

          Serial.println("[CONTROLLER STATUS]");
          Serial.printf("BMC Temp: %d°C, 12V Supply: %.2fV\n", bmcTemp, powerVolt);

          Firebase.RTDB.setInt(&fbdo, path + "/BMC_Temp", bmcTemp);
          Firebase.RTDB.setInt(&fbdo, path + "/BIC_Temp_High", bicHigh);
          Firebase.RTDB.setInt(&fbdo, path + "/BIC_Temp_Low", bicLow);
          Firebase.RTDB.setInt(&fbdo, path + "/Fault_Code", fault);
          Firebase.RTDB.setInt(&fbdo, path + "/Warning_Code", warning);
          Firebase.RTDB.setBool(&fbdo, path + "/Charging", isCharging);
          Firebase.RTDB.setBool(&fbdo, path + "/Time_Accurate", hasAccuracy);
          Firebase.RTDB.setInt(&fbdo, path + "/Remaining_Minutes", minutes);
          Firebase.RTDB.setFloat(&fbdo, path + "/Power_12V", powerVolt);
        }
        break;
      }

      default:
        Serial.printf("Unknown ID: 0x%08X\n", rx_frame.MsgID);
        break;
    }

    // เพิ่ม timestamp ทุกครั้ง
    String timeString = String(millis() / 1000);
    Firebase.RTDB.setString(&fbdo, path + "/Timestamp", timeString);
  }
}

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "CAN_config.h"
#include "ESP32CAN.h"

// ------------------ CONFIG ------------------
#define WIFI_SSID       "CMUF19"
#define WIFI_PASSWORD   "ilovef19"
#define FIREBASE_HOST   "test-data-f19-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH   "HbMfY46ykieKqMFzv4Kdl7vyim5ByDn4aOBr88nm"
#define CAN_TX_PIN      GPIO_NUM_27
#define CAN_RX_PIN      GPIO_NUM_26
#define CAN_SPEED       CAN_SPEED_250KBPS

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
CAN_device_t CAN_cfg;
const int RX_QUEUE_SIZE = 10;

// ------------------ FUNC ------------------
float hexToFloat(uint32_t hex) {
  float f;
  memcpy(&f, &hex, sizeof(float));
  return f;
}

void sendToFirebase(String path, String key, float val) {
  Firebase.RTDB.setFloat(&fbdo, path + "/" + key, val);
}

void sendToFirebase(String path, String key, int val) {
  Firebase.RTDB.setInt(&fbdo, path + "/" + key, val);
}

void sendToFirebase(String path, String key, bool val) {
  Firebase.RTDB.setBool(&fbdo, path + "/" + key, val);
}

void sendToFirebase(String path, String key, String val) {
  Firebase.RTDB.setString(&fbdo, path + "/" + key, val);
}

// ------------------ SETUP ------------------
void setup() {
  Serial.begin(115200);
  Serial.println("\n== ESP32: CAN + Firebase ==");

  CAN_cfg = {CAN_SPEED, CAN_TX_PIN, CAN_RX_PIN, xQueueCreate(RX_QUEUE_SIZE, sizeof(CAN_frame_t))};
  ESP32Can.CANInit();
  Serial.println("CAN Bus ready @ 250kbps");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(300); }
  Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase connected!");
}

// ------------------ LOOP ------------------
void loop() {
  CAN_frame_t frame;
  String path = "/ESP32_Data/Location1";

  if (xQueueReceive(CAN_cfg.rx_queue, &frame, 3 / portTICK_PERIOD_MS) == pdTRUE) {
    uint8_t *d = frame.data.u8;

    switch (frame.MsgID) {
      // ✅ Real-time Power
      case 0x09A10003: {
        if (frame.FIR.B.DLC == 8) {
          float current = hexToFloat(((uint32_t)d[0] << 24) | ((uint32_t)d[1] << 16) | ((uint32_t)d[2] << 8) | d[3]);
          float voltage = hexToFloat(((uint32_t)d[4] << 24) | ((uint32_t)d[5] << 16) | ((uint32_t)d[6] << 8) | d[7]);
          Serial.printf("Real-Time >> I: %.2f A, V: %.2f V\n", current, voltage);
          sendToFirebase(path, "Current", current);
          sendToFirebase(path, "Voltage", voltage);
        }
        break;
      }

      // ✅ Battery Status
      case 0x09A10004: {
        if (frame.FIR.B.DLC == 8) {
          uint16_t socStatus = ((uint16_t)d[0] << 8) | d[1];
          float soc = (socStatus & 0x03FF) / 10.0;
          float cellHigh = (((uint16_t)d[2] << 8) | d[3]) / 1000.0;
          float cellLow  = (((uint16_t)d[4] << 8) | d[5]) / 1000.0;
          int8_t tHigh = (int8_t)d[6], tLow = (int8_t)d[7];

          Serial.printf("[BATTERY] SOC: %.1f%% | CH: %.3fV | CL: %.3fV | T:%d/%d°C\n", soc, cellHigh, cellLow, tHigh, tLow);
          sendToFirebase(path, "SOC_Percent", soc);
          sendToFirebase(path, "Cell_Highest_V", cellHigh);
          sendToFirebase(path, "Cell_Lowest_V", cellLow);
          sendToFirebase(path, "Temp_High", tHigh);
          sendToFirebase(path, "Temp_Low", tLow);
        }
        break;
      }

      // ✅ Controller Status
      case 0x09A10005: {
        if (frame.FIR.B.DLC == 8) {
          uint8_t fault = d[0], warning = d[1];
          int8_t bmcT = (int8_t)d[2], bicH = (int8_t)d[3], bicL = (int8_t)d[4];
          uint16_t timeRaw = ((uint16_t)d[5] << 8) | d[6];
          bool charging = (timeRaw >> 15) & 1, accurate = (timeRaw >> 14) & 1;
          uint16_t mins = timeRaw & 0x3FFF;
          float power12 = (d[7] * 0.1f) + 6.5f;

          Serial.printf("[CTRL] BMC:%d°C | 12V:%.2f | Fault:%d | Warn:%d\n", bmcT, power12, fault, warning);
          sendToFirebase(path, "BMC_Temp", bmcT);
          sendToFirebase(path, "BIC_Temp_High", bicH);
          sendToFirebase(path, "BIC_Temp_Low", bicL);
          sendToFirebase(path, "Fault_Code", fault);
          sendToFirebase(path, "Warning_Code", warning);
          sendToFirebase(path, "Charging", charging);
          sendToFirebase(path, "Time_Accurate", accurate);
          sendToFirebase(path, "Remaining_Minutes", mins);
          sendToFirebase(path, "Power_12V", power12);
        }
        break;
      }

      default:
        Serial.printf("Unknown ID: 0x%08X\n", frame.MsgID);
    }

    // ⏱ Timestamp
    sendToFirebase(path, "Timestamp", String(millis() / 1000));
  }
}

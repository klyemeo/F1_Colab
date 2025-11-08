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
    if (rx_frame.MsgID == 0x09A10003 && rx_frame.FIR.B.DLC == 8) {
      // ดึงค่า float 2 ค่า (Current และ Voltage)
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

      Serial.printf("CAN >> Current: %.2f A, Voltage: %.2f V\n", current, voltage);

      // ส่งขึ้น Firebase
      String path = "/ESP32_Data/Location1";

      if (Firebase.RTDB.setFloat(&fbdo, path + "/Current", current)) {
        Serial.println("ส่ง Current สำเร็จ");
      } else {
        Serial.println("ส่ง Current ล้มเหลว: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, path + "/Voltage", voltage)) {
        Serial.println("ส่ง Voltage สำเร็จ");
      } else {
        Serial.println("ส่ง Voltage ล้มเหลว: " + fbdo.errorReason());
      }

      // ส่ง timestamp เป็นวินาที
      String timeString = String(millis() / 1000);
      Firebase.RTDB.setString(&fbdo, path + "/Timestamp", timeString);
    } else {
      // ถ้า ID ไม่ใช่ หรือข้อมูลไม่ครบ 8 byte
      Serial.printf("CAN[0x%08X] DLC=%d >> ", rx_frame.MsgID, rx_frame.FIR.B.DLC);
      for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
        Serial.printf("0x%02X ", rx_frame.data.u8[i]);
      }
      Serial.println();
    }
  }
}

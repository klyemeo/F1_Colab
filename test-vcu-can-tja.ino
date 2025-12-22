#include "driver/twai.h"
#define RX_PIN GPIO_NUM_26
#define TX_PIN GPIO_NUM_27

void setup() {
  
  // 1. ตั้งค่า Config ของ TWAI
Serial.begin(115200);

  // ---------- CAN CONFIG ----------
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, TWAI_MODE_NORMAL);

  twai_timing_config_t t_config =
      TWAI_TIMING_CONFIG_500KBITS();

  twai_filter_config_t f_config =
      TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();

  Serial.println("ESP32 CAN Receiver Ready @500kbps");

  // 2. ติดตั้ง Driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return;
  }
}

void loop() {
  twai_message_t message;

  if (twai_receive(&message, pdMS_TO_TICKS(50)) == ESP_OK) {
      // --- แสดงผลข้อมูลที่ได้รับ ---
      Serial.print("ID: 0x");
      Serial.print(message.identifier, HEX); // แสดง CAN ID
    
      // --- ตัวอย่างการนำไปใช้: เช็ค ID เพื่อทำงานเฉพาะอย่าง ---
      if (message.identifier == 0x7E1) {
        uint16_t raw = (message.data[1] << 8) | message.data[0];
        float voltage = raw * 0.1; // Example: 923 → 92.3V
        Serial.print(" -> Udc (V): ");
        Serial.println(voltage, 1);
      }
      if (message.identifier == 0x7E2) {
        uint16_t raw = (message.data[1] << 8) | message.data[0];
        float current = raw * 0.1;
        Serial.print(" -> Idc (A): ");
        Serial.println(current, 1);
      }
      if (message.identifier== 0x7E3) {
        int16_t speed = (message.data[1] << 8) | message.data[0];
        // float speed = raw;
        Serial.print(" -> speed (rpm): ");
        Serial.println(speed);
      }
      // ====== Decode CAN ID: 0x7E4 ======
      if (message.identifier== 0x7E4) {
        int16_t tmpm = (message.data[1] << 8) | message.data[0];
        // float tmpm = raw;
        Serial.print(" -> tmpm (C): ");
        Serial.println(tmpm);
      }
      // ====== Decode CAN ID: 0x7E5 ======
      if (message.identifier== 0x7E5) {
        int16_t tmphs = (message.data[1] << 8) | message.data[0];
        // float  = raw;
        Serial.print(" -> tmphs (C): ");
        Serial.println(tmphs);
      }
      if (message.identifier== 0x7E6) {
        int16_t lasterr = (message.data[1] << 8) | message.data[0];
        // float  = raw;
        Serial.print(" -> lasterr (A): ");
        Serial.println(lasterr);
      }
      if (message.identifier== 0x7E7) {
        int16_t gerrer = (message.data[1] << 8) | message.data[0];
        // float  = raw;
        Serial.print(" -> gerrer (A): ");
        Serial.println(gerrer*0.1);
      }
      if (message.identifier== 0x7E8) {
        int16_t uaux = (message.data[1] << 8) | message.data[0];
        // float  = raw;
        Serial.print(" -> uaux (V): ");
        Serial.println(uaux*0.1,1);
      }
      if (message.identifier== 0x7E9) {
        int16_t dir = (message.data[1] << 8) | message.data[0];
        // float  = raw;
        Serial.print(" -> dir (-): ");
        Serial.println(dir);
      }

      if (message.identifier == 0x7EA) {
        // ทำงานเมื่อได้รับ ID 0x123 เช่น อ่านค่า RPM
        int opmode = (message.data[1] << 8) | message.data[0]; 
        Serial.print("--> opmode :");
        Serial.println(opmode);
      }
    }
}

float hexToFloat(uint32_t hexValue) {
  float floatValue;
  memcpy(&floatValue, &hexValue, sizeof(float));
  return floatValue;
}

#include "driver/twai.h"

#define CAN_TX_PIN GPIO_NUM_27   // ไม่ได้ใช้ส่ง แต่ต้องกำหนด
#define CAN_RX_PIN GPIO_NUM_26

float bytesToFloat(uint8_t *data) {
  float value;
  memcpy(&value, data, sizeof(float));
  return value;
}

void setup() {
  Serial.begin(115200);

  // ---------- CAN CONFIG ----------
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);

  twai_timing_config_t t_config =
      TWAI_TIMING_CONFIG_500KBITS();

  twai_filter_config_t f_config =
      TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();

  Serial.println("ESP32 CAN Receiver Ready @500kbps");
}

void loop() {
  twai_message_t rx_msg;

  if (twai_receive(&rx_msg, pdMS_TO_TICKS(100)) == ESP_OK) {

    if (rx_msg.identifier == 0x300 && rx_msg.data_length_code == 8) {

      float current = bytesToFloat(&rx_msg.data[0]);
      float voltage = bytesToFloat(&rx_msg.data[4]);

      Serial.print("CURRENT = ");
      Serial.println(current, 2);

      Serial.print("VOLTAGE = ");
      Serial.println(voltage, 2);
      Serial.println("-------------------");
    }
  }
}

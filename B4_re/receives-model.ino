#include "driver/twai.h"

#define TWAI_TX GPIO_NUM_27
#define TWAI_RX GPIO_NUM_26

void setup() {
  Serial.begin(115200);

  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX, TWAI_RX, TWAI_MODE_NORMAL);

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  ESP_ERROR_CHECK(twai_start());

  Serial.println("✅ ESP32 #2 TWAI Receiver Ready");
}

void loop() {
  twai_message_t rx_msg;

  if (twai_receive(&rx_msg, pdMS_TO_TICKS(100)) == ESP_OK) {

    Serial.printf("RX ID:0x%X DLC:%d Data:",
                  rx_msg.identifier,
                  rx_msg.data_length_code);

    for (int i = 0; i < rx_msg.data_length_code; i++) {
      Serial.printf(" %02X", rx_msg.data[i]);
    }
    Serial.println();

    // 👉 ตรงนี้เอาไปส่ง WiFi / Firebase ต่อได้เลย
  }
}

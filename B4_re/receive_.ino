#include "driver/twai.h"

#define TX_PIN GPIO_NUM_4
#define RX_PIN GPIO_NUM_5

void setup() {
  Serial.begin(115200);
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config  = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config  = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();
  Serial.println("TWAI Driver Started");
}

void loop() {
  // 1. Send Sensor Data
  static uint32_t lastSend = 0;
  if (millis() - lastSend > 1000) {
    float sensorVal = 36.6; // Replace with your sensor read
    twai_message_t tx_msg;
    tx_msg.identifier = 0x102;
    tx_msg.data_length_code = 4;
    memcpy(tx_msg.data, &sensorVal, 4);
    twai_transmit(&tx_msg, pdMS_TO_TICKS(10));
    lastSend = millis();
  }

  // 2. Receive Data
  twai_message_t rx_msg;
  if (twai_receive(&rx_msg, pdMS_TO_TICKS(1)) == ESP_OK) {
    if (rx_msg.identifier == 0x101) {
      float receivedVal;
      memcpy(&receivedVal, rx_msg.data, 4);
      Serial.printf("Received from Node 1: %.2f\n", receivedVal);
    }
  }
}

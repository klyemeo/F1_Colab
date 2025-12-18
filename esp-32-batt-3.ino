#include <SPI.h>
#include <mcp2515.h>
#include "driver/twai.h"

struct can_frame canMsg;
MCP2515 mcp2515(5);   // CS MCP2515

#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26

// =================== FLOAT CONVERT ===================
float hexToFloat(uint32_t hexValue) {
  float floatValue;
  memcpy(&floatValue, &hexValue, sizeof(float));
  return floatValue;
}

// =================== SETUP ===================
void setup() {
  Serial.begin(115200);

  // ---------- MCP2515 (250 kbps) ----------
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  // ---------- ESP32 CAN (500 kbps) ----------
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);

  twai_timing_config_t t_config =
      TWAI_TIMING_CONFIG_500KBITS();

  twai_filter_config_t f_config =
      TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();

  Serial.println("CAN Gateway Ready (250 -> 500 kbps)");
}

// =================== LOOP ===================
void loop() {

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {

    if (canMsg.can_id == 0x89A10003) {

      uint32_t ieeeHex1 =
          ((uint32_t)canMsg.data[0] << 24) |
          ((uint32_t)canMsg.data[1] << 16) |
          ((uint32_t)canMsg.data[2] << 8) |
          ((uint32_t)canMsg.data[3]);

      uint32_t ieeeHex2 =
          ((uint32_t)canMsg.data[4] << 24) |
          ((uint32_t)canMsg.data[5] << 16) |
          ((uint32_t)canMsg.data[6] << 8) |
          ((uint32_t)canMsg.data[7]);

      float current = hexToFloat(ieeeHex1);
      float voltage = hexToFloat(ieeeHex2);

      Serial.print("CURRENT: ");
      Serial.println(current, 2);
      Serial.print("VOLTAGE: ");
      Serial.println(voltage, 2);

      // ---------- ส่งต่อ CAN @500 kbps ----------
      twai_message_t tx_msg = {};
      tx_msg.identifier = 0x300;     // ID ใหม่ฝั่ง main bus
      tx_msg.extd = 0;               // standard ID
      tx_msg.data_length_code = 8;

      memcpy(&tx_msg.data[0], &current, 4);
      memcpy(&tx_msg.data[4], &voltage, 4);

      twai_transmit(&tx_msg, pdMS_TO_TICKS(10));

      delay(150);
    }
  }
}

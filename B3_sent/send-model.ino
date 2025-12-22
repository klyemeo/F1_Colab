#include "driver/twai.h"
#include <SPI.h>
#include <mcp_can.h>

#define TWAI_TX GPIO_NUM_27
#define TWAI_RX GPIO_NUM_26

// MCP2515
#define MCP_CS  5
#define MCP_INT 4

MCP_CAN mcp2515(MCP_CS);

void setup() {
  Serial.begin(115200);

  // ================= TWAI CONFIG =================
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX, TWAI_RX, TWAI_MODE_NORMAL);

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  ESP_ERROR_CHECK(twai_start());

  Serial.println("✅ TWAI started (VCU side)");

  // ================= MCP2515 CONFIG =================
  SPI.begin(18, 19, 23, MCP_CS);

  while (mcp2515.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) != CAN_OK) {
    Serial.println("❌ MCP2515 init failed");
    delay(500);
  }

  mcp2515.setMode(MCP_NORMAL);
  Serial.println("✅ MCP2515 ready");
}

void loop() {
  twai_message_t rx_msg;

  // รับ CAN จาก VCU
  if (twai_receive(&rx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {

    Serial.printf("RX VCU ID:0x%X DLC:%d\n",
                  rx_msg.identifier, rx_msg.data_length_code);

    // ส่งต่อไป MCP2515

    if (rx_msg.identifier == 0x7EA) {
    mcp2515.sendMsgBuf(
      rx_msg.identifier,
      rx_msg.extd,                      // 0=STD 1=EXT
      rx_msg.data_length_code,
      rx_msg.data
    );
    }
  }
}

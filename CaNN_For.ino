#include "CAN_config.h"
#include "ESP32CAN.h"

// REQUIRE:
// ESP32 by Espressif
// 2.0.17

// CAN1: TJA1050 parameters
#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26
CAN_device_t CAN_cfg;             // CAN Config
const int    rx_queue_size = 10;  // Receive Queue size

static unsigned long previousMillis50   = 0;   // will store last time a 50ms CAN Message was send
static unsigned long previousMillis500  = 0;   // will store last time a 50ms CAN Message was send
static unsigned long previousMillis1000 = 0;   // will store last time a 50ms CAN Message was send

void setup() {
  Serial.begin(115200);
  Serial.println("WELCOME USB CAN-MESSAGE MONITORING");
  Serial.println("by");
  Serial.println("DR.ANUCHA PROMWUNGKWA");
  Serial.println("USB SERIAL: 115200 bps");
  Serial.println("CAN SPEED: 500 kbps");
  delay(3000);

  // CAN1
  CAN_cfg.speed = CAN_SPEED_250KBPS;
  CAN_cfg.tx_pin_id = CAN_TX_PIN;
  CAN_cfg.rx_pin_id = CAN_RX_PIN;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();
}

void loop() {

  // CAN1: rx frame
  CAN_frame_t rx_frame;

  // Receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
  {

    if (rx_frame.FIR.B.FF == CAN_frame_std)
    {
      printf("New standard frame");
    }
    else
    {
      printf("New extended frame");
    }

    if (rx_frame.FIR.B.RTR == CAN_RTR)
    {
      printf(" RTR from 0x%08X, DLC %d\r\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
    }
    else
    {
      printf(" from 0x%08X, DLC %d, Data ", rx_frame.MsgID, rx_frame.FIR.B.DLC);
      for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
      {
        printf("0x%02X ", rx_frame.data.u8[i]);
      }
      printf("\n");
    }
  }



}


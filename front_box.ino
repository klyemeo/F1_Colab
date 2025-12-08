#include "driver/twai.h"

// กำหนดขา GPIO
#define RX_PIN 26
#define TX_PIN 27

const int BR1 = 36;
const int BR2 = 39;
const int APP1 = 34;
const int APP2 = 35;


const int VCU_state = 32;
int HVA;
int break_state;
int RTD;
int RTD_state;

void setup() {
  Serial.begin(115200);
  
  // 1. ตั้งค่า Config ของ TWAI
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); // ตั้งค่า Baudrate 500kbps
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // 2. ติดตั้ง Driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return;
  }

  // 3. เริ่มการทำงาน
  if (twai_start() == ESP_OK) {
    Serial.println("Driver started");
  } else {
    Serial.println("Failed to start driver");
    return;
  }
}


bool check_condition(int &APP1_Val , int &APP2_Val , int threshold , int duration_ms ) {
  unsigned long start_time = millis();  // Get the current time at the start
  while (millis() - start_time < duration_ms) {
    APP1_Val = analogRead(APP1);  // Read the value from potentiometer A
    APP2_Val= analogRead(APP2);  // Read the value from potentiometer B

    if (abs(APP1_Val - APP2_Val) <= threshold) {
      return false;  // Condition failed within the duration
    }
  }
  return true;  // Condition held true for the entire duration
}

void loop() {
  break_state = 0;
  RTD_state = 0;

  int APP1_val = analogRead(APP1);
  int APP2_val = analogRead(APP2);
  int BR1_val = analogRead(BR1);
  int BR2_val = analogRead(BR2);
  int RTD = analogRead(VCU_state);
  int opmode;

  int horn = digitalRead(12);


  twai_message_t message;
   
  //LOW Voltage on
  while((HVA == LOW) && (break_state == 0)){
    send_brake(); //ส่งค่าเบรกผ่าน CAN
  }
  break_state = 1;

  while(HVA == HIGH){
    while((RTD_state == 0) && (break_state == 1)){
      send_brake();
      //receive_op_state();
      //twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(50)) == ESP_OK) {
          // --- แสดงผลข้อมูลที่ได้รับ ---
          Serial.print("ID: 0x");
          Serial.print(message.identifier, HEX); // แสดง CAN ID
          
          // --- ตัวอย่างการนำไปใช้: เช็ค ID เพื่อทำงานเฉพาะอย่าง ---
          
          if (message.identifier == 0x7EA) {
            // ทำงานเมื่อได้รับ ID 0x123 เช่น อ่านค่า RPM
            int opmode = (message.data[1] << 8) | message.data[0]; 
            Serial.print("--> opmode :");
            Serial.println(opmode);
          }
        }
        if (((( BR1_val > 8 or BR2_val > 8 ) && RTD > 3000) && opmode == 0) && RTD_state == 0) {
          RTD_state = 1;
          send_vcu();
          Serial.println("if RTD");
          send_brake();
          digitalWrite(horn,HIGH) ;
          delay (1000) ;
          digitalWrite(horn,LOW) ;
          delay (100) ;
          digitalWrite(horn,HIGH) ;
          delay (100) ;
          digitalWrite(horn,LOW) ;
          delay (100) ;
          digitalWrite(horn,HIGH) ;
          delay (100) ;
          digitalWrite(horn,LOW) ;
          send_brake();
          // receive_op_state();
      }
      // receive_op_state();
      //twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(50)) == ESP_OK) {
          // --- แสดงผลข้อมูลที่ได้รับ ---
          Serial.print("ID: 0x");
          Serial.print(message.identifier, HEX); // แสดง CAN ID
          
          // --- ตัวอย่างการนำไปใช้: เช็ค ID เพื่อทำงานเฉพาะอย่าง ---
          
          if (message.identifier == 0x7EA) {
            // ทำงานเมื่อได้รับ ID 0x123 เช่น อ่านค่า RPM
            int opmode = (message.data[1] << 8) | message.data[0]; 
            Serial.print("--> opmode :");
            Serial.println(opmode);
          }
        }
      while (opmode == 1) {
        //State_Vcu = 0 ;
        
        if (HVA == LOW){
          opmode = 1 ;
          Serial.println("re-state");  
        }
          int BR1_val = analogRead(BR1);
          int BR2_val = analogRead(BR2);
        
          int check = 1 ;
          
          if (check_condition(APP1_val, APP2_val, 70, 100)) {

            check = 0;
            while (APP1_val >= 201) {
              check = 0;
              Serial.println("Fail w/ diff for 100ms");
                int APP1_val = analogRead(APP1);
                int APP2_val = analogRead(APP2);

            }

          } else if (APP1_val>343 and (BR1_val > 30 or BR2_val > 30)) {
            check = 0;
              while (APP1_val >= 201) {
                check = 0;
                Serial.println("Fail w/ brake");
                int APP1_val = analogRead(APP1);
                int APP2_val = analogRead(APP2);
            }

          } else if (check == 1){
            
              int APPS1 = map(APP1_val, 0, 4095, 0, 255);
              int APPS2 = map(APP2_val, 0, 4095, 0, 255);
              int BRAKE1 = map(BR1_val , 0, 4095, 0, 255);
              int BRAKE2 = map(BR2_val, 0, 4095, 0, 255);   
                
              twai_message_t message;
              message.identifier = 0x111; // CAN ID
              message.extd = 0;           // Standard Frame (11-bit)
              message.data_length_code = 4;
              // for (int i = 0; i < 2; i++) {
              //   message.data[i] = i;
              // }
              message.data[0] = BRAKE1;
              message.data[1] = BRAKE2;
              message.data[2] = APPS1;
              message.data[3] = APPS2;

              if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
              Serial.println("Message queued for transmission");
              } else {
              Serial.println("Failed to queue message for transmission");
              }
              
            delay(200); 
          }
      
      }
    }
  }
}
 
int send_brake(){
  int BR1_val = analogRead(BR1);
  int BR2_val = analogRead(BR2);

  int BRAKE1 = map(BR1_val , 0, 4095, 0, 255);
  int BRAKE2 = map(BR2_val, 0, 4095, 0, 255); 

  twai_message_t message;
  message.identifier = 0x123; // CAN ID
  message.extd = 0;           // Standard Frame (11-bit)
  message.data_length_code = 2;
  // for (int i = 0; i < 2; i++) {
  //   message.data[i] = i;
  // }
  message.data[0] = BRAKE1;
  message.data[1] = BRAKE2;


  if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
  //   Serial.println("Message queued for transmission");
  // } else {
  //   Serial.println("Failed to queue message for transmission");
  // }
    Serial.print("Sent -> S1: ");
    Serial.print(BRAKE1);
    Serial.print(" | S2: ");
    Serial.println(BRAKE2);

  }
}

/*
int receive_op_state(){
    twai_message_t message;
   if (twai_receive(&message, pdMS_TO_TICKS(50)) == ESP_OK) {
    
    // --- แสดงผลข้อมูลที่ได้รับ ---
     Serial.print("ID: 0x");
     Serial.print(message.identifier, HEX); // แสดง CAN ID
    
    // --- ตัวอย่างการนำไปใช้: เช็ค ID เพื่อทำงานเฉพาะอย่าง ---
    
    if (message.identifier == 0x7EA) {
       // ทำงานเมื่อได้รับ ID 0x123 เช่น อ่านค่า RPM
       int opmode = (message.data[1] << 8) | message.data[0]; 
       Serial.print("--> opmode :");
       Serial.println(opmode);
    }
  }
  return opmode;
}
*/

int send_vcu(){
  twai_message_t message;
  message.identifier = 0x222; // CAN ID
  message.extd = 0;           // Standard Frame (11-bit)
  message.data_length_code = 1;
  // for (int i = 0; i < 2; i++) {
  //   message.data[i] = i;
  // }
  message.data[0] = 1;


  if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
  //   Serial.println("Message queued for transmission");
  // } else {
  //   Serial.println("Failed to queue message for transmission");
  // }
    Serial.print("Sent -> VCU: ");
    Serial.print("On");
    
  }
}

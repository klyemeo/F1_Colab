#include <SPI.h>
#include <mcp2515.h>

struct  can_frame canMsg2;

struct  can_frame canMsg;
MCP2515 mcp2515(10);


int ana0 = A0 ;
int ana1 = A1 ;
int ana2 = A2 ;
int ana4 = A4 ;
int ana5 = A5 ;

int horn = 2 ;

int State_Vcu ;
int HVA ;

void setup() {
  Serial.begin(112500);

  pinMode(ana0,INPUT) ;
  pinMode(ana1,INPUT) ;
  pinMode(ana2,INPUT) ;
  pinMode(ana4,INPUT) ;
  pinMode(ana5,INPUT) ;
  
  pinMode(horn,OUTPUT) ;

  pinMode(3,INPUT) ;

  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  canMsg2.can_id  = 0x036;
  canMsg2.can_dlc = 8;
  canMsg2.data[0] = 0x0E;
  canMsg2.data[1] = 0x00;
  canMsg2.data[2] = 0x00;
  canMsg2.data[3] = 0x08;
  canMsg2.data[4] = 0x01;
  canMsg2.data[5] = 0x00;
  canMsg2.data[6] = 0x00;
  canMsg2.data[7] = 0xA0;

}


//Set up  Condition APPS and Break  
  bool check_condition(int &AP1 , int &AP2 , int threshold , int duration_ms ) {
  unsigned long start_time = millis();  // Get the current time at the start
  
  while (millis() - start_time < duration_ms) {
    AP1 = analogRead(ana1);  // Read the value from potentiometer A
    AP2 = analogRead(ana2);  // Read the value from potentiometer B

    if (abs(AP1 - AP2) <= threshold) {
      return false;  // Condition failed within the duration
    }
  }
  
  return true;  // Condition held true for the entire duration
}


void loop() {

  int RTD = analogRead(ana0) ;
  int AP1 = analogRead(ana1) ;
  int AP2 = analogRead(ana2) ;
  int BR4 = analogRead(ana4) ;
  int BR5 = analogRead(ana5) ;

  int check = 1;

  HVA = digitalRead(3) ;

  State_Vcu = 1 ;

  

  //tfunc () ;
  
  Serial.println("loop");
  
  
  
  if (HVA == LOW)
    {
    State_Vcu = 1 ;

    Serial.println("if vcu state");

    if ((( BR4 > 8 or BR5 > 8 ) & RTD > 1000) & State_Vcu == 1) {

      Serial.println("if RTD");
        
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

      while (State_Vcu == 1) {
        //State_Vcu = 0 ;
        HVA = digitalRead(3) ;
        if (HVA == LOW){
            
          State_Vcu = 1 ;
          Serial.println("re stateeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");  
          
        }
  
        Serial.println("while");
          //int AP1 = analogRead(ana1) ;
          //int AP2 = analogRead(ana2) ;
          BR4 = analogRead(ana4) ;
          BR5 = analogRead(ana5) ;
          check = 1 ;
          
        if (check_condition(AP1, AP2, 70, 100)) {

          check = 0;
          while (AP1 >= 201) {
            check = 0;
            Serial.println("Fail w/ diff for 100ms");
            AP1 = analogRead(ana1) ;
            AP2 = analogRead(ana2) ;
          }



        } else if (AP1>343 and (BR4 > 30 or BR5 > 30)) {
          check = 0;
          while (AP1 >= 201) {
            check = 0;
            Serial.println("Fail w/ brake");
            AP1 = analogRead(ana1) ;
            AP2 = analogRead(ana2) ;
          }
    
  
        } else if (check == 1){
          
          
            
          int APPS1 = map(AP1, 0, 1023, 0, 255);
          int APPS2 = map(AP2, 0, 1023, 0, 255);
          int BRAKE5 = map(BR4, 0, 1023, 0, 255);
          int BRAKE4 = map(BR5, 0, 1023, 0, 255);  
            
          canMsg2.can_id  = 0x001;
          canMsg2.can_dlc = 4;
          canMsg2.data[0] = APPS1;
          canMsg2.data[1] = APPS2;
          canMsg2.data[2] = BRAKE4;
          canMsg2.data[3] = BRAKE5;

          mcp2515.sendMessage(&canMsg2);
            
          Serial.print(AP1);
          Serial.print("  ");
          Serial.print(AP2);
          Serial.print("  ");
          Serial.print(BR4);
          Serial.print("  ");
          Serial.println(BR5);
          

    
        
        }
      }
    } 

    
  }


//delay(100);
}












void tfunc () {
  
  int AP1 = analogRead(ana1) ;
  int AP2 = analogRead(ana2) ;
  int BR4 = analogRead(ana4) ;
  int BR5 = analogRead(ana5) ;

  int check = 1;

  
  if (check_condition(AP1, AP2, 70, 100)) {
    check = 0;
    while (AP1 >= 201) {
      check = 0;
      Serial.println("Fail w/ diff for 100ms");
      AP1 = analogRead(ana1) ;
      AP2 = analogRead(ana2) ;
    }



  } else if (AP1>343 and (BR4 > 3 or BR5 > 3)) {
    check = 0;
    while (AP1 >= 201) {
      check = 0;
      Serial.println("Fail w/ brake");
      AP1 = analogRead(ana1) ;
      AP2 = analogRead(ana2) ;
    }
    
  
  } else if (check == 1){
    

    Serial.print(AP1);
    Serial.print("  ");
    Serial.print(AP2);
    Serial.print("  ");
    Serial.print(BR4);
    Serial.print("  ");
    Serial.println(BR5);

  }
}





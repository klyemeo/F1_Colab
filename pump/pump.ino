int RPWM_Output = 4;
int LPWM_Output = 2;

const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

void setup() {
  pinMode(RPWM_Output, OUTPUT);
  pinMode(LPWM_Output, OUTPUT);

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(RPWM_Output, ledChannel); 
}

void loop() {
  digitalWrite(LPWM_Output, LOW); 
  ledcWrite(ledChannel, 255); 
  delay(100);
}
#include <ESP32Servo.h>

Servo myservo;

const int buttonPin = 34;
const int vibrationPin = 33;
int buttonState = HIGH;  // 初始狀態設為HIGH（未按下）
int lastButtonState = HIGH;
int vibrationState = 0;
bool servoState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  myservo.attach(14);
  pinMode(buttonPin, INPUT);  // 不使用內部上拉，假設使用外部上拉電阻
  pinMode(vibrationPin, INPUT);
  Serial.begin(115200);
  Serial.println("System initialized");
}

void loop() {
  int reading = digitalRead(buttonPin);
  
  // 添加調試信息
  Serial.print("Button reading: ");
  Serial.println(reading);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == LOW) {  // 按鈕按下
        Serial.println("press");
        servoState = !servoState;
        myservo.write(servoState ? 180 : 0);
        Serial.println(servoState ? "舵機轉到180度" : "舵機轉到0度");
      } else {  // 按鈕釋放
        Serial.println("x");
      }
    }
  }

  vibrationState = digitalRead(vibrationPin);
  if (vibrationState == HIGH) {
    myservo.write(60);
    Serial.println("震動感應到，舵機轉到60度");
    delay(1000);
    myservo.write(servoState ? 180 : 0);
    Serial.println(servoState ? "舵機回到180度" : "舵機回到0度");
  }

  lastButtonState = reading;
  delay(10);  // 小延遲以穩定讀取
}
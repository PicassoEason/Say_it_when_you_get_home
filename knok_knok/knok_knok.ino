#include <Servo.h>

Servo myservo;  // 創建Servo對象來控制舵機

const int buttonPin = D1;  // 按鈕連接到D1引腳（GPIO5）
const int vibrationPin = D0; // SW-420模組連接到D0引腳（GPIO0）
int buttonState = 0;      // 用於儲存按鈕的狀態
int lastButtonState = 0;  // 用於儲存上一次按鈕的狀態
int vibrationState = 0;   // 用於儲存震動感應器的狀態
bool servoState = false;  // 用於記錄舵機當前的狀態

void setup() {
  myservo.attach(D2);       // 將舵機物理連接到D2引腳（GPIO4）
  pinMode(buttonPin, INPUT_PULLUP); // 設置按鈕引腳為輸入並啟用內建上拉電阻
  pinMode(vibrationPin, INPUT);     // 設置震動感應模組引腳為輸入
  Serial.begin(115200);     // 啟動序列監視器，設置波特率為115200
}

void loop() {
  buttonState = digitalRead(buttonPin);  // 讀取按鈕的狀態
  vibrationState = digitalRead(vibrationPin); // 讀取震動感應器的狀態
    myservo.write(60); // 如果感應到震動，將舵機轉到90度

  // 檢查按鈕是否從未按下變為按下
  if (buttonState == LOW && lastButtonState == HIGH) {
    servoState = !servoState; // 改變舵機的狀態
    if (servoState) {
      myservo.write(180);     // 將舵機轉到180度
      Serial.println("舵機轉到180度");
    } else {
      myservo.write(0);       // 將舵機轉到0度
      Serial.println("舵機轉到0度");
    }
    delay(50); // 消除按鈕抖動
  }
 
  // 檢查震動感應器是否觸發
  if (vibrationState == HIGH) {
    myservo.write(90); // 如果感應到震動，將舵機轉到90度
    Serial.println("震動感應到，舵機轉到90度");
    delay(1000); // 保持位置一段時間
  }

  lastButtonState = buttonState; // 記錄當前按鈕狀態
}
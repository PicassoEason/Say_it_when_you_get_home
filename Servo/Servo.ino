#include <ESP32Servo.h>  // 使用ESP32Servo庫

Servo myservo;  // 創建Servo對象來控制舵機

const int buttonPin = 34;  // 按鈕連接到D34引腳（GPIO34）
const int vibrationPin = 33; // SW-420模組連接到D33引腳（GPIO33）
int buttonState = 0;      // 用於儲存按鈕的狀態
int lastButtonState = 0;  // 用於儲存上一次按鈕的狀態
int vibrationState = 0;   // 用於儲存震動感應器的狀態
bool servoState = false;  // 用於記錄舵機當前的狀態

void setup() {
  myservo.attach(25);       // 將舵機物理連接到D25引腳（GPIO25）
  pinMode(buttonPin, INPUT); // 設置按鈕引腳為輸入並啟用內建上拉電阻
  pinMode(vibrationPin, INPUT);     // 設置震動感應模組引腳為輸入
  Serial.begin(9600);     // 啟動序列監視器，設置波特率為9600
}

void loop() {
  buttonState = digitalRead(buttonPin);  // 讀取按鈕的狀態
  vibrationState = digitalRead(vibrationPin); // 讀取震動感應器的狀態
  

  // 檢查按鈕是否從未按下變為按下
  // if (buttonState != lastButtonState && buttonState == HIGH) {
  //   servoState = !servoState; // 改變舵機的狀態
  //   if (servoState) {
  //     myservo.write(180);     // 將舵機轉到180度
  //     Serial.println("舵機轉到180度");
  //   } else {
  //     myservo.write(0);       // 將舵機轉到0度
  //     Serial.println("舵機轉到0度");
  //   }
  //   delay(50); // 消除按鈕抖動
  // }

  if(buttonState == LOW){          //如果按鍵按了
    myservo.write(180);       // 將舵機轉到0度
    Serial.println("舵機轉到180度");
    Serial.println("按鍵按了");
  }else{                           //如果按鍵是未按下
    Serial.println("按鍵是未按下");
    myservo.write(0);       // 將舵機轉到0度
    Serial.println("舵機轉到0度");
  }
    delay(50);
  // 檢查震動感應器是否觸發
  if (vibrationState == HIGH) {
    myservo.write(60); // 如果感應到震動，將舵機轉到60度
    Serial.println("震動感應到，舵機轉到60度");
    delay(1000); // 保持位置一段時間
  }

  lastButtonState = buttonState; // 記錄當前按鈕狀態
}
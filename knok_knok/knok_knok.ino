#include <Servo.h>

const int vibrationPin = D3; // SW-420 模块连接到 D3 引脚（GPIO0）
const int servoPin = D2; // 伺服电机连接到 D2 引脚（GPIO4）
Servo myservo;

void setup() {
  pinMode(vibrationPin, INPUT);  // 设置震动感应模块引脚为输入
  myservo.attach(servoPin); // 附加伺服电机到 D2 引脚
  Serial.begin(115200); // 启动串行监视器，设置波特率为 115200
  myservo.write(0); // 初始化伺服电机位置
}

void loop() {
  int vibrationState = digitalRead(vibrationPin); // 读取震动感应器的状态

  if (vibrationState == HIGH) {
    Serial.println("震动感应到");
    myservo.write(45); // 设置伺服电机到 45 度
  } else {
    Serial.println("无震动");
    
  }

  delay(500); // 等待 500 毫秒再读取下一次
}

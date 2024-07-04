const int vibrationPin = D3; // SW-420模块连接到D3引脚（GPIO0）

void setup() {
  pinMode(vibrationPin, INPUT);  // 设置震动感应模块引脚为输入
  Serial.begin(115200);          // 启动序列监视器，设置波特率为115200
}

void loop() {
  int vibrationState = digitalRead(vibrationPin); // 读取震动感应器的状态

  if (vibrationState == HIGH) {
    Serial.println("震动感应到");
  } else {
    Serial.println("无震动");
  }

  delay(500); // 等待500毫秒再读取下一次
}
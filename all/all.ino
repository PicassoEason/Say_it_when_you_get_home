#include <ESP32Servo.h>
#include <WiFi.h> // 引入WiFi庫
#include <WebSocketsServer.h> // 引入WebSocketsServer庫
#include <SPIFFS.h> // 引入SPIFFS庫
#include <driver/i2s.h> // 引入I2S驅動庫
#define buttonPin 34; //關門按鈕
#define vibrationPin 33; //震動感測器
// INMP441設置
#define I2S_WS 25 // 定義I2S左右聲道選擇引腳
#define I2S_SD 33 // 定義I2S數據輸入引腳
#define I2S_SCK 32 // 定義I2S時鐘引腳
#define I2S_PORT I2S_NUM_0 // 定義I2S接口
#define I2S_SAMPLE_RATE (16000) // 定義I2S取樣率
#define I2S_SAMPLE_BITS (16) // 定義I2S取樣位數
#define I2S_READ_LEN (16 * 1024) // 定義I2S讀取長度
#define RECORD_TIME (5) // 錄音時間（秒）
Servo myservo;
int buttonState = HIGH;  // 初始狀態設為HIGH（未按下）
int lastButtonState = HIGH; // 前一次按鈕狀態
int vibrationState = 0; //震動感測器狀態
bool servoState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// WiFi設置
const char* ssid = "你的WiFi名稱"; // 設定WiFi名稱
const char* password   = "你的WiFi密碼"; // 設定WiFi密碼

// WebSocket設置
WebSocketsServer webSocket = WebSocketsServer(81); // 創建WebSocket伺服器對象，端口設置為81

// 緩衝區和文件名
uint8_t i2s_read_buff[I2S_READ_LEN]; // 定義I2S讀取緩衝區
const char filename[] = "/audio.wav"; // 定義音頻文件名

// 優化後的緩衝區大小
const size_t bufferSize = 4096; // 定義緩衝區大小

// WAV檔案頭結構
struct WAVHeader {
    char riff[4] = {'R', 'I', 'F', 'F'}; // RIFF標識
    uint32_t chunkSize; // 整個文件大小
    char wave[4] = {'W', 'A', 'V', 'E'}; // WAVE標識
    char fmt[4] = {'f', 'm', 't', ' '}; // fmt標識
    uint32_t subchunk1Size = 16; // 子塊大小
    uint16_t audioFormat = 1; // 音頻格式（PCM）
    uint16_t numChannels = 1; // 通道數（單聲道）
    uint32_t sampleRate = I2S_SAMPLE_RATE; // 取樣率
    uint32_t byteRate = I2S_SAMPLE_RATE * 2; // 每秒字節數（取樣率 * 通道數 * 每個樣本的字節數）
    uint16_t blockAlign = 2; // 塊對齊（通道數 * 每個樣本的字節數）
    uint16_t bitsPerSample = 16; // 每個樣本的位數
    char data[4] = {'d', 'a', 't', 'a'}; // data標識
    uint32_t subchunk2Size; // 數據塊大小
};



void setup() {
   Serial.begin(115200); // 設置串口通信速度為115200
  //servo
  myservo.attach(14);
  pinMode(buttonPin, INPUT);  // 不使用內部上拉，假設使用外部上拉電阻
  pinMode(vibrationPin, INPUT);
  Serial.begin(115200);
  Serial.println("System initialized");
  if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS初始化失敗"); // SPIFFS初始化失敗時打印錯誤信息
        return; // 結束setup函數
    }
  // 連接WiFi
    WiFi.begin(ssid, password); // 開始連接WiFi
    while (WiFi.status() != WL_CONNECTED) { // 等待WiFi連接
        delay(1000); // 每秒打印一次
        Serial.println("正在連接WiFi...");
    }
    Serial.println("WiFi已連接"); // WiFi連接成功後打印信息
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP()); // 打印本地IP地址
  
    // 啟動WebSocket伺服器
    webSocket.begin(); // 啟動WebSocket伺服器
    webSocket.onEvent(webSocketEvent); // 設置WebSocket事件回調函數
  
    // 初始化I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // 設置I2S為主模式和接收模式
        .sample_rate = I2S_SAMPLE_RATE, // 設置取樣率
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 設置取樣位數
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 設置為單聲道
        .communication_format = I2S_COMM_FORMAT_I2S, // 設置通訊格式為I2S
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // 設置中斷級別
        .dma_buf_count = 4, // 設置DMA緩衝區數量
        .dma_buf_len = 1024, // 設置DMA緩衝區長度
        .use_apll = false, // 不使用APLL
        .tx_desc_auto_clear = false, // 不自動清除發送描述符
        .fixed_mclk = 0 // 不使用固定主時鐘
    };
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL); // 安裝I2S驅動
  
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK, // 設置I2S時鐘引腳
        .ws_io_num = I2S_WS, // 設置I2S左右聲道選擇引腳
        .data_out_num = I2S_PIN_NO_CHANGE, // 不改變數據輸出引腳
        .data_in_num = I2S_SD // 設置I2S數據輸入引腳
    };
    i2s_set_pin(I2S_PORT, &pin_config); // 設置I2S引腳配置

}


void loop() {
  // put your main code here, to run repeatedly:
  //myservo
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
  webSocket.loop(); // 處理WebSocket事件
  
    // 每隔10秒錄音並發送
    static unsigned long lastRecordTime = 0;
    if (millis() - lastRecordTime > 10000) { // 如果距離上次錄音時間超過10秒
        recordAudioWAV(); // 錄音
        sendAudioWAV(); // 發送錄音
        lastRecordTime = millis(); // 記錄本次錄音時間
    }
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] 斷開連接!\n", num); // WebSocket斷開連接時打印信息
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] 連接來自 %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]); // WebSocket連接成功時打印信息
            }
            break;
        case WStype_TEXT:
            Serial.printf("收到文本消息: %s\n", payload); // 收到文本消息時打印消息內容
            break;
        case WStype_BIN:
            Serial.println("收到二進制消息"); // 收到二進制數據時打印信息
            break;
    }
}

void recordAudioWAV() {
    Serial.println("開始錄音..."); // 打印開始錄音信息
    File file = SPIFFS.open(filename, FILE_WRITE); // 打開文件進行寫入
    if(!file){
        Serial.println("無法打開文件進行寫入"); // 文件打開失敗時打印錯誤信息
        return;
    }
  
    WAVHeader header; // 創建WAV文件頭對象
    size_t headerSize = sizeof(WAVHeader); // 獲取WAV文件頭大小
    size_t wavSize = headerSize + RECORD_TIME * I2S_SAMPLE_RATE * 2; // 計算WAV文件總大小（16位=2字節）
    header.chunkSize = wavSize - 8; // 設置WAV文件塊大小
    header.subchunk2Size = wavSize - headerSize; // 設置數據塊大小
  
    file.write((const uint8_t*)&header, headerSize); // 寫入WAV文件頭
  
    int i2s_read_len = I2S_READ_LEN; // 設置I2S讀取長度
    size_t bytes_read; // 已讀取的字節數

    for(int i = 0; i < RECORD_TIME * I2S_SAMPLE_RATE / (I2S_READ_LEN / 2); i++){ // 循環讀取I2S數據
        i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY); // 從I2S讀取數據
        file.write(i2s_read_buff, i2s_read_len); // 寫入數據到文件
    }
  
    file.close(); // 關閉文件
    Serial.println("錄音完成"); // 打印錄音完成信息
    Serial.printf("已寫入 %u 字節\n", wavSize); // 打印寫入的字節數
}

void sendAudioWAV() {
    Serial.println("開始發送音頻..."); // 打印開始發送音頻信息
    File file = SPIFFS.open(filename, FILE_READ); // 打開文件進行讀取
    if(!file){
        Serial.println("無法打開文件進行讀取"); // 文件打開失敗時打印錯誤信息
        return;
    }
  
    uint8_t buffer[bufferSize]; // 定義緩衝區
    size_t bytesRead; // 已讀取的字節數
  
    while((bytesRead = file.read(buffer, bufferSize)) > 0){ // 循環讀取文件內容到緩衝區
        webSocket.broadcastBIN(buffer, bytesRead); // 發送二進制數據通過WebSocket
        delay(5); // 稍微減少延遲，因為每次發送的數據量更大了
    }
  
    file.close(); // 關閉文件
    Serial.println("音頻發送完成"); // 打印音頻發送完成信息
}


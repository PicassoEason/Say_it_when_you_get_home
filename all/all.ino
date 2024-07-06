#include <ESP32Servo.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <driver/i2s.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Audio.h>

#define BUTTON_PIN 34
#define VIBRATION_PIN 33
#define SERVO_PIN 14

// INMP441 設置
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE 16000
#define I2S_SAMPLE_BITS 16
#define I2S_READ_LEN (16 * 1024)
#define RECORD_TIME 5

#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

Servo myservo;
WebSocketsServer webSocket = WebSocketsServer(81);
Audio audio;

// WiFi設置
const char* ssid = "Era";
const char* password = "123321123";

// 全域變數
int buttonState = HIGH;
int lastButtonState = HIGH;
bool servoState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// 音頻相關
const char filename[] = "/audio.wav";
const size_t bufferSize = 4096;

struct WAVHeader {
    char riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunkSize;
    char wave[4] = {'W', 'A', 'V', 'E'};
    char fmt[4] = {'f', 'm', 't', ' '};
    uint32_t subchunk1Size = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels = 1;
    uint32_t sampleRate = I2S_SAMPLE_RATE;
    uint32_t byteRate = I2S_SAMPLE_RATE * 2;
    uint16_t blockAlign = 2;
    uint16_t bitsPerSample = 16;
    char data[4] = {'d', 'a', 't', 'a'};
    uint32_t subchunk2Size;
};

void setup() {
    Serial.begin(115200);
    Serial.println("System initialized");

    myservo.attach(SERVO_PIN);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(VIBRATION_PIN, INPUT);

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS初始化失敗");
        return;
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("正在連接WiFi...");
    }
    Serial.println("WiFi已連接");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());

    websocket_setup();
    voicer_setup();
}

void websocket_setup() {
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };
    i2s_set_pin(I2S_PORT, &pin_config);
}

void voicer_setup() {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(10000);
    audio.connecttohost("https://firebasestorage.googleapis.com/v0/b/home9-58f42.appspot.com/o/ddd16000hz16bp.wav?alt=media&token=c70a5283-9bca-4550-a141-80eb57e39c28");
}

void loop() {
    btn_loop();
    webSocket.loop();
    voice_loop();
    audio.loop();
}

void btn_loop() {
    int reading = digitalRead(BUTTON_PIN);
    
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            
            if (buttonState == LOW) {
                Serial.println("press");
                servoState = !servoState;
                myservo.write(servoState ? 180 : 0);
                Serial.println(servoState ? "舵機轉到180度" : "舵機轉到0度");
            } else {
                Serial.println("x");
            }
        }
    }

    lastButtonState = reading;
}

void voice_loop() {
    static unsigned long lastRecordTime = 0;
    if (millis() - lastRecordTime > 10000) {
        recordAudioWAV();
        sendAudioWAV();
        lastRecordTime = millis();
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] 斷開連接!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] 連接來自 %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
            }
            break;
        case WStype_TEXT:
            Serial.printf("收到文本消息: %s\n", payload);
            break;
        case WStype_BIN:
            Serial.println("收到二進制消息");
            break;
        default:
            break;
    }
}

void recordAudioWAV() {
    Serial.println("開始錄音...");
    File file = SPIFFS.open(filename, FILE_WRITE);
    if(!file){
        Serial.println("無法打開文件進行寫入");
        return;
    }

    WAVHeader header;
    size_t headerSize = sizeof(WAVHeader);
    size_t wavSize = headerSize + RECORD_TIME * I2S_SAMPLE_RATE * 2;
    header.chunkSize = wavSize - 8;
    header.subchunk2Size = wavSize - headerSize;

    file.write((const uint8_t*)&header, headerSize);

    int i2s_read_len = I2S_READ_LEN;
    size_t bytes_read;
    uint8_t i2s_read_buff[I2S_READ_LEN];

    for(int i = 0; i < RECORD_TIME * I2S_SAMPLE_RATE / (I2S_READ_LEN / 2); i++){
        i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        file.write(i2s_read_buff, i2s_read_len);
    }

    file.close();
    Serial.println("錄音完成");
    Serial.printf("已寫入 %u 字節\n", wavSize);
}

void sendAudioWAV() {
    Serial.println("開始發送音頻...");
    File file = SPIFFS.open(filename, FILE_READ);
    if(!file){
        Serial.println("無法打開文件進行讀取");
        return;
    }

    uint8_t buffer[bufferSize];
    size_t bytesRead;

    while((bytesRead = file.read(buffer, bufferSize)) > 0){
        webSocket.broadcastBIN(buffer, bytesRead);
        delay(5);
    }

    file.close();
    Serial.println("音頻發送完成");
}

// Audio callback functions
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}

void audio_id3data(const char *info){
    Serial.print("id3data     ");Serial.println(info);
}

void audio_eof_mp3(const char *info){
    Serial.print("eof_mp3     ");Serial.println(info);
}

void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}

void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}

void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}

void audio_commercial(const char *info){
    Serial.print("commercial  ");Serial.println(info);
}

void audio_icyurl(const char *info){
    Serial.print("icyurl      ");Serial.println(info);
}

void audio_lasthost(const char *info){
    Serial.print("lasthost    ");Serial.println(info);
}
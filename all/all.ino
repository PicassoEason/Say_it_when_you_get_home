#include <ESP32Servo.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <driver/i2s.h>
#include <WiFi.h>

#define BUTTON_PIN 34
#define SERVO_PIN 14
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE 16000
#define I2S_READ_LEN (8 * 1024)
#define RECORD_TIME 5

Servo myservo;
WebSocketsServer webSocket(81);

const char* ssid = "Era";
const char* password = "123321123";
const char filename[] PROGMEM = "/audio.wav";

void setup() {
    myservo.attach(SERVO_PIN);
    pinMode(BUTTON_PIN, INPUT);
    SPIFFS.begin(true);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    
    webSocket.begin();
    webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
        if (type == WStype_CONNECTED) {
            IPAddress ip = webSocket.remoteIP(num);
        }
    });

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
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

void loop() {
    static int lastButtonState = HIGH;
    static unsigned long lastDebounceTime = 0;
    static bool servoState = false;
    static unsigned long lastRecordTime = 0;

    int reading = digitalRead(BUTTON_PIN);
    if (reading != lastButtonState) lastDebounceTime = millis();

    if ((millis() - lastDebounceTime) > 50) {
        if (reading != lastButtonState && reading == LOW) {
            servoState = !servoState;
            myservo.write(servoState ? 180 : 0);
        }
    }
    lastButtonState = reading;

    if (millis() - lastRecordTime > 10000) {
        File file = SPIFFS.open(filename, FILE_WRITE);
        if(file) {
            struct {
                char header[44];
            } wavHeader = {{
                'R','I','F','F', 0,0,0,0, 'W','A','V','E',
                'f','m','t',' ', 16,0,0,0, 1,0, 1,0,
                0,0,0,0, 0,0,0,0, 2,0, 16,0,
                'd','a','t','a', 0,0,0,0
            }};
            uint32_t wavSize = sizeof(wavHeader) + RECORD_TIME * I2S_SAMPLE_RATE * 2;
            *(uint32_t*)(wavHeader.header + 4) = wavSize - 8;
            *(uint32_t*)(wavHeader.header + 24) = I2S_SAMPLE_RATE;
            *(uint32_t*)(wavHeader.header + 28) = I2S_SAMPLE_RATE * 2;
            *(uint32_t*)(wavHeader.header + 40) = wavSize - sizeof(wavHeader);

            file.write((const uint8_t*)&wavHeader, sizeof(wavHeader));

            uint8_t i2s_read_buff[I2S_READ_LEN];
            for(int i = 0; i < RECORD_TIME * I2S_SAMPLE_RATE / (I2S_READ_LEN / 2); i++){
                size_t bytes_read;
                i2s_read(I2S_PORT, i2s_read_buff, I2S_READ_LEN, &bytes_read, portMAX_DELAY);
                file.write(i2s_read_buff, I2S_READ_LEN);
            }
            file.close();

            file = SPIFFS.open(filename, FILE_READ);
            if(file) {
                uint8_t buffer[1024];
                size_t bytesRead;
                while((bytesRead = file.read(buffer, 1024)) > 0){
                    webSocket.broadcastBIN(buffer, bytesRead);
                    delay(5);
                }
                file.close();
            }
        }
        lastRecordTime = millis();
    }

    webSocket.loop();
}
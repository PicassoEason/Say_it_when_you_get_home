//device A : servo, sw420, LED, record
#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ESP32Servo.h>
#include <Ultrasonic.h>
#define WIFI_SSID "csie523"
#define WIFI_PASSWORD "MakeReality"
#define API_KEY "AIzaSyDOeXrq21A4-L4XrqD3owwkSY7O0AktqsE"
#define DATABASE_URL "https://home9-58f42-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "austin60616@gmail.com"
#define USER_PASSWORD "austin1204"
// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
Servo myservo;
unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif
#define servoPin 14
#define LEDpin 13
Ultrasonic ultrasonic(32,33);
int delayT = 1000;
int buttonState;  // 初始狀態設為HIGH（未按下）
int lastButtonState = LOW;
int vibrationState = 0;
int state = 1;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int distance;
void setup() {
  Serial.begin(115200);
 // pinMode(buttonPin, INPUT_PULLUP);  // 不使用內部上拉，假設使用外部上拉電阻
  // pinMode(vibrationPin, INPUT);
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW);
  myservo.attach(servoPin);
  myservo.write(0);
//wifi setting
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  multi.addAP(WIFI_SSID, WIFI_PASSWORD);
  multi.run();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    if (millis() - ms > 10000)
      break;
#endif
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
  Firebase.reconnectNetwork(true);
  // fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  // fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
}

void loop() {
  switch (state) {
    case 1:
      firebase_on_receive_message_from_A();
      break;

    case 2:
      firebase_on_B_close_door();
      break;

    case 3:
      firebase_on_A_close_door();
      break;
  }
}
//get string "a_open_door"
void firebase_on_receive_message_from_A() {
     if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();

      Serial.printf("Get string... %s\n", Firebase.RTDB.getString(&fbdo, F("/openhci/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
      if (Firebase.RTDB.getString(&fbdo, F("/openhci/string")) && fbdo.to<const char *>() == String("A_knock")) {
        digitalWrite(LEDpin, HIGH); // 將LEDpin設置為高電平
        myservo.write(90);
        state++;
      }

    }
     delay(20);  
}
//set String "b-push"
void firebase_on_B_close_door() {
  distance = ultrasonic.read();
  Serial.print("Distance in CM: ");
  Serial.println(distance);
  if (distance <=4) {

    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      // Get the string value from the specified Firebase endpoint
      Serial.println("b-push.........");
      Serial.printf("Set string... %s\n", Firebase.RTDB.setString(&fbdo, F("/openhci/string"), F("b_push")) ? "ok" : fbdo.errorReason().c_str());
      myservo.write(60);
      digitalWrite(LEDpin, LOW);

      state++;
    }
 
    //music
  }

  delay(10);  // 小延遲以穩定讀取
}
void firebase_on_A_close_door(){
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      Serial.printf("Get string... %s\n", Firebase.RTDB.getString(&fbdo, F("/openhci/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
      if (Firebase.RTDB.getString(&fbdo, F("/openhci/string")) && fbdo.to<const char *>() == String("a_close_door")) {
        myservo.write(0);
        state++;
      }
      
    }
  delay(20);     
}
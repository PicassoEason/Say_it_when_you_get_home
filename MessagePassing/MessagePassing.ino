#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Servo.h>
Servo myservo;
WiFiClientSecure espClient;  // Secure client for TLS connection
PubSubClient client(espClient);
const char* ssid = "TomatoBreeder";
const char* password = "11223344";
const char* mqtt_server = "8fea42e8822b44208274cce2c2f246bf.s1.eu.hivemq.cloud";
const char* mqtt_username = "austinjeng";
const char* mqtt_password = "Austin1204";
const int mqtt_port = 8883;
const int buttonPin = D1;  // 按鈕連接到D1引腳（GPIO5）
const int sensorPin = D7; // GPIO where your vibration sensor is connected 震動感測
const int ledPin = D6;
int sensorValue = 0; // Variable to store the sensor value 震動感測
int buttonState = 0;      // 用於儲存按鈕的狀態
int lastButtonState = 0;  // 用於儲存上一次按鈕的狀態
bool servoState = false;  // 用於記錄舵機當前的狀態
void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 

  digitalWrite(sensorPin, LOW);
  digitalWrite(ledPin, LOW);
  myservo.attach(D2);    
  setup_wifi();

  espClient.setInsecure(); // This is for testing purposes only. Do not use in production!

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Set the MQTT callback function

  reconnect(); 
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266ButtonClient", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Once connected, subscribe to the topic
      if (client.subscribe("ESP8266-2/vibration")) {
        Serial.println("Subscription successful");
      } else {
        Serial.println("Subscription failed");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;
  for (unsigned int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);
  
  // Handle the message received on the subscribed topic
  if (String(topic) == "ESP8266-2/vibration") {
    if (messageTemp == "Vibration detected") {
      digitalWrite(ledPin, HIGH);
      delay(1000); // Keep the LED on for 1 second
      digitalWrite(ledPin, LOW);
    }
  }
}

void loop() {
  buttonState = digitalRead(buttonPin); 

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (buttonState == LOW && lastButtonState == HIGH) {
    servoState = !servoState; 
    if (servoState) {
      myservo.write(180);     
      Serial.println("the servo motor turns to 180 degrees.");
    } else {
      myservo.write(0);       /
      Serial.println("the servo motor turns to 0 degrees.");
    }
    delay(50); // 消除按鈕抖動
  }
  sensorValue = digitalRead(sensorPin);
  if (sensorValue == HIGH) {
    myservo.write(90); 
    Serial.println("When vibration is detected, the servo motor turns to 90 degrees.");
    Serial.println("Vibration detected, publishing message");
    client.publish("ESP8266-1/vibration", "Vibration detected");
    client.publish("ESP8266-1/door_status", "opendoor");
    delay(1000); // Delay to prevent multiple rapid detections
  }
}

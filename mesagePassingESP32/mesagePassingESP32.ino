#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid = "TomatoBreeder";
const char* password = "11223344";
const char* mqtt_server = "8fea42e8822b44208274cce2c2f246bf.s1.eu.hivemq.cloud";
const char* mqtt_username = "austinjeng";
const char* mqtt_password = "Austin1204";
const int mqtt_port = 8883;

WiFiClientSecure espClient;  // Secure client for TLS connection
PubSubClient client(espClient);

const int sensorPin = 27; // GPIO where your vibration sensor is connected
const int ledPin = 26;
int sensorValue = 0; // Variable to store the sensor value

void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(sensorPin, LOW);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  setup_wifi();

  espClient.setInsecure(); // This is for testing purposes only. Do not use in production!

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Set the MQTT callback function

  reconnect(); // Ensure to connect and subscribe initially
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
    if (client.connect("ESP32ButtonClient", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Once connected, subscribe to the topic
      if (client.subscribe("ESP32-2/vibration")) {
        Serial.println("Subscription successful");
      } else {
        Serial.println("Subscription failed");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
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
  if (String(topic) == "ESP32-2/vibration") {
    if (messageTemp == "Vibration detected") {
      digitalWrite(ledPin, HIGH);
      delay(1000); // Keep the LED on for 1 second
      digitalWrite(ledPin, LOW);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensorValue = digitalRead(sensorPin);
  if (sensorValue == HIGH) {
    Serial.println("Vibration detected, publishing message");
    client.publish("ESP32-1/vibration", "Vibration detected");
    delay(1000); // Delay to prevent multiple rapid detections
  }
}

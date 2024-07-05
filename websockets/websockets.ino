#include <WiFi.h>
#include <WebSocketsClient.h>

// Replace with your network credentials
const char* ssid = "JENG_2.4GHz";
const char* password = "2000120400";

// WebSocket client
WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from server");
      break;
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      webSocket.sendTXT("Hello from ESP32");
      break;
    case WStype_TEXT:
      payload[length] = '\0'; // Ensure null-termination
      Serial.printf("Received from server: %s\n", (char*)payload);
      break;
    case WStype_BIN:
      Serial.printf("Received binary data from server, length: %u\n", length);
      break;
    case WStype_ERROR:
      Serial.println("Error from server");
      break;
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      // Fragmented messages, ignore for now
      break;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.print("Connected to WiFi, IP address: ");
  Serial.println(WiFi.localIP());

  // WebSocket settings - replace with your server IP and port
  webSocket.begin("192.168.0.107", 3000, "/"); // Replace with your server IP and port
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}

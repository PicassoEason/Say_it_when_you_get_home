#include <ESP32Servo.h>

const int vibrationSensorPin = 14; // Pin connected to SW420 digital output
const int servoPin = 16;           // Pin connected to servo motor signal

Servo myServo;                     // Create a Servo object
int degreeArray[10] = {0,20,40,60,80,100,120,140,160,180};
int myIndex = 0;
bool vibrationDetected = false;

void setup() {
  pinMode(vibrationSensorPin, INPUT);
  myServo.attach(servoPin);       // Attach the servo to the pin
  myServo.write(0);               // Set initial position of servo to 0 degrees
  Serial.begin(115200);
}

void loop() {
  // int sensorValue = digitalRead(vibrationSensorPin);

  // if (sensorValue == HIGH) {
  //   if (!vibrationDetected) {
  //     vibrationDetected = true;
  //     Serial.println("Vibration detected!");
  //     myServo.write(180);          // Turn servo to 180 degrees
  //     delay(1000);                // Hold position for 1 second
  //     myServo.write(0);           // Return servo to 0 degrees
  //   }
  // } else {
  //   vibrationDetected = false;    // Reset detection flag
  // }

  if(myIndex == 11)
  {
    myIndex = 0;
    delay(50);
  }

  myServo.write(degreeArray[myIndex]);
  myIndex++;
  delay(1000); // Short delay to debounce the sensor input
}

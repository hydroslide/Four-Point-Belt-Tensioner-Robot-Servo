#include <Servo.h>

// Declare servo and current sensor pins
const int servoPin = 9;
const int currentSensorPin = A0;

// Declare servo and current sensor constants
const int zeroPoint = 512;
const int sensitivity = 185;
const int currentThreshold = 1; // in amps
const int positionIncrement = 5; // in degrees

// Declare servo and current sensor variables
Servo servo;
int currentSensorValue;
float current;
int servoPosition = 0;

void setup() {
  // Initialize servo and current sensor
  servo.attach(servoPin);
  pinMode(currentSensorPin, INPUT);
}

void loop() {
  // Read current sensor value
  currentSensorValue = analogRead(currentSensorPin);

  // Calculate current
  current = (currentSensorValue - zeroPoint) / sensitivity;

  // Check if current exceeds threshold
  if (current > currentThreshold) {
    // Decrease servo position
    servoPosition -= positionIncrement;

    // Constrain servo position to 0-180 range
    servoPosition = constrain(servoPosition, 0, 180);

    // Set servo position
    servo.write(servoPosition);
  } else {
    // Increase servo position
    servoPosition += positionIncrement;

    // Constrain servo position to 0-180 range
    servoPosition = constrain(servoPosition, 0, 180);

    // Set servo position
    servo.write(servoPosition);
  }
}
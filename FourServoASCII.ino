// Multi Direct
// -> 4 servos
// <255><LeftBelt><127><127><RightBelt>
// Rig : Bit output -> 8 bits
// avec inversion
// PT Mover envoie de 0 à 255 par axe

/*Mover = output "Binary" et "10bits"
Arduino = Byte Data[2]
Data[0] = Serial.read();
Data[1] = Serial.read();
result = (Data[0] * 256 + Data[1]);

OU

Mover = output "Binary" et "8bits"
Arduino = Byte Data
Data = Serial.read(); on obtient directement le résultat*/

#include <Servo.h>  // local library "Servo.h"

const byte nbServos = 4;

// create servo objects to control any servo
Servo myServo[nbServos];
const byte servoPin[nbServos] = {3,5,6,9};  // digital pins (not necessarily ~pwm)
const byte inversion[nbServos] = {1, 1, 0, 0 }; // parameter to change if a servo goes the wrong way
int OldSerialValue[nbServos] = {0, 0, 0, 0};
int NewSerialValue[nbServos] = {0, 0, 0, 0};

// servo span:
int servoHomeDegrees[nbServos] = { 0, 0, 0, 0}; //will be updated with the initial pressure measurement
int servoMaxDegrees[nbServos] = { 180,180,180,180}; // leftthigh, rightthigh, leftside, rightside
int servoPositionTarget[nbServos] = {0, 0, 0, 0};

const byte deadZone = 0;

// =======================================
// Variables for info received from serial
// =======================================
//int bufferPrevious = 0;      // To hold previous read fom serial command
int bufferCurrent = 0;       // To hold current read fom serial command
int bufferCount = 0;         // To hold current position in bufferCommand array
// byte bufferCommand[2*nbServos] = {0};  // (*2 if 10 bits) To hold received info from serial
int bufferCommand[nbServos] = {0,0,0,0};  // To hold received info from serial

void setup()
{
  Serial.begin(115200); // opens serial port at specified baud rate

  // attach the Servos to the pins
  for (byte i = 0; i < nbServos; i++) {
    // pinMode(servoPin[i], OUTPUT); // done within the library
    myServo[i].attach(servoPin[i]);  // attaches the servo on servoPin pin
    }
  // move the servos to signal startup
  MoveAllServos255toDegrees(255); // Max
  delay(2000);
  // send all servos to home
  MoveAllServos255toDegrees(0);
  delay(2000);
  MoveAllServos255toDegrees(127);
}

void loop()
{
  // SerialValues contain the last order received (if there is no newer received, the last is kept)

  if (Serial.available())
  {
//    bufferPrevious = bufferCurrent; // Store previous byte
    bufferCurrent = Serial.read(); // Get the new byte
    bufferCommand[bufferCount] = bufferCurrent; // Put the new byte in the array
    bufferCount++; // Change to next position in the array
    //if (bufferCurrent == 255) bufferCount = 0; // one 255 is the start of the position info
    if (bufferCount == nbServos) //if 8 bits, nbServos // if 10 bits nbServos*2
      //Having reach buffer count, means we have the new positions and that we can update the aimed position
    {
      for (byte i = 0; i < nbServos; i++) {
        NewSerialValue[i] = bufferCommand[i];
        //NewSerialValue[i]= (bufferCommand[i*2] * 256) + bufferCommand[i*2+1]; // si 10 bits
      }
      bufferCount = 0;
    }
  }
  // Update orders sent to motor driver
  for (byte i = 0; i < nbServos; i++) {
    if (abs(OldSerialValue[i] - NewSerialValue[i]) > deadZone) {
      sendServoSetpoint255toDegrees(i, CheckForInversion(NewSerialValue[i], i));
      OldSerialValue[i] = NewSerialValue[i];
    }
  }
}

void sendServoSetpoint255toDegrees(byte servoID, int val )
{
  byte targetDegrees;
  val = constrain(val, 0, 255); // constrain cut above and below: clipping and not scaling (like map)
  // security to avoid cases where Simtools sends negative or above 255
  targetDegrees = map(val, 0, 255, servoHomeDegrees[servoID], servoMaxDegrees[servoID]);
  //  map(value, fromLow, fromHigh, toLow, toHigh)
  myServo[servoID].write(targetDegrees);              // tell servo to go to position in variable : in steps of 1 degree
  Serial.println("Servo ID: "+(String)servoID+", val: "+(String)val+", degrees: "+(String)targetDegrees);
}

int CheckForInversion(int val, int servoIndex){
  if (inversion[servoIndex] == 1)
    return 255 - val;
  else
    return val;
}

void MoveAllServos255toDegrees( int target)
{
  // send all servos to home
  for (byte i = 0; i < nbServos; i++) {
    sendServoSetpoint255toDegrees(i, CheckForInversion(target, i));
  }
}
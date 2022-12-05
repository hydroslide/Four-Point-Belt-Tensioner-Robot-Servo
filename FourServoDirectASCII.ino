

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

const byte leftShoulderCtl = 252;
const byte rightShoulderCtl = 253;
const byte leftWaistCtl = 254;
const byte rightWaistCtl = 255;

const byte maxVal = 251;

// create servo objects to control any servo
Servo myServo[nbServos];
const byte servoPin[nbServos] = {3,5,6,9};  // digital pins (not necessarily ~pwm)
const byte inversion[nbServos] = {1, 1, 0, 0 }; // parameter to change if a servo goes the wrong way
int OldSerialValue[nbServos] = {0, 0, 0, 0};
//int NewSerialValue[nbServos] = {0, 0, 0, 0};

// servo span:
int servoHomeDegrees[nbServos] = { 0, 0, 0, 0}; //will be updated with the initial pressure measurement
int servoMaxDegrees[nbServos] = { 180,180,180,180}; // leftthigh, rightthigh, leftside, rightside
int currentServoIndex = 0;

const byte deadZone = 0;



void setup()
{
  Serial.begin(115200); // opens serial port at specified baud rate

  // attach the Servos to the pins
  for (byte i = 0; i < nbServos; i++) {
    // pinMode(servoPin[i], OUTPUT); // done within the library
    myServo[i].attach(servoPin[i]);  // attaches the servo on servoPin pin
    }
  // move the servos to signal startup
  MoveAllServosMaxtoDegrees(maxVal); // Max
  delay(2000);
  // send all servos to home
  MoveAllServosMaxtoDegrees(0);
  delay(2000);
  MoveAllServosMaxtoDegrees(127);
}

void loop()
{
  // SerialValues contain the last order received (if there is no newer received, the last is kept)

  if (Serial.available())
  {
//    bufferPrevious = bufferCurrent; // Store previous byte
    byte bufferCurrent = Serial.read(); // Get the new byte
    if (bufferCurrent > maxVal){
      currentServoIndex = bufferCurrent-(maxVal+1);
      Debug("Got a control character: "+(String)bufferCurrent+", currentServoIndex: "+(String)currentServoIndex);
    }
    else
    {   
      Debug("Got a value. Gonna set servo: "+(String)currentServoIndex+" to "+(String)bufferCurrent);
      //NewSerialValue[currentServoIndex] = bufferCurrent;
      if (abs(OldSerialValue[currentServoIndex] - bufferCurrent) > deadZone) {
      sendServoSetpointMaxtoDegrees(currentServoIndex, CheckForInversion(bufferCurrent, currentServoIndex));
      OldSerialValue[currentServoIndex] = bufferCurrent;
    }
    }
  }
  
}

void sendServoSetpointMaxtoDegrees(byte servoID, int val )
{
  byte targetDegrees;
  val = constrain(val, 0, maxVal); // constrain cut above and below: clipping and not scaling (like map)

  targetDegrees = map(val, 0, maxVal, servoHomeDegrees[servoID], servoMaxDegrees[servoID]);
  //  map(value, fromLow, fromHigh, toLow, toHigh)
  myServo[servoID].write(targetDegrees);              // tell servo to go to position in variable : in steps of 1 degree
  Debug("Servo ID: "+(String)servoID+", val: "+(String)val+", degrees: "+(String)targetDegrees);
}

int CheckForInversion(int val, int servoIndex){
  if (inversion[servoIndex] == 1)
    return maxVal - val;
  else
    return val;
}

void MoveAllServosMaxtoDegrees( int target)
{
  // send all servos to home
  for (byte i = 0; i < nbServos; i++) {
    sendServoSetpointMaxtoDegrees(i, CheckForInversion(target, i));
  }
}

bool shouldDebug = true;

void Debug(String msg){
  if (shouldDebug)
    Serial.println(msg);
}
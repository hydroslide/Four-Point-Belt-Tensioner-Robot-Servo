#include <Servo.h>  // local library "Servo.h"

bool shouldDebug = true;

const byte nbServos = 4;

const byte leftShoulderCtl = 0;
const byte rightShoulderCtl = 1;
const byte leftWaistCtl = 2;
const byte rightWaistCtl = 3;
const byte above127Ctl = 4;

const byte maxVal = 250;

bool isAbove127 = false;

// create servo objects to control any servo
Servo myServo[nbServos];
const byte servoPin[nbServos] = {5,6,3,4};  // digital pins (not necessarily ~pwm)
const byte inversion[nbServos] = {0, 0, 0, 0 }; // parameter to change if a servo goes the wrong way
int OldSerialValue[nbServos] = {0, 0, 0, 0};
//int NewSerialValue[nbServos] = {0, 0, 0, 0};

// servo span:
int servoHomeDegrees[nbServos] = { 0, 0, 0, 0}; //will be updated with the initial pressure measurement
int servoMaxDegrees[nbServos] = { 180,180,180,180}; // leftthigh, rightthigh, leftside, rightside
int currentServoIndex = 0;

const byte deadZone = 2;



void setup()
{
  Serial.begin(115200); // opens serial port at specified baud rate

  // attach the Servos to the pins
  for (byte i = 0; i < nbServos; i++) {
    // pinMode(servoPin[i], OUTPUT); // done within the library
    myServo[i].attach(servoPin[i],500,2500);  // attaches the servo on servoPin pin
    }
  // move the servos to signal startup
  MoveAllServosMaxtoDegrees(maxVal); // Max
  delay(1000);
  // send all servos to home
  MoveAllServosMaxtoDegrees(0);
  delay(1000);

  MoveAllServosMaxtoDegrees(50);
  delay(1000);
}

void loop()
{
  // SerialValues contain the last order received (if there is no newer received, the last is kept)

  if (Serial.available())
  {
//    bufferPrevious = bufferCurrent; // Store previous byte
    byte bufferCurrent = Serial.read(); // Get the new byte

    if (isAbove127 == true){        
        Debug("Got a Buffer: "+(String)bufferCurrent+" isAbove127 is set. New Value: "+(String)(bufferCurrent + 128));
        bufferCurrent += 128;
        isAbove127 = false;
    }

    if (bufferCurrent <= 4){
      if (bufferCurrent == above127Ctl){
        Debug("Got a control character: "+(String)bufferCurrent+". Setting Above 127 to true");
        isAbove127 = true;
      }
      else{
        currentServoIndex = bufferCurrent;
        Debug("Got a control character: "+(String)bufferCurrent+", currentServoIndex: "+(String)currentServoIndex);
      }
    }
    else
    { 
      bufferCurrent -= 5;        
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

void Debug(String msg){
  if (shouldDebug)
    Serial.println(msg);
}
#include <Servo.h>  // local library "Servo.h"
#include "LEDFunctions.h"

LEDFunctions leds;

bool shouldDebug = true;

const byte nbServos = 4;


// COntrol Codes
const byte above127Ctl = 0;
const byte controlCtl = 1;
//const byte valueCtl = 2;
const byte leftShoulderCtl = 3;
const byte rightShoulderCtl = 4;
const byte leftWaistCtl = 5;
const byte rightWaistCtl = 6;
const byte forceSleepCtl = 7;
const byte setRedCtl = 8;
const byte setGreenCtl = 9;
const byte setBlueCtl = 10;

const byte maxVal = 253;

bool isAbove127 = false;

// create servo objects to control any servo
Servo myServo[nbServos];
const byte servoPin[nbServos] =  {4,7,2,8};//{2,4,7,8};  // digital pins (not necessarily ~pwm)
const byte inversion[nbServos] = {0, 0, 0, 0}; // parameter to change if a servo goes the wrong way
int OldSerialValue[nbServos] = {0, 0, 0, 0};
//int NewSerialValue[nbServos] = {0, 0, 0, 0};

// servo span:
int servoHomeDegrees[nbServos] = { 0, 0, 2, 0}; //will be updated with the initial pressure measurement
int servoMaxDegrees[nbServos] = { 175,179,179,175}; // leftthigh, rightthigh, leftside, rightside
int currentServoIndex = 0;

const int servoOffset[nbServos] = {-2,-2,1,-2};

int currentColorIndex = 0;

const byte deadZone = 2;

bool expectControl = false;
bool expectServoValue = false;
bool expectRGBValue = false;


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

  leds.setup();
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

    if (bufferCurrent == above127Ctl){
        Debug("Got a control character: "+(String)bufferCurrent+". Setting Above 127 to true");
        isAbove127 = true;
    }else if (bufferCurrent == controlCtl){
      expectControl =true;
    }else if (expectControl){
      expectControl = false;
      // TODO: set bools based on control codes
      if (bufferCurrent >= leftShoulderCtl && bufferCurrent <= rightWaistCtl){
        expectServoValue=true;
        currentServoIndex = bufferCurrent-leftShoulderCtl;
        Debug("Got a control character: "+(String)bufferCurrent+", currentServoIndex: "+(String)currentServoIndex);
      } else if (bufferCurrent >= setRedCtl && bufferCurrent <= setBlueCtl){
        expectRGBValue = true;
        currentColorIndex = bufferCurrent-setRedCtl;
        Debug("Got a control character: "+(String)bufferCurrent+", currentServoIndex: "+(String)currentColorIndex);
      }
    }else { 
      bufferCurrent -= (255-maxVal); 
      if (expectServoValue){  
        expectServoValue=false;     
        Debug("Got a value. Gonna set servo: "+(String)currentServoIndex+" to "+(String)bufferCurrent);
        //NewSerialValue[currentServoIndex] = bufferCurrent;
        if (abs(OldSerialValue[currentServoIndex] - bufferCurrent) > deadZone) {
          sendServoSetpointMaxtoDegrees(currentServoIndex, CheckForInversion(bufferCurrent, currentServoIndex));
          OldSerialValue[currentServoIndex] = bufferCurrent;
        }
      }else if (expectRGBValue){
        expectRGBValue = false;
        // TODO: Set the correct pins for RGB.
      }
    }
  }
  leds.loop();
}

void sendServoSetpointMaxtoDegrees(byte servoID, int val )
{
  int targetDegrees;
  val = constrain(val, 0, maxVal); // constrain cut above and below: clipping and not scaling (like map)

  targetDegrees = map(val, 0, maxVal, servoHomeDegrees[servoID], servoMaxDegrees[servoID]);
  //targetDegrees +=servoOffset[servoID];

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
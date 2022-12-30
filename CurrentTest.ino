#include <Servo.h>  // local library "Servo.h"
#include "CurrentMonitor.h"



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
const byte setRedLeftCtl = 8;
const byte setGreenLeftCtl = 9;
const byte setBlueLeftCtl = 10;
const byte setRedRightCtl = 11;
const byte setGreenRightCtl = 12;
const byte setBlueRightCtl = 13;
const byte leftShoulderNeutralCtl = 14;
const byte rightShoulderNeutralCtl = 15;
const byte leftWaistNeutralCtl = 16;
const byte rightWaistNeutralCtl = 17;

const byte maxVal = 253;

bool isAbove127 = false;

// create servo objects to control any servo
Servo myServo[nbServos];
const byte servoPin[nbServos] =  {10,8,4,7};//{4,7,2,8};  // digital pins (not necessarily ~pwm)
const byte inversion[nbServos] = {0, 0, 0, 0}; // parameter to change if a servo goes the wrong way
int OldSerialValue[nbServos] = {0, 0, 0, 0};
//int NewSerialValue[nbServos] = {0, 0, 0, 0};

// servo span:
int servoHomeDegrees[nbServos] = { 0, 0, 2, 0}; //will be updated with the initial pressure measurement
int servoMaxDegrees[nbServos] = { 175,179,179,175}; // leftthigh, rightthigh, leftside, rightside
int currentServoIndex = 0;

int servoNativeDegrees[nbServos] = { 180,270,180,270}; 

const int currentSensorPins[nbServos] = {A0,A1,A2,A3};

const int milliVoltsPerAmp = 100;
const int ACSOffset = 2500;

int currentColorIndex = 0;

const byte deadZone = 2;

bool expectControl = false;
bool expectServoValue = false;
bool expectRGBValue = false;

long previousTime=0;

CurrentMonitor* _currentMonitor;
double _currentThreshold = 3.0;
long _overCurrentTimeout = 1500;

void setup()
{
    shouldDebug=false;

  _currentMonitor = new CurrentMonitor(nbServos,  currentSensorPins, _currentThreshold, _overCurrentTimeout);
  Serial.begin(115200); // opens serial port at specified baud rate

  _currentMonitor->setup();

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

  //leds.setup();
}

int step = 0;

byte cycle(){

    byte val = 0;
    switch(step){
        case 0:
            val= 64;
            break;
        case 1:
            val= 192;
            break;
        case 2:
            val= 64;
            break;
        case 3:
            val= 253;
            break;
    }
        step++;
    if (step>3)
      step=0;
    return val;
}


int cycleInterval = 5000;
int msSinceCycle = 0;

int nominalInterval = 100;
int msSinceNominal = 0;

void loop()
{
  long delta = timeDelta();

    byte id = 0;
  if (Serial.available()){
    Serial.read();
    byte val = cycle();
    sendServoSetpointMaxtoDegrees(id,val);
  }

  if (msSinceCycle >= cycleInterval){
    msSinceCycle=0;
    byte val = cycle();
    sendServoSetpointMaxtoDegrees(id,val);
  }else
    msSinceCycle+=delta;


  _currentMonitor->loop();
  bool* nominal = _currentMonitor->isEverythingNominal();
  printNominal(nominal, delta);  
  //printNominalOnDemand(delta);
  delete[] nominal;
}

void printNominal(bool* nominal, long delta){
 if (msSinceNominal >= nominalInterval){
    msSinceNominal=0;
    for(byte i=0; i<nbServos; i++){      
      if (i>0)
        Serial.print(",");
      Serial.print("Nominal_"+(String)i+":");
      Serial.print(nominal[i]);
    }
    Serial.println();
  }else
    msSinceNominal+=delta;
}

void printNominalOnDemand(long delta){
 if (msSinceNominal >= nominalInterval){
    msSinceNominal=0;
    bool* nominal = _currentMonitor->onDemandOverCurrentCheck();
    for(byte i=0; i<nbServos; i++){      
      if (i>0)
        Serial.print(",");
      Serial.print("ODNominal_"+(String)i+":");
      Serial.print(nominal[i]);
    }
    Serial.println();
    delete[] nominal;
  }else
    msSinceNominal+=delta;
}



void sendServoSetpointMaxtoDegrees(byte servoID, int val )
{
  int targetDegrees;
  val = constrain(val, 0, maxVal); // constrain cut above and below: clipping and not scaling (like map)

  targetDegrees = map(val, 0, maxVal, servoHomeDegrees[servoID], servoMaxDegrees[servoID]);


  //Translate the 180 degree range into the equivalent based on the native range. 
  double nativeRatio = (double)180/(double)servoNativeDegrees[servoID];
  targetDegrees = (int)((double)targetDegrees*nativeRatio);

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

long timeDelta(){
  unsigned long currentTime = millis();
   long delta = (long)(currentTime-previousTime);
  previousTime = currentTime;
  return delta;
}
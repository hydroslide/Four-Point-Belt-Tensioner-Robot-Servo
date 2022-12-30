#include <Servo.h>  // local library "Servo.h"
//#include "LEDFunctions.h"
#include "CurrentMonitor.h"

//LEDFunctions leds;

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
int intendedDegrees[nbServos] = {0, 0, 0, 0};

// servo span:
int servoHomeDegrees[nbServos] = { 0, 0, 2, 0}; //will be updated with the initial pressure measurement
int servoEndDegrees[nbServos] = { 175,179,179,175}; // leftthigh, rightthigh, leftside, rightside
int currentServoIndex = 0;

int servoNativeDegrees[nbServos] = { 270,270,180,270}; 

int servoNeutralDegrees[nbServos] = {0,0,0,0};
int servoCurrentMinDegrees[nbServos] = { 0, 0, 0, 0}; //will be updated with the initial pressure measurement
int servoCurrentMaxDegrees[nbServos] = { 0,0,0,0};

const int currentSensorPins[nbServos] = {A0,A1,A2,A3};


int currentColorIndex = 0;

const byte deadZone = 2;

bool expectControl = false;
bool expectServoValue = false;
bool expectRGBValue = false;

long previousTime=0;

CurrentMonitor* _currentMonitor;
double _currentThreshold = 3.0;
long _overCurrentTimeout = 700;
int overCurrentDegreeDelta = 1;
int overCurrentDelay=200;
int postNotNominalDelay = 3000;

void setup()
{
  _currentMonitor = new CurrentMonitor(nbServos,  currentSensorPins, _currentThreshold, _overCurrentTimeout);

  Serial.begin(115200); // opens serial port at specified baud rate

  _currentMonitor->setup();

  // attach the Servos to the pins
  for (byte i = 0; i < nbServos; i++) {
    // pinMode(servoPin[i], OUTPUT); // done within the library
    myServo[i].attach(servoPin[i],500,2500);  // attaches the servo on servoPin pin
    }
  // move the servos to signal startup
  MoveAllServosMaxtoDegrees(100); // Max
  delay(1000);
  // send all servos to home
  MoveAllServosMaxtoDegrees(0);
  delay(1000);

  MoveAllServosMaxtoDegrees(50);
  delay(1000);

  //leds.setup();
}

// ***************** Begin Test Code ********************
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
bool doCycle=false;

void loopCycle(long delta){
  if (doCycle){
    byte id = 0;
    if (msSinceCycle >= cycleInterval){
      msSinceCycle=0;
      byte val = cycle();
      sendServoSetpointMaxtoDegrees(id,val);
    }else
      msSinceCycle+=delta;
  }
}
// ***************** End  Test  Code ********************

void loop()
{
  long delta = timeDelta();


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
      } else if (bufferCurrent >= setRedLeftCtl && bufferCurrent <= setBlueRightCtl){
        expectRGBValue = true;
        currentColorIndex = bufferCurrent-setRedLeftCtl;
        Debug("Got a control character: "+(String)bufferCurrent+", currentColorIndex: "+(String)currentColorIndex);
      }
    }else { 
      bufferCurrent -= (255-maxVal); 
      if (expectServoValue){  
        expectServoValue=false;     
        Debug("Got a value. Gonna set servo: "+(String)currentServoIndex+" to "+(String)bufferCurrent);
        
        if (abs(OldSerialValue[currentServoIndex] - bufferCurrent) > deadZone) {
          sendServoSetpointMaxtoDegrees(currentServoIndex, CheckForInversion(bufferCurrent, currentServoIndex));
          OldSerialValue[currentServoIndex] = bufferCurrent;
        }
      }else if (expectRGBValue){
        Debug("Got a value. Gonna set LED: "+(String)currentColorIndex+" to "+(String)bufferCurrent);
        expectRGBValue = false;
        //leds.SetColorLevel(currentColorIndex,bufferCurrent);
      }
    }
  }
  //leds.LedLoop();

  bool wasntNominal = monitorCurrents(delta);
  if (wasntNominal)
    Serial.flush();

  loopCycle(delta);
}


bool monitorCurrents(long delta){
  _currentMonitor->loop();
  bool* nominal = _currentMonitor->isEverythingNominal();
  bool notNominal = false;
  bool wasntNominal = false;
  printNominal(nominal,delta);
  do{
    notNominal = false;
    // todo change back to nbServos
    for(int i=0; i<1; i++){
      if (!nominal[i]){
        Serial.println("Not Nominal: "+String(i));
        notNominal=true;
        wasntNominal=true;
        int newDegrees = intendedDegrees[i];
        if (newDegrees> servoNeutralDegrees[i]){
         newDegrees -=overCurrentDegreeDelta;
         Serial.println("Too far. New Degrees: "+(String)newDegrees);
         servoCurrentMaxDegrees[i]= newDegrees;
        }else{
         newDegrees+=overCurrentDegreeDelta;
         Serial.println("Too close. New Degrees: "+(String)newDegrees);
         servoCurrentMinDegrees[i]=newDegrees;
        }
        moveServoToDegrees(i,newDegrees);
      }
    }
    if(notNominal){
      Serial.println("Not Nominal. Gonna wait "+(String)overCurrentDelay+"ms before checking again");
      delay(overCurrentDelay);
      nominal = _currentMonitor->onDemandOverCurrentCheck();
      printNominal(nominal,timeDelta());
    }
  }while(notNominal);
  if (wasntNominal){
    Serial.println("Wasn't Nominal, but AG now!!!");
    delay(postNotNominalDelay);
  }
  return wasntNominal;
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

int mapValueToDegrees(byte servoID, int val ){

  val = constrain(val, 0, maxVal); // constrain cut above and below: clipping and not scaling (like map)

  return = map(val, 0, maxVal, servoHomeDegrees[servoID], servoEndDegrees[servoID]);

}

void sendServoSetpointMaxtoDegrees(byte servoID, int val )
{
  int targetDegrees mapValueToDegrees(servoID,val);

  int neutralDegrees =servoNeutralDegrees[i];
  if (neutralDegrees!=0 && servoCurrentMaxDegrees[i]!=0){
    if (targetDegrees < neutralDegrees)
      targetDegrees = map(targetDegrees,0, neutralDegrees,servoCurrentMinDegrees[i],neutralDegrees);
    else
      targetDegrees = map(targetDegrees, neutralDegrees, neutralDegrees, servoCurrentMaxDegrees[i]);
  }

  moveServoToDegrees(servoID, targetDegrees);
  
}

void moveServoToDegrees(byte servoID, int targetDegrees){
  //Translate the 180 degree range into the equivalent based on the native range. 
  double nativeRatio = (double)180/(double)servoNativeDegrees[servoID];
  targetDegrees = (int)((double)targetDegrees*nativeRatio);

  myServo[servoID].write(targetDegrees);              // tell servo to go to position in variable : in steps of 1 degree
  intendedDegrees[servoID]=targetDegrees;
  Debug("Servo ID: "+(String)servoID+", degrees: "+(String)targetDegrees); //val: "+(String)val+",
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
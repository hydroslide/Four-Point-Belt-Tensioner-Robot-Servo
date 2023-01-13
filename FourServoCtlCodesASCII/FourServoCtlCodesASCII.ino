#include <Servo.h>  // local library "Servo.h"
//#include "LEDFunctions.h"
#include "CurrentMonitor.h"

//LEDFunctions leds;

bool shouldDebug = false;
bool testMode=true;
bool shouldPrintNominal = true;
bool shouldPrintAmps = true;
bool currentAutocorrectEnabled = true;
const byte testStepSize=10;

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
const byte resetCurrentDegreesCtl = 18;
const byte advanceCycleCtl = 61;

const byte maxVal = 253;

bool isAbove127 = false;

// create servo objects to control any servo
Servo myServo[nbServos];
const byte servoPin[nbServos] =  {10,8,7,4};//{4,7,2,8};  // digital pins (not necessarily ~pwm)
const byte inversion[nbServos] = {0, 0, 0, 0}; // parameter to change if a servo goes the wrong way
int OldSerialValue[nbServos] = {0, 0, 0, 0};
int intendedDegrees[nbServos] = {0, 0, 0, 0};

// servo span:
int servoHomeDegrees[nbServos] = { 5, 5, 5, 5}; //will be updated with the initial pressure measurement
int servoEndDegrees[nbServos] = { 170,170,170,170}; // leftthigh, rightthigh, leftside, rightside
int currentServoIndex = 0;

int servoNativeDegrees[nbServos] = { 270,270,270,270}; 

int servoNeutralDegrees[nbServos] = {90,90,90,90};
int servoCurrentMinDegrees[nbServos] = { 0, 0, 0, 0}; //Based on known physical restrictions in available range of movement.
int servoCurrentMaxDegrees[nbServos] = { 0,0,0,0};

const int currentSensorPins[nbServos] = {A0,A1,A2,A3};

bool doCycle=false;
bool servoEnabled[nbServos] = { true,true,true,true};

int currentColorIndex = 0;

const byte deadZone = 2;

bool expectControl = false;
bool expectServoValue = false;
bool expectRGBValue = false;
bool expectNeutralValue = false;

long previousTime=0;

CurrentMonitor* _currentMonitor;
double _currentThreshold = 2.0;
long _overCurrentTimeout = 500;
int overCurrentDegreeDelta = 3;
long overCurrentDelay=200;
long postNotNominalDelay = 100;


void setup()
{
  _currentMonitor = new CurrentMonitor(nbServos,  currentSensorPins, _currentThreshold, _overCurrentTimeout, shouldPrintAmps);

    Serial.begin(115200); // opens serial port at specified baud rate

  _currentMonitor->setup();

  // attach the Servos to the pins
  for (byte i = 0; i < nbServos; i++) {
    // pinMode(servoPin[i], OUTPUT); // done within the library
    myServo[i].attach(servoPin[i],500,2500);  // attaches the servo on servoPin pin
    }
    
  // move the servos to signal startup
  MoveAllServosMaxtoDegrees(0); // Max
  delay(1000);
  // send all servos to home
  for (byte i = 0; i < nbServos; i++) {    
    moveServoToDegrees(i,servoNeutralDegrees[i]);
    OldSerialValue[i]=127;
  }
  // MoveAllServosMaxtoDegrees(20);
  // delay(1000);

  // MoveAllServosMaxtoDegrees(50);
  // delay(1000);

  //leds.setup();


}

// ***************** Begin Test Code ********************
struct ABC {
  int a = 97;
  int b = 98;
  int c = 99;
  int d = 100;
  int e = 101;
  int f = 102;
  int g = 103;
  int h = 104;
  int i = 105;
  int j = 106;
  int k = 107;
  int l = 108;
  int m = 109;
  int n = 110;
  int o = 111;
  int p = 112;
  int q = 113;
  int r = 114;
  int s = 115;
  int t = 116;
  int u = 117;
  int v = 118;
  int w = 119;
  int x = 120;
  int y = 121;
  int z = 122;
};


ABC abc;


int step = 0;

byte cycle(){

    byte val = 0;
    switch(step){
        case 0:
            val= 64;
            break;
        case 1:
            val= 100;
            break;
        case 2:
            val= 64;
            break;
        case 3:
            val= 192;
            break;
    }
        step++;
    if (step>3)
      step=0;
    return val;
}

bool servoIsEnabled(int id){
  // TODO: restore this to just return true
  //return true;
  return servoEnabled[id];
}


int cycleInterval = 10000;
int msSinceCycle = 0;
int nominalInterval = 100;
int msSinceNominal = 0;


void loopCycle(long delta){
  if (doCycle){   
    if (msSinceCycle >= cycleInterval){
      msSinceCycle=0;
      performCycleTest();
    }else
      msSinceCycle+=delta;
  }
}

void performCycleTest(){
  byte val = cycle();
       for(int i = 0; i<nbServos; i++){
        if (servoIsEnabled(i)){
          MoveServoToByteValue(i, val);
        }
      }      
}


void performTest(byte buffer){
  Debug("In Test Mode. Got buffer "+(String)buffer);
  int servo;
  int val;
  if (buffer == abc.q || buffer == abc.a || buffer == abc.z)
    servo = 0;
  if (buffer == abc.w || buffer == abc.s || buffer == abc.x)
    servo = 1;
  if (buffer == abc.e || buffer == abc.d || buffer == abc.c)
    servo = 2;
  if (buffer == abc.r || buffer == abc.f || buffer == abc.v)
    servo = 3;   

  if (buffer == abc.q || buffer == abc.w || buffer == abc.e || buffer == abc.r)
    val = 0;//64;
  if (buffer == abc.a || buffer == abc.s || buffer == abc.d || buffer == abc.f)
    val = max(0,OldSerialValue[servo]-testStepSize);
  if (buffer == abc.z || buffer == abc.x || buffer == abc.c || buffer == abc.v)
    val = min(OldSerialValue[servo]+testStepSize,maxVal);//255;//192;
  OldSerialValue[servo]=val;
  MoveServoToByteValue(servo, val);         
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
    if (testMode){
      performTest(bufferCurrent);
      return;
    }

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
        byte tempServo = bufferCurrent-leftShoulderCtl;
        if (!servoIsEnabled(tempServo))
          return;
        currentServoIndex=tempServo;
        expectServoValue=true;
        Debug("Got a control character: "+(String)bufferCurrent+", currentServoIndex: "+(String)currentServoIndex);
      } else if (bufferCurrent >= setRedLeftCtl && bufferCurrent <= setBlueRightCtl){
        expectRGBValue = true;
        currentColorIndex = bufferCurrent-setRedLeftCtl;
        Debug("Got a control character: "+(String)bufferCurrent+", currentColorIndex: "+(String)currentColorIndex);
      } else if (bufferCurrent >= leftShoulderNeutralCtl && bufferCurrent <= rightWaistNeutralCtl){
        currentServoIndex = bufferCurrent-leftShoulderNeutralCtl;
        expectNeutralValue=true;
      }else if (bufferCurrent == resetCurrentDegreesCtl){
        for (int i=0; i<nbServos; i++){
          servoCurrentMaxDegrees[i]=0;
          servoCurrentMinDegrees[i]=0;
        }        
        Debug("Min and Max current values reset");
      }
    }else { 
      bufferCurrent -= (255-maxVal); 
      if (expectServoValue){  
        expectServoValue=false;     
        Debug("Got a value. Gonna set servo: "+(String)currentServoIndex+" to "+(String)bufferCurrent);
        
        if (abs(OldSerialValue[currentServoIndex] - bufferCurrent) > deadZone) {
          MoveServoToByteValue(currentServoIndex, bufferCurrent);
          OldSerialValue[currentServoIndex] = bufferCurrent;
        }
      }else if (expectRGBValue){
        Debug("Got a value. Gonna set LED: "+(String)currentColorIndex+" to "+(String)bufferCurrent);
        expectRGBValue = false;
        //leds.SetColorLevel(currentColorIndex,bufferCurrent);
      }else if (expectNeutralValue){
        Debug("Got a NEUTRAL value. Gonna set servo neutral value: "+(String)currentServoIndex+" to "+(String)bufferCurrent);
        servoNeutralDegrees[currentServoIndex] = mapValueToDegrees(currentServoIndex, bufferCurrent);
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
  printNominal(nominal,delta);
  bool notNominal = false;
  bool wasntNominal = false;
  if (currentAutocorrectEnabled){
    do{
      notNominal = false;
      // todo change back to nbServos
      for(int i=0; i<nbServos; i++){
        if (!servoIsEnabled(i))
          continue;
        if (!nominal[i]){
          Debug("Not Nominal: "+String(i));
          notNominal=true;
          wasntNominal=true;
          int newDegrees = intendedDegrees[i];
          if (newDegrees> servoNeutralDegrees[i]){
            newDegrees -=overCurrentDegreeDelta;
            Debug("Too far. New Degrees: "+(String)newDegrees);
            servoCurrentMaxDegrees[i]= newDegrees;
          }else{
            newDegrees+=overCurrentDegreeDelta;
            Debug("Too close. New Degrees: "+(String)newDegrees);
            servoCurrentMinDegrees[i]=newDegrees;
          }
          moveServoToDegrees(i,newDegrees);
        }
      }
      if(notNominal){
        Debug("Not Nominal. Gonna wait "+(String)overCurrentDelay+"ms before checking again");
        delay(overCurrentDelay);
        nominal = _currentMonitor->onDemandOverCurrentCheck(overCurrentDelay);
        printNominal(nominal,timeDelta());
      }
    }while(notNominal);
    if (wasntNominal){
      Debug("Wasn't Nominal, but AG now!!!");
      delay(postNotNominalDelay);
    }
  }
  return wasntNominal;
}

void printNominal(bool* nominal, long delta){
 if (msSinceNominal >= nominalInterval){
    msSinceNominal=0;
    if (shouldPrintNominal){
      for(byte i=0; i<nbServos; i++){      
        if (i>0)
          Serial.print(",");
        Serial.print("Nominal_"+(String)i+":");
        Serial.print(nominal[i]);
      }
      Serial.println();
    }
  }else
    msSinceNominal+=delta;
}

int mapValueToDegrees(byte servoID, int val ){

  val = constrain(val, 0, maxVal); // constrain cut above and below: clipping and not scaling (like map)

  return map(val, 0, maxVal, servoHomeDegrees[servoID], servoEndDegrees[servoID]);

}

void MoveServoToByteValue(byte servoID, int val )
{
  val = CheckForInversion(val, servoID);
  int targetDegrees = mapValueToDegrees(servoID,val);
  Debug("Attempting to move servo "+(String)servoID+" to val "+(String)val+" at targetDegrees "+(String)targetDegrees);

  int neutralDegrees =servoNeutralDegrees[servoID];
  if (neutralDegrees!=0){
    int newDegrees = targetDegrees;
    if (targetDegrees < neutralDegrees){
      if (servoCurrentMinDegrees[servoID]!=0){        
        newDegrees = map(targetDegrees,servoHomeDegrees[servoID], neutralDegrees, servoCurrentMinDegrees[servoID],neutralDegrees);
        Debug("targetDegrees is below neutralDegrees. Mapped "+(String)targetDegrees+" to "+(String)newDegrees);
      }else
        Debug("targetDegrees is below neutralDegrees, but... servoCurrentMinDegrees[servoID] == "+(String)servoCurrentMinDegrees[servoID]);
    }
    else{
      if (servoCurrentMaxDegrees[servoID]!=0){
        newDegrees = map(targetDegrees, neutralDegrees, servoEndDegrees[servoID], neutralDegrees, servoCurrentMaxDegrees[servoID]);
        Debug("targetDegrees is above neutralDegrees. Mapped "+(String)targetDegrees+" to "+(String)newDegrees);
      }else
        Debug("targetDegrees is above neutralDegrees, but... servoCurrentMaxDegrees[servoID] == "+(String)servoCurrentMaxDegrees[servoID]);
    }
    targetDegrees = newDegrees;
  }else
    Debug("Neutral Degrees is zero. no need to map anything");

  moveServoToDegrees(servoID, targetDegrees);
}

void moveServoToDegrees(byte servoID, int targetDegrees){
  //Translate the 180 degree range into the equivalent based on the native range. 
  intendedDegrees[servoID]=targetDegrees;

  double nativeRatio = 180.0/(double)servoNativeDegrees[servoID];
  int newTargetDegrees = (int)(((double)targetDegrees)*nativeRatio);

  myServo[servoID].write(newTargetDegrees);              // tell servo to go to position in variable : in steps of 1 degree
  Debug("Writing Servo ID "+(String)servoID+" to degrees "+(String)newTargetDegrees+" using nativeRatio "+(String)nativeRatio); //val: "+(String)val+",
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
    MoveServoToByteValue(i, target);
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



// CurrentMonitor.cpp
#include "CurrentMonitor.h"

CurrentMonitor::CurrentMonitor(int nbServos, int currentSensorPins[], double currentThreshold, long overCurrentTimeout){
  _nbServos=nbServos;
  _currentSensorPins=currentSensorPins;
  _currentThreshold=currentThreshold;
  _overCurrentTimeout=overCurrentTimeout;
  long[nbServos] overCurrentDurations;
  _overCurrentDurations=overCurrentDurations;
}

bool _shouldPrintAmps = true;

unsigned long previousTime=0;

long[] _overCurrentDurations;

void CurrentMonitor::setup() {
  for (int i = 0; i < _nbServos; i++) {
    pinMode(_currentSensorPins[i],INPUT);
  }
}

int measureInterval = 50;
int msSinceMeasure=0;

void CurrentMonitor::loop() {
  long delta = timeDelta();

  if (msSinceMeasure >= measureInterval){
    msSinceMeasure=0;

    double[] amps = calculateAmps();
    printAmps(amps);

  }else
    msSinceMeasure+=delta;

  bool[] nominal = onDemandOverCurrentCheck();
  for(int i=0; i<_nbServos; i++){
    if(nominal[i])
      overCurrentDurations[i]=0;
    else
      overCurrentDurations[i] = overCurrentDurations[i]+timeDelta;
  }
  
}

bool[] CurrentMonitor::isEverythingNominal(){
  bool[_nbServos] nominal;
  
  for(int i=0; i<_nbServos; i++){
    nominal[i] = (overCurrentDurations[i]<_overCurrentTimeout);
  }
  return nominal;
}

bool[] CurrentMonitor::onDemandOverCurrentCheck(){
  double[] amps = calculateAmps();
  return nominalCheck(amps);
}

bool[] nominalCheck(double[] amps){
  bool[_nbServos] nominal;
  
  for(int i=0; i<_nbServos; i++){
    nominal[i] = (amps[i]<_currentThreshold);
  }
  return nominal;
}

void printAmps(double[] amps){
    if (_shouldPrintAmps){
      Serial.print("Min_Amps:0,Max_Amps:6");
      for (int i=0; i<_nbServos; i++){
        Serial.print(",Amps_"+(String)i+":");
        Serial.print(amps[i]);
      }
      Serial.println("");
    }    
}

//  long CurrentMonitor::TimeDelta(){
//   unsigned long currentTime = millis();
//    long delta = (long)(currentTime-previousTime);
//   previousTime = currentTime;
//   return delta;
// }

double[] calculateAmps(){
  unsigned int x=0;

  double[nbServos] Samples;
  double[nbServos] AvgAcs;
  double[nbServos] AcsValueF;

    int numSamples = 10;
    for (int x = 0; x < numSamples; x++){ //Get 150 samples
      for(int i=0; i<_nbServos; i++){
        double AcsValue = analogRead(_currentSensorPins[i]);     //Read current sensor values   
        Samples[i] = Samples[i] + AcsValue;  //Add samples together 
      }
      delay (3); // let ADC settle before next sample 3ms
  }
  for(int i=0; i<_nbServos; i++){
    AvgAcs[i]=Samples[i]/(double)numSamples;//Taking Average of Samples

    //((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
    //2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
    //out to be 2.5 which is out offset. If your arduino is working on different voltage than 
    //you must change the offset according to the input voltage)
    //0.100v(100mV) is rise in output voltage when 1A current flows at input
    AcsValueF[i] = (2.5 - (AvgAcs[i] * (5.0 / 1024.0)) )/0.100;
  }
  return AcsValueF;
}

/*
double calculateAmps(){
  double volts;
  double amps;
  int rawValue=analogRead(pin);
  volts = (rawValue/1024.0)*5000;
  amps = ((volts-ACSOffset)/milliVoltsPerAmp);

  // int current = map(rawValue, 0, 1023, 0, 5000);
  // amps = (double)current/1000.0;

  return abs(amps);
}
*/


long timeDelta(){
  unsigned long currentTime = millis();
   long delta = (long)(currentTime-previousTime);
  previousTime = currentTime;
  return delta;
}

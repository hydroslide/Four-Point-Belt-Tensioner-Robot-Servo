// CurrentMonitor.cpp
#include "CurrentMonitor.h"
#include "Arduino.h"

CurrentMonitor::CurrentMonitor(int nbServos, int* currentSensorPins, double currentThreshold, long overCurrentTimeout) //:
  //_nbServos(nbServos), _currentSensorPins(currentSensorPins), _currentThreshold(currentThreshold), _overCurrentTimeout(overCurrentTimeout)
{
  _nbServos=nbServos;
  _currentSensorPins=currentSensorPins;
  _currentThreshold=currentThreshold;
  _overCurrentTimeout=overCurrentTimeout;
  long* overCurrentDurations = new long[nbServos];
  _overCurrentDurations = overCurrentDurations;
  //_shouldPrintAmps=false;
}

void CurrentMonitor::setup() {
  for (int i = 0; i < _nbServos; i++) {
    pinMode(_currentSensorPins[i], INPUT);
    Serial.println("i: "+(String)i+", pin: "+(String)_currentSensorPins[i]+", Input: "+(String)INPUT);
    //delay(1000);
  }
}

void CurrentMonitor::loop() {
  long delta = monitorTimeDelta();
  
  if (msSinceMeasure >= measureInterval){
    // Serial.println("msSinceMeasure: "+(String)msSinceMeasure+", delta: "+(String)delta);

    long oldMsSinceMeasure = msSinceMeasure;
    this->msSinceMeasure = 0;

    double* amps = calculateAmps();
    printAmps(amps);

    bool* nominal = nominalCheck(amps);
    for(int i = 0; i < _nbServos; i++) {
      if(nominal[i]) {
        _overCurrentDurations[i] = 0;
      } else {
        _overCurrentDurations[i] = _overCurrentDurations[i] +(oldMsSinceMeasure);
      }
    }

    
    delete[] amps;
    delete[] nominal;
  } else {
    this->msSinceMeasure += delta;
  }

}

bool* CurrentMonitor::isEverythingNominal() {
  bool* nominal = new bool[_nbServos];
  
  for(int i = 0; i < _nbServos; i++) {
    nominal[i] = (_overCurrentDurations[i] < _overCurrentTimeout);
  }
  return nominal;
}

bool* CurrentMonitor::onDemandOverCurrentCheck() {
  double* amps = calculateAmps();
  bool* nominal = nominalCheck(amps);
  delete[] amps;
  return nominal;
}

bool* CurrentMonitor::nominalCheck(double* amps) {
  bool* nominal = new bool[_nbServos];
  
  for(int i = 0; i < _nbServos; i++) {
    nominal[i] = (amps[i] < _currentThreshold);
  }
  return nominal;
}

void CurrentMonitor::printAmps(double* amps) {
  if (_shouldPrintAmps) {
    Serial.print("Min_Amps:0,Max_Amps:6");
    for (int i = 0; i < _nbServos; i++) {
      Serial.print(",Amps_" + (String)i + ":");
      Serial.print(amps[i]);
    }
    Serial.println("");
  }  
}

long CurrentMonitor::monitorTimeDelta() {
  unsigned long currentTime = millis();
  long delta = (long)(currentTime - this->_previousTime);
  this->_previousTime = currentTime;
  return delta;
}

double* CurrentMonitor::calculateAmps() {
  unsigned int x = 0;

  double* Samples = new double[_nbServos];
  double* AvgAcs = new double[_nbServos];
  double* AcsValueF = new double[_nbServos];

  int numSamples = 10;
  for (int x = 0; x < numSamples; x++) { //Get 150 samples
    for(int i = 0; i < _nbServos; i++) {
      double AcsValue = analogRead(_currentSensorPins[i]);     //Read current sensor values   
      Samples[i] = Samples[i] + AcsValue;  //Add samples together 
    }
    delay(3);
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

  delete Samples;
  delete AvgAcs;
  return AcsValueF;
}




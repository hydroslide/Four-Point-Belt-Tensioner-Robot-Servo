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
  _nominal = new bool[_nbServos];
  _amps = new double[_nbServos];
  _samples = new double[_nbServos];
  _avgAcs = new double[_nbServos];
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

    calculateAmps();
    printAmps();

    nominalCheck();
    for(int i = 0; i < _nbServos; i++) {
      if(_nominal[i]) {
        _overCurrentDurations[i] = 0;
      } else {
        _overCurrentDurations[i] = _overCurrentDurations[i] +(oldMsSinceMeasure);
      }
    }

    
  } else {
    this->msSinceMeasure += delta;
  }

}

bool* CurrentMonitor::isEverythingNominal() {
    
  for(int i = 0; i < _nbServos; i++) {
    _nominal[i] = (_overCurrentDurations[i] < _overCurrentTimeout);
  }
  return _nominal;
}

bool* CurrentMonitor::onDemandOverCurrentCheck() {
  calculateAmps();
  nominalCheck();  
  return _nominal;
}

void CurrentMonitor::nominalCheck() {
    
  for(int i = 0; i < _nbServos; i++) {
    _nominal[i] = (_amps[i] < _currentThreshold);
  }
  return _nominal;
}

void CurrentMonitor::printAmps() {
  if (_shouldPrintAmps) {
    Serial.print("Min_Amps:0,Max_Amps:6");
    for (int i = 0; i < _nbServos; i++) {
      Serial.print(",Amps_" + (String)i + ":");
      Serial.print(_amps[i]);
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

void CurrentMonitor::calculateAmps() {
  unsigned int x = 0;

  double* Samples = new double[_nbServos];
  double* AvgAcs = new double[_nbServos];
  //double* AcsValueF = new double[_nbServos];

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
    _amps[i] = (2.5 - (AvgAcs[i] * (5.0 / 1024.0)) )/0.100;
  }

  delete Samples;
  delete AvgAcs;
}




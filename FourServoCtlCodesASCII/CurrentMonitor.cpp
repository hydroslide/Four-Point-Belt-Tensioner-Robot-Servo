// CurrentMonitor.cpp
#include "CurrentMonitor.h"
#include "Arduino.h"

CurrentMonitor::CurrentMonitor(int nbServos, int* currentSensorPins, double currentThreshold, long overCurrentTimeout, bool shouldPrintAmps) //:
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
  for (int i=0; i<_nbServos; i++){
     _overCurrentDurations[i] = 0;
    _nominal[i] = true;
    _amps[i] = 0.0;
    _samples[i] =0.0;
    _avgAcs[i] = 0.0;
    _sensorOffsets[i]=0.0;
  }
  _shouldPrintAmps=shouldPrintAmps;
}

void CurrentMonitor::setup() {
  for (int i = 0; i < _nbServos; i++) {
    pinMode(_currentSensorPins[i], INPUT);
    Serial.println("i: "+(String)i+", pin: "+(String)_currentSensorPins[i]+", Input: "+(String)INPUT);   
  }
  delay(1000);
  calibrateSensors();
}

void CurrentMonitor::loop() {
  long delta = monitorTimeDelta();
  
  if (msSinceMeasure >= measureInterval){
    // Serial.println("msSinceMeasure: "+(String)msSinceMeasure+", delta: "+(String)delta);

    long oldMsSinceMeasure = msSinceMeasure;
    this->msSinceMeasure = 0;

    calculateAmps();
    printAmps(false);

    nominalCheck();
    updateDurations(oldMsSinceMeasure);

    
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

void CurrentMonitor::updateDurations(long durationDelta){
    for(int i = 0; i < _nbServos; i++) {
      if(_nominal[i]) {
        _overCurrentDurations[i] =  max(0, _overCurrentDurations[i]-(durationDelta/2));
      } else {
        _overCurrentDurations[i] = _overCurrentDurations[i] +(durationDelta);
      }
    }
}

bool* CurrentMonitor::onDemandOverCurrentCheck(long durationDelta) {
  calculateAmps();
  printAmps(true);
  nominalCheck();  

  updateDurations(durationDelta);
  return _nominal;
}

void CurrentMonitor::nominalCheck() {
    
  for(int i = 0; i < _nbServos; i++) {
    _nominal[i] = (_amps[i] < _currentThreshold);
  }
  return _nominal;
}

void CurrentMonitor::printAmps(bool forcePrint) {
  if (_shouldPrintAmps || forcePrint) {
    Serial.print("Max_Amps:"+(String)_currentThreshold);
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

void CurrentMonitor::calibrateSensors(){
  Serial.println("Calibrating Sensors...");
  calculateAmpsRaw(150);
  String msg="Result";
  for(int i = 0; i < _nbServos; i++) {
    _sensorOffsets[i] = _amps[i];
    msg+=" - "+(String)i+": "+(String)_sensorOffsets[i];
  }
  Serial.println(msg);
}

void CurrentMonitor::calculateAmps() {
  calculateAmpsRaw(10);
  for(int i = 0; i < _nbServos; i++) {
    _amps[i] = _amps[i]-_sensorOffsets[i];
  }
}

void CurrentMonitor::calculateAmpsRaw(int numSamples){
  unsigned int x = 0;
  double maxValue = 1234;
  double* Samples = new double[_nbServos];
  double* AvgAcs = new double[_nbServos];

   for(int i = 0; i < _nbServos; i++) {
    Samples[i]=0.0;
    AvgAcs[i]=0.0;
   }

  for (int x = 0; x < numSamples; x++) { //Get 150 samples
    for(int i = 0; i < _nbServos; i++) {
      double AcsValue = analogRead(_currentSensorPins[i]);     //Read current sensor values   
      if (AcsValue<maxValue){
        Samples[i] = Samples[i] + AcsValue;  //Add samples together 
        //Serial.println(("AcsValue[i] "+(String)i+" - "+(String)AcsValue));
      }
    }
    delay(3);
  }

  for(int i=0; i<_nbServos; i++){   
    
    AvgAcs[i]= abs(Samples[i])/(double)numSamples;

    //((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
    //2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
    //out to be 2.5 which is out offset. If your arduino is working on different voltage than 
    //you must change the offset according to the input voltage)
    //0.100v(100mV) is rise in output voltage when 1A current flows at input
  double offset = 2.5;
  
    _amps[i] = (offset - (AvgAcs[i] * (5.0 / 1024.0)) )/0.100;

    Samples[i]=0;
    AvgAcs[i]=0;
  }

  delete Samples;
  delete AvgAcs;
}




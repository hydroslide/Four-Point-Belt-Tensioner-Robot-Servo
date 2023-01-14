// CurrentMonitor.cpp
#include "CurrentMonitor.h"
#include "Arduino.h"

CurrentMonitor::CurrentMonitor(int nbServos, int* currentSensorPins, double currentThreshold, double maxCurrent, long overCurrentTimeout, long maxTimeout, bool shouldPrintAmps) //:
  //_nbServos(nbServos), _currentSensorPins(currentSensorPins), _currentThreshold(currentThreshold), _overCurrentTimeout(overCurrentTimeout)
{
  _sensorsFunctioningProperly = true;
  _nbServos=nbServos;
  _currentSensorPins=currentSensorPins;
  _currentThreshold=currentThreshold;
  _maxCurrent = maxCurrent;
  _overCurrentTimeout=overCurrentTimeout;
  _maxTimeout = maxTimeout;
  long* overCurrentDurations = new long[nbServos];
  _overCurrentDurations = overCurrentDurations;
  _nominal = new bool[_nbServos];
  _amps = new double[_nbServos];
  _samples = new double[_nbServos];
  _avgAcs = new double[_nbServos];
  _sensorOffsets = new double[_nbServos];
  _fakeCurrentOffset = new double[_nbServos];
  _fakeCurrentBuffer = new double[_nbServos];
  _fakeCurrentMsSinceDecrementRequest= new long[nbServos];
  for (int i=0; i<_nbServos; i++){
     _overCurrentDurations[i] = 0;
    _nominal[i] = true;
    _amps[i] = 0.0;
    _samples[i] =0.0;
    _avgAcs[i] = 0.0;
    _sensorOffsets[i]=0.0;
    _fakeCurrentOffset[i]=0.0;
    _fakeCurrentBuffer[i]=0.0;
    _fakeCurrentMsSinceDecrementRequest[i]=0;
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


  fakeCurrentAction(delta);
}

bool CurrentMonitor::isFunctioningProperly(){
  return _sensorsFunctioningProperly;
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
      if ( _overCurrentDurations[i] > _maxTimeout){
        _sensorsFunctioningProperly=false;
        Serial.println("Servo "+(String)i+" has been overCurrent for "+(String)_overCurrentDurations[i]+"ms, over the limit of "+(String)_maxTimeout+"ms. NO BUENO.");
        return;
      }    
    }
}

bool* CurrentMonitor::onDemandOverCurrentCheck(long durationDelta) {
  fakeCurrentAction(durationDelta);
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

void CurrentMonitor::setCurrentThreshold(double currentThreshold){
  _currentThreshold=currentThreshold;
}

double CurrentMonitor::getCurrentThreshold(){
  return _currentThreshold;
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
    _amps[i] = (_amps[i]-_sensorOffsets[i])+_fakeCurrentOffset[i];
    if ((String)_amps[i] == "ovf"){
      Serial.println("Amp "+(String)i+" is reporting ovf. Not good.");
      _sensorsFunctioningProperly=false;
    }
    if (_amps[i]>=_maxCurrent){
       Serial.println("Amp "+(String)i+" is over the max current of "+(String)_maxCurrent+" reporting "+(String)_amps[i]+". VERY BAD.");
      _sensorsFunctioningProperly=false;
    }
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

void CurrentMonitor::incrementFakeCurrent(int servoId, double stepSize){
  _fakeCurrentBuffer[servoId] = _fakeCurrentBuffer[servoId]+stepSize;
  _fakeCurrentMsSinceDecrementRequest[servoId]=0;
}

void CurrentMonitor::fakeCurrentAction(long delta){
  if (_fakeCurrentInterval == 0)
    return;
  _fakeCurrentMsSinceChange+=delta;
  if (_fakeCurrentMsSinceChange >=_fakeCurrentInterval){
    double stepSize = _fakeCurrentStepSize * (((double)_fakeCurrentMsSinceChange)/_fakeCurrentInterval);
    for(int i=0; i<_nbServos; i++){
      int servoId=i;
      // Check for decrement requests
      if (_fakeCurrentMsSinceDecrementRequest[servoId]!=0){
        _fakeCurrentMsSinceDecrementRequest[servoId] = _fakeCurrentMsSinceDecrementRequest[servoId]+_fakeCurrentMsSinceChange;
        if (_fakeCurrentMsSinceDecrementRequest[servoId] >= _fakeCurrentDecrementTimeout){
          _fakeCurrentBuffer[servoId] = _fakeCurrentOffset[servoId] *-1;
          _fakeCurrentMsSinceDecrementRequest[servoId] = 0;
        }
      }


      double buffer = _fakeCurrentBuffer[i];
      if (buffer!=0){
        double actualStep = min(((stepSize*buffer)/abs(buffer)), buffer);
        _fakeCurrentOffset[i] = _fakeCurrentOffset[i] + actualStep;
        _fakeCurrentBuffer[i] = _fakeCurrentBuffer[i] - actualStep;

        // Kick off decrement request
        if (_fakeCurrentBuffer[i] == 0 && _fakeCurrentOffset[i]>0)
          _fakeCurrentMsSinceDecrementRequest[servoId]=1;
      }
    }
     _fakeCurrentMsSinceChange=0;
  }
}

void CurrentMonitor::incrementFakeCurrentDecrementTimeout(long stepSize){
  _fakeCurrentDecrementTimeout+=stepSize;
  for(int i=0; i<3; i++){
    Serial.println("FakeCurrentTimeout:"+(String)((double)_fakeCurrentDecrementTimeout/1000));
    delay(100);
  }
}


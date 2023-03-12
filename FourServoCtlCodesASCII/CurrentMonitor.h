// CurrentMonitor.h
#ifndef CURRENT_MONITOR_H
#define CURRENT_MONITOR_H

class CurrentMonitor {
public:
  CurrentMonitor(int nbServos, int* currentSensorPins, double currentThreshold, double maxCurrent, long overCurrentTimeout, long maxTimeout, bool shouldPrintAmps);
  void setup();  
  void loop();
  bool* isEverythingNominal();
  bool* onDemandOverCurrentCheck(long durationDelta);
  void setCurrentThreshold(double currentThreshold);
  double getCurrentThreshold();
  bool isFunctioningProperly();
  void incrementFakeCurrent(int servoId, double stepSize);  
  void incrementFakeCurrentDecrementTimeout(long stepSize);
  void resetServoFunctionality();
  

private:
  int _nbServos;  
  double _currentThreshold;
  double _maxCurrent;
  long _overCurrentTimeout;
  long _maxTimeout;
  bool _sensorsFunctioningProperly;
 
  long monitorTimeDelta();
  void calculateAmpsRaw(int numSamples);
  void calculateAmps();
  void printAmps(bool forcePrint);
  void nominalCheck();
  void updateDurations(long durationDelta);

  void calibrateSensors();
  void fakeCurrentAction(long delta);


  int* _currentSensorPins;
  long* _overCurrentDurations;
  bool _shouldPrintAmps = false;
  unsigned long _previousTime = 0;
  int measureInterval = 50;
  int msSinceMeasure = 0;
  double* _amps;
  bool* _nominal;
  double* _samples;
  double* _avgAcs;
  double* _sensorOffsets;

  double* _fakeCurrentOffset;
  double* _fakeCurrentBuffer;
  long* _fakeCurrentMsSinceDecrementRequest;
  double _fakeCurrentStepSize=.1;
  long _fakeCurrentInterval=15;
  long _fakeCurrentMsSinceChange=0;
  long _fakeCurrentDecrementTimeout = 500;
};

#endif // CURRENT_MONITOR_H
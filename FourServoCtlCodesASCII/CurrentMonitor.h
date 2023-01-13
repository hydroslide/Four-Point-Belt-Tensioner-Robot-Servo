// CurrentMonitor.h
#ifndef CURRENT_MONITOR_H
#define CURRENT_MONITOR_H

class CurrentMonitor {
public:
  CurrentMonitor(int nbServos, int* currentSensorPins, double currentThreshold, long overCurrentTimeout, bool shouldPrintAmps);
  void setup();  
  void loop();
  bool* isEverythingNominal();
  bool* onDemandOverCurrentCheck(long durationDelta);

private:
  int _nbServos;  
  double _currentThreshold;
  long _overCurrentTimeout;
 
  long monitorTimeDelta();
  void calculateAmpsRaw(int numSamples);
  void calculateAmps();
  void printAmps(bool forcePrint);
  void nominalCheck();
  void updateDurations(long durationDelta);

  void calibrateSensors();

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
};

#endif // CURRENT_MONITOR_H
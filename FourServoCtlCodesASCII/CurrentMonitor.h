// CurrentMonitor.h
#ifndef CURRENT_MONITOR_H
#define CURRENT_MONITOR_H

class CurrentMonitor {
public:
  CurrentMonitor(int nbServos, int* currentSensorPins, double currentThreshold, long overCurrentTimeout);
  void setup();  
  void loop();
  bool* isEverythingNominal();
  bool* onDemandOverCurrentCheck();

private:
  int _nbServos;  
  double _currentThreshold;
  long _overCurrentTimeout;
 
  long monitorTimeDelta();
  void calculateAmps();
  void printAmps();
  void nominalCheck();
  void updateDurations(long durationDelta);

  int* _currentSensorPins;
  long* _overCurrentDurations;
  bool _shouldPrintAmps = true;
  unsigned long _previousTime = 0;
  int measureInterval = 50;
  int msSinceMeasure = 0;
  double* _amps;
  bool* _nominal;
  double* _samples;
  double* _avgAcs;
};

#endif // CURRENT_MONITOR_H
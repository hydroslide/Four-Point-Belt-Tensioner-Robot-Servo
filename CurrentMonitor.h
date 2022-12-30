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
  double* calculateAmps();
  void printAmps(double* amps);
  bool* nominalCheck(double* amps);

  int* _currentSensorPins;
  long* _overCurrentDurations;
  bool _shouldPrintAmps = true;
  unsigned long _previousTime = 0;
  int measureInterval = 50;
  int msSinceMeasure = 0;
};

#endif // CURRENT_MONITOR_H
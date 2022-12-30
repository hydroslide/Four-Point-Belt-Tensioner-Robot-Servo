// CurrentMonitor.h
#ifndef CURRENT_MONITOR_H
#define CURRENT_MONITOR_H


class CurrentMonitor {
public:
  CurrentMonitor(int nbServos, int currentSensorPins[], double currentThreshold, long overCurrentTimeout);
  void setup();  
  void loop();
  void isEverythingNominal(bool * nominal);
  void onDemandOverCurrentCheck(bool * nominal);

private:
  int _nbServos;  
  double _currentThreshold;
  long _overCurrentTimeout;
 
  long timeDelta();
  calculateAmps(double * amps);

  int _currentSensorPins[];
};

#endif // CURRENT_MONITOR_H

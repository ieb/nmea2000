
#ifndef __PULSESENSOR_H__
#define __PULSESENSOR_H__

#ifndef TEST
#include <N2kMessages.h>
#endif


/**
 * Monitors pulses using an ISR to generate speed and distance measurements.
 */
class PulseSensor {
public:
  PulseSensor(double defaultPulsesPerM, void (*readFunction)(unsigned long *), void (*interruptHandler)(void), int pin, double* demoSpeed){
    this->defaultPulsesPerMeter = defaultPulsesPerM;
    this->pulseIIR = 5;
    this->readFunction = readFunction;
    this->pin = pin;
    this->averagePulsePeriod = 0.0F;
    this->pulseVariance = 0.0F;
    this->resetCalibration();
    this->demoSpeed = demoSpeed;
    this->demoMode = false;
    attachInterrupt(digitalPinToInterrupt(this->pin),interruptHandler,RISING);
  }

  ~PulseSensor() {
    detachInterrupt(digitalPinToInterrupt(this->pin));
  }

  bool init() {
    return true;
  }

  void resetCalibration() {
      for ( int i = 0; i < 10; i++) {
        frequencyCalibration[i] = 0.0F;
        pulsesPerMCalibration[i] = defaultPulsesPerMeter;
      }
      ncalibrationPoints = 1;
  }

  void setDamping(int pulseIIR) {
    this->pulseIIR = pulseIIR;
  }


  void calibrate(float* frequencyCalibrationPoint, float* pulsesPerMCalibrationPoint, int ncal) {
    ncalibrationPoints = ncal>10?10:ncal;
    for ( int i = 0; i < ncalibrationPoints; i++) {
      frequencyCalibration[i] = frequencyCalibrationPoint[i];
      pulsesPerMCalibration[i] = pulsesPerMCalibrationPoint[i];   
      INFO("Calibration: F,PPKn, ");    
      INFOC(frequencyCalibration[i]);    
      INFOC(",");    
      INFOLN(hzPerMPerSToHzPerKn(pulsesPerMCalibration[i]));    
    }
  }

  void setDemoMode(bool demoMode) {
    this->demoMode = demoMode;
    lastDemoRead = 0;
  }

  void read() {
    unsigned long data[2];
    if (demoMode) {
      speed = demoSpeed[0];
      unsigned long now = millis();
      if ( lastDemoRead != 0) {
        distance = distance + speed*((double)(now-lastDemoRead))/1000.0F;
      }
      lastDemoRead = now;
      speedVariance = 0;
      frequency = 0;
      pulsesPerMeter = findCalibrationPoint();
      frequency = speed * pulsesPerMeter;
      pulsesPerMeter = findCalibrationPoint();
      frequency = speed * pulsesPerMeter;
      frequencyVariance = 0;
      if ( frequency > 1E-8) {
        averagePulsePeriod = 1000000.0/frequency;
      } else {
        averagePulsePeriod = 100000000.0;
      }
      pulseVariance = 0;
      return;
    }
    (*(this->readFunction))(&data[0]);
    if ( data[0] == edges ) {
        speed = 0.0F;
        frequency = 0.0F;
        pulsesPerMeter = findCalibrationPoint();
    }  else {
      unsigned long pulse_diff = 0;
      if ( data[0] < edges) { 
        // is a wrap!!!, been up a long long time.
        pulse_diff = (0xFFFFFFFF-edges)+data[1];
      } else {
       pulse_diff = data[0] - edges;
      }
      edges = data[0];
      double periodPeriodDiff = (double)data[1]-averagePulsePeriod;
      averagePulsePeriod = averagePulsePeriod + (periodPeriodDiff)/pulseIIR;
      pulseVariance = pulseVariance + ((periodPeriodDiff*periodPeriodDiff)-pulseVariance)/pulseIIR;
      frequency = 1000000.0/averagePulsePeriod;
      frequencyVariance = 1000000.0/pulseVariance;
      pulsesPerMeter = findCalibrationPoint();
      speed = frequency/pulsesPerMeter;
      speedVariance = frequencyVariance/pulsesPerMeter;
      distance = distance + (pulse_diff/pulsesPerMeter);
    }
  }

  void dumpstate(char * id) {
    INFO(id);
    INFOC(",demo,");
    INFOC(demoMode);
    INFOC(",pulsePeriod,");
    INFOC(averagePulsePeriod);
    INFOC(",frequency,");
    INFOC(frequency);
    INFOC(",ppm,");
    INFOC(pulsesPerMeter);
    INFOC(",ppk,");
    INFOC(hzPerMPerSToHzPerKn(pulsesPerMeter));
    INFOC(",speed,");
    INFOC(msToKnots(speed));
    INFOC(",distanceM,");
    INFO(distance);
    INFOC(",distanceK,");
    INFON(metersToNMiles(distance));
  }
  double getTripDistance() {
    return distance;
  }
  double getSpeed() {
    return speed;
  }
  double getSpeedStdev() {
    return sqrt(speedVariance);
  }
private:
    double speed; // in m/s
    double frequency; // in Hz
    double distance; // in m, this is the trip distance.
    double pulsesPerMeter; // pulses/m
    double defaultPulsesPerMeter; // default pulses PerM.
    double averagePulsePeriod;
    double pulseVariance;
    double frequencyVariance;
    double speedVariance;
    unsigned long edges; // number of edges at last update.
    double frequencyCalibration[10];
    double pulsesPerMCalibration[10];
    int ncalibrationPoints;
    int pulseIIR;
    int pin;
    double *demoSpeed;
    unsigned long lastDemoRead;
    bool demoMode;
    void (*readFunction)(unsigned long *);

    double findCalibrationPoint() {
      for ( int i = 0; i < ncalibrationPoints; i++) {
        if (frequency < frequencyCalibration[i]) {
          return pulsesPerMCalibration[i];
        }
      }
      return pulsesPerMCalibration[ncalibrationPoints-1];
    }
};




#endif
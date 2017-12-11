
#ifndef __ANGLESENSOR_H__
#define __ANGLESENSOR_H__

#ifndef TEST
#include <N2kMessages.h>
#endif

#include "conversions.h"
#include "demo.h"
#include "configuration.h"








/**
 * Reads teh sin and cos voltages from a sensor to produce an angular reading with stdev
 */
class AngleSensor {
public:
  AngleSensor(int sinPin, int cosPin, demo_data_t *demoData) {
    analogReadResolution(12);
    this->pin[0] = sinPin;
    this->pin[1] = cosPin;
    this->minv[0] = this->minv[1] = 0xFFFFFF; // 12bit max
    this->maxv[0] = this->maxv[1] = 0;
    setAutoConfig(false);
    this->nsamples = 5;
    this->offset = 0.0F;
    this->autoConfig = false;
    this->demoData = demoData;
    this->demoMode = false;
  }

  bool init() {
    return true;
  }  

  void calibrate(int *minv, int *maxv, double offset){
    this->offset = offset;
    for (int i = 0; i < 2; i++) {
      this->minv[i] = minv[i];
      this->maxv[i] = maxv[i];
      range[i] = ((double)maxv[i]-(double)minv[i])/2.0F;
      mean[i] = ((double)maxv[i]+(double)minv[i])/2.0F;
    }
  }

  void setDamping(int nsamples) {
    this->nsamples = nsamples;
  }

  void setAutoConfig(bool autoConfig) {
    if (!autoConfig) {
      for (int i = 0; i < 2; i++) {      
        range[i] = ((double)maxv[i]-(double)minv[i])/2.0F;
        mean[i] = ((double)maxv[i]+(double)minv[i])/2.0F;
      }      
    }
    this->autoConfig = autoConfig;
  }

  void setDemoMode(bool demoMode) {
    this->demoMode = demoMode;
  }


  void read() {
    if (demoMode) {
      // 2 voltages representing sin and cos at the angle specified
      // the adc is 12 bit == 3.3v
      // voltage in is 2-6 measured as 1-3
      // bits per v is 
      // 
      double bitsPerV = 4096.0F/3.3F;
      LOG(F("Wind Angle,"));
      LOGC(demoData->windAngle);
      LOGC(F(",BPV,"));
      LOGC(bitsPerV);
      LOGC(F(",nsamples,"));
      LOGC(nsamples);
      LOGC(F(",sin,"));
      LOGC(sin(demoData->windAngle));
      LOGC(F(",cos,"));
      LOGN(cos(demoData->windAngle));
      vmean[0] = vmean[0] + ((bitsPerV*(sin(demoData->windAngle)*1.0+2.0)) - vmean[0])/this->nsamples;
      vmean[1] = vmean[1] + ((bitsPerV*(cos(demoData->windAngle)*1.0+2.0)) - vmean[1])/this->nsamples;
      maxv[0] = maxv[1] = (int)(bitsPerV*3.0F);
      minv[0] = minv[1] = (int)(bitsPerV*1.0F);
      for (int i = 0; i < 2; i++) {      
        maxv[i] = (int)(bitsPerV*3.0F);
        minv[0] = (int)(bitsPerV*1.0F);
        range[i] = ((double)maxv[i]-(double)minv[i])/2.0F;
        mean[i] = ((double)maxv[i]+(double)minv[i])/2.0F;
      }      
    } else {
      for (int i = 0; i < 2; i++) {
        double arv = analogRead(pin[i]);
        rawv[i] = arv;
        if (autoConfig) {
          maxv[i] = arv>maxv[i]?arv:maxv[i];
          minv[i] = arv<minv[i]?arv:minv[i];
        } else {
          arv = rawv[i]>maxv[i]?maxv[i]:arv;
          arv = arv<minv[i]?minv[i]:arv;
        }
        vmean[i] = vmean[i] + (arv - vmean[i])/this->nsamples;
      }      
    }
  }

  void dumpstate(char *id) {
    double angles[2];
    for (int i = 0; i < 2; i++ ) {
      if (range[i] < 1.0E-8) {
        angles[i] = 0.0F;
      } else {
        angles[i] = (vmean[i]-mean[i])/range[i];
      }
      INFO(id);
      INFOC(F(",demo,"));
      INFOC(demoMode);
      INFOC(F(",ch,"));
      INFOC(i);
      INFOC(F(",pin,"));
      INFOC(pin[i]);
      INFOC(F(",mean,"));
      INFOC(mean[i]);
      INFOC(F(",range,"));
      INFOC(range[i]);
      INFOC(F(",rawv,"));
      INFOC(rawv[i]);
      INFOC(F(",vmean,"));
      INFOC(vmean[i]);
      INFOC(F(",angle,"));
      INFON(angles[i]);
    }
    double wa = atan2(angles[0], angles[1]);
    INFO(id);
    INFOC(F(",angleRad,"));
    INFOC(wa);
    INFOC(F(",angleDeg,"));
    INFON(RadToDeg(wa));
  }
  double getAngle() {
    return this->offset+calc(0);
  }
  double getAngleStdev() {
    return calc(1);
  }

private:
  int pin[2], maxv[2], minv[2], vmean[2], rawv[2], nsamples;
  double range[2], mean[2], offset;
  bool autoConfig;
  demo_data_t *demoData;
  bool demoMode;

 
  /**
   * Calculate the current angle or stdev in radians.
   */ 
  double calc(int stdev) {
    double angles[2], avars[2];
    if (autoConfig) {
      for (int i = 0; i < 2; i++) {      
        range[i] = ((double)maxv[i]-(double)minv[i])/2.0F;
        mean[i] = ((double)maxv[i]+(double)minv[i])/2.0F;
      }      
      LOGLN(F("autoConfig running"));
    }
    for (int i = 0; i < 2; i++) {
      if (range[i] < 1.0E-8) {
        angles[i] = 0.0F;
      } else {
        angles[i] = (vmean[i]-mean[i])/range[i];
      }
      LOG(F("mean,"));
      LOGC(mean[i]);
      LOGC(F(",range,"));
      LOGC(range[i]);
      LOGC(F(",vmean,"));
      LOGC(vmean[i]);
      LOGC(F(",angles,"));
      LOGN(angles[i]);
    }
    if (stdev) {
      // stdev of an angle is sqrt(-2*log(r)) were r = mean of all samples.
      // same method as uses by SciPy.
      double r2 = angles[0]*angles[0]+angles[1]*angles[1];
      if ( r2 < 1E-8 ) {
        return 2*PI;
      } else if ( r2 > 1.0F) {
        return 0;
      } else {
        return sqrt(-2*log(r2));
      }
    } else {
      return atan2(angles[0], angles[1]);
    } 
  }
};




#endif
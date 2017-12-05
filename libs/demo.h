#ifndef __DEMO_H__
#define __DEMO_H__

#ifndef TEST
#include <Arduino.h>
#endif


struct demovector_t {
  double x,y,z;
};


struct demo_data_t {
  double windAngle; // in radians
  double windSpeed; // in m/s
  double waterSpeed; // in m/s
  demovector_t orientation; // in radians
  demovector_t gyro; // in radians
  double waterTemperature; // in C.
};




#endif

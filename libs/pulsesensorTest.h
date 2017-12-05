
#ifndef __PULSESENSORTEST_H__
#define __PULSESENSORTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif

#include "testmocks.h"
#include <math.h>
#include "pulsesensor.h"


#ifdef TEST// unit tests can be run on a desktop.


// eventHandlerToRecord the time between rising edges.
// Looking at the cuircuit diagram of the WindVane it looks like it is a pull down 
// cuircuit via an opti isplator. It is clamped with 2 diodes to 0-8V - the diode votage
// drop.
volatile unsigned long wind_period_test = 0;
volatile unsigned long wind_edges_test = 0;
void windPulseHandler_test() {
  static unsigned long period_millis = 0;
  static unsigned long last_period_millis = 0;
  last_period_millis = period_millis;
  period_millis = millis();
  wind_period_test = period_millis - last_period_millis;
  wind_edges_test++;
}

void readWindMonitor_test(unsigned long *data) {
  data[0] = wind_edges_test;
  data[1] = wind_period_test;
}

inline double knotsTomsPuseSensor(double v) { return v*1852.0/3600.0; }



bool testPulseSensor() {
    double demoSpeed;
    PulseSensor pulseSensor = PulseSensor(knotsTomsPuseSensor(1.045F), &readWindMonitor_test, &windPulseHandler_test, 12, &demoSpeed);
    pulseSensor.resetCalibration();

    pulseSensor.setDamping(5);
    float cal[] = {1.0, 10.0, 15.0, 20.0, 25.0, 100.0 };
    float ppm[] = {1.045, 1.04, 1.03, 1.0, 0.99, 0.85 };
    pulseSensor.calibrate(&cal[0], &ppm[0], 6);

    for ( int i = 0; i < 100; i++ ) {
        windPulseHandler_test();
        // cycle between 20Hz and 10Hz
        delay(50+cos((i%10)/10)*50);
        if ( i % 7 == 0 ) {
            pulseSensor.read();
            pulseSensor.getTripDistance();
            pulseSensor.getSpeed();
            pulseSensor.getSpeedStdev();
        }
    }
    return true;
}

#endif
#endif

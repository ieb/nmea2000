
#ifndef __ANGLESENSORTEST_H__
#define __ANGLESENSORTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif

#include "testmocks.h"
#include <math.h>
#include "anglesensor.h"


#ifdef TEST// unit tests can be run on a desktop.



bool testAngleSensor() {
    demo_data_t demoData;
    AngleSensor angleSensor = AngleSensor(5,6,&demoData);
    int minv[] = { 0xFFFFFF, 0xFFFFFF };
    int maxv[] = { 0,0  };
    angleSensor.calibrate(&minv[0], &maxv[0], 0.123F);
    angleSensor.setDamping(3);

    angleSensor.setAutoConfig(true);
    angleSensor.read();
    angleSensor.read();
    angleSensor.read();
    angleSensor.read();
    angleSensor.read();
    angleSensor.setAutoConfig(false);
    angleSensor.read();
    angleSensor.read();
    
    angleSensor.getAngle();
    angleSensor.getAngleStdev();

    return true;
}

#endif
#endif

#ifndef __MULTISENSORTEST_H__
#define __MULTISENSORTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif


#include "testmocks.h"
#include <math.h>
#include "multiSensor.h"


#ifdef TEST

// unit tests can be run on a desktop, otherwise look in the h class where all the code is to enable inlining.

bool testMultiSensorMonitor() {
    Statistics statistics = Statistics();
    MotionSensor motionSensor = MotionSensor();
    MultiSensor multiSensor = MultiSensor(&statistics, 
        &motionSensor,
        true, 
        1,2,3,4,17.5F, 12);
    multiSensor.setMode(MONITOR_MODE_DEMO);
    multiSensor.read();
    tN2kMsg message;
    multiSensor.fullStatusMessage(message);
    return true;
}



#endif
#endif



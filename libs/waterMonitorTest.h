#ifndef __WATERMONITORTEST_H__
#define __WATERMONITORTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif

#include "testmocks.h"
#include <math.h>
#include "demo.h"
#include "waterMonitor.h"


#ifdef TEST

// unit tests can be run on a desktop.

bool testWaterMonitor() {
    demo_data_t demoData;
    WaterMonitor waterMonitor = WaterMonitor(12, &demoData);
    waterMonitor.read();
    tN2kMsg DummyMessage;
    waterMonitor.fillWaterTemperature(DummyMessage);
    return true;
}



#endif
#endif



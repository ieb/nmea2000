
#ifndef __BATTERYMONITORTEST_H__
#define __BATTERYMONITORTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif

#include "testmocks.h"
#include <math.h>
#include "batteryMonitor.h"


#ifdef TEST// unit tests can be run on a desktop.


bool testBatteryMonitor() {
    BatteryMonitor batteryMonitor = BatteryMonitor(1, 200);
    batteryMonitor.begin();
    batteryMonitor.read();
    tN2kMsg DummyMessage;
    batteryMonitor.fillStatusMessage(DummyMessage);
    batteryMonitor.fillChargeStatusMessage(DummyMessage);
    batteryMonitor.fillBatteryConfigurationMessage(DummyMessage);
    return true;
}

#endif
#endif

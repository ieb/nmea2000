
#ifndef __ENVIROMONITORTEST_H__
#define __ENVIROMONITORTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif

#include "testmocks.h"
#include <math.h>
#include "enviroMonitor.h"


#ifdef TEST// unit tests can be run on a desktop.

bool testEnviroMonitor() {
    EnviroMonitor enviroMonitor = EnviroMonitor();
    enviroMonitor.begin();
    enviroMonitor.read();
    tN2kMsg DummyMessage;
    enviroMonitor.fillStatusMessage(DummyMessage);
    enviroMonitor.read();
    enviroMonitor.fillStatusMessage(DummyMessage);
    return true;
}

#endif
#endif

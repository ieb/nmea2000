
#ifndef __BOATMONITORTEST_H__
#define __BOATMONITORTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif

#include "testmocks.h"
#include <math.h>
#include "boatMonitor.h"


#ifdef TEST// unit tests can be run on a desktop.


#include "pogo1250.h"

bool testBoatMonitor() {
    Polar_Performance polarPerf = Polar_Performance((char *)POGO1250_NAME, POGO1250_N_TWA, POGO1250_N_TWS, pogo1250Data_twa, pogo1250Data_tws, pogo1250Data_bsp);
    Statistics statistics = Statistics();
    BoatMonitor boatMonitor = BoatMonitor(&polarPerf, &statistics);
    boatMonitor.read(millis());

    tN2kMsg DummyMessage;
    boatMonitor.fillPolarPerformance(DummyMessage);
    boatMonitor.fillTargetBoatSpeed(DummyMessage);
    boatMonitor.fillBoatSpeed(DummyMessage);
    boatMonitor.fillAparentWind(DummyMessage);
    boatMonitor.fillTrueWind(DummyMessage);
    return true;
}

#endif
#endif

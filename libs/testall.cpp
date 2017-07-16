#ifndef ARDUINO
#define TEST 1
#endif


#include "testmocks.h"
#include <math.h>


#ifdef TEST

#include "eventsTest.h"
#include "enviroMonitorTest.h"
#include "multiSensorTest.h"
#include "polarTest.h"
#include "statisticTest.h"
#include "waterMonitorTest.h"
#include "boatMonitorTest.h"
#include "batteryMonitorTest.h"




bool batteryMonitorTest() {
    if (testBatteryMonitor()) {
        return true;
    }
    return false;
}

bool boatMonitorTest() {
    if (testBoatMonitor()) {
        return true;
    }
    return false;
}



bool waterMonitorTest() {
    if (testWaterMonitor()) {
        return true;
    }
    return false;
}

bool statisticTest() {
    if (testLinearStatistics() &&
        testDegreeStatistic() &&
        testRadianStatistic() ) {
        return true;
    }
    return false;
}


bool testPolar() {
    if ( testPolarPerformance() &&
         testTrueWind() &&
           testACos() &&
           testCos()
         ) {
        return true;
    }
    return false;
}

bool testEvents() {
    if ( testTimedEventQueue() &&
         testEventEmitter() 
         ) {
        return true;
    }
    return false;
}


int main() {
    int failed = 0;
    std::cout << "Testing Statistics Module" << std::endl;
    if (! statisticTest() ) {
        std::cout << "Testing Statistics Module Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing Statistics Module Passed" << std::endl;

    }
    std::cout << "Testing testEvents" << std::endl;
    if (! testEvents() ) {
        std::cout << "Testing testEvents Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing testEvents Passed" << std::endl;
    }
    std::cout << "Testing testEnviroMonitor" << std::endl;
    if (! testEnviroMonitor() ) {
        std::cout << "Testing testEnviroMonitor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing testEnviroMonitor Passed" << std::endl;
    }
    std::cout << "Testing testMultiSensorMonitor" << std::endl;
    if (! testMultiSensorMonitor() ) {
        std::cout << "Testing testMultiSensorMonitor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing testMultiSensorMonitor Passed" << std::endl;
    }
    std::cout << "Testing testing Polar" << std::endl;
    if (! testPolar() ) {
        std::cout << "Testing testMultiSensorMonitor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing testMultiSensorMonitor Passed" << std::endl;
    }
    std::cout << "Testing testing WaterMonitor" << std::endl;
    if (! testWaterMonitor() ) {
        std::cout << "Testing WaterMonitor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing WaterMonitor Passed" << std::endl;
    }
    std::cout << "Testing testing BoatMonitor" << std::endl;
    if (! testBoatMonitor() ) {
        std::cout << "Testing BoatMonitor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing BoatMonitor Passed" << std::endl;
    }
    std::cout << "Testing testing BatteryMonitor" << std::endl;
    if (! testBatteryMonitor() ) {
        std::cout << "Testing BatteryMonitor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing BatteryMonitor Passed" << std::endl;
    }
    return 0;
}
#endif




#ifndef ARDUINO
#define TEST 1
#endif


#include "testmocks.h"
#include <math.h>


#ifdef TEST

#include "eventsTest.h"
#include "anglesensorTest.h"
#include "pulsesensorTest.h"
#include "multiSensorTest.h"
#include "polarTest.h"
#include "statisticTest.h"
#include "waterMonitorTest.h"
#include "boatMonitorTest.h"



bool pulseSensorTest() {
    if (testPulseSensor() ) {
        return true;
    }
    return false;
}

bool angleSensorTest() {
    if ( testAngleSensor() ) {
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
    std::cout << "Testing testPulseSensor" << std::endl;
    if (! testPulseSensor() ) {
        std::cout << "Testing testPulseSensor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing testPulseSensor Passed" << std::endl;
    }
    std::cout << "Testing testAngleSensor" << std::endl;
    if (! testAngleSensor() ) {
        std::cout << "Testing testAngleSensor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing testAngleSensor Passed" << std::endl;
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
}
#endif




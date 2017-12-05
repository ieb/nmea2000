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


int main() {
    int failed = 0;
    
    std::cout << "Testing testing Polar" << std::endl;
    if (! testPolar() ) {
        std::cout << "Testing testMultiSensorMonitor Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing testMultiSensorMonitor Passed" << std::endl;
    }
}
#endif




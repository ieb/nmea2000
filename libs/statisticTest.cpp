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


bool statisticTest() {
    if (testLinearStatisticsFill() &&
        testLinearStatistics() &&
        testDegreeStatistic() &&
        testRadianStatistic() ) { 
        return true;
    }
    return false;
}


int main() {
    int failed = 0;
    
    std::cout << "Testing statisticTest" << std::endl;
    if (! statisticTest() ) {
        std::cout << "Testing statisticTest Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
        failed++;
        return failed;
    } else {
        std::cout << "Testing statisticTest Passed" << std::endl;
    }
}
#endif




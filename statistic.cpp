#ifndef ARDUINO
#define TEST 1
#endif

#ifdef TEST
#include <iostream>
#define TRACE(x)
#define DEBUG(x) x
#define ERROR(x) x
#else
#define TRACE(x)
#define DEBUG(x)
#define ERROR(x)
#endif


#include <math.h>
#include "statistic.h"


#ifdef TEST

// Unit tests ============================================================================

void Statistic::outputSeconds(void) {
    std::cout << "Seconds ";
    for ( int i = 0; i < 60; i++) {
        std::cout << toOutput(seconds[i]) << " ";
    }
    std::cout << std::endl;
}

void Statistic::outputMinutes(void) {
    std::cout << "Minutes ";
    for ( int i = 0; i < 60; i++) {
        std::cout << toOutput(minutes[i]) << " ";
    }
    std::cout << std::endl;
}


float Statistic::getSecond(int n) {
    if ( n < 0 || n >= 60 ) {
         std::cout << " Invalid Index for getSecond " << n << std::endl;
         return -1;
    }
    return toOutput(seconds[n]);
}
float Statistic::getMinute(int n) {
    if ( n < 0 || n >= 60 ) {
         std::cout << " Invalid Index for getMinute " << n << std::endl;
         return -1;
    }
    return toOutput(minutes[n]);

}

bool testLinearStatistics(void) {
    std::cout << "new Statistic" << std::endl;
    Statistic linear = Statistic();
    std::cout << "Start Loading " << std::endl;    
    unsigned long tnow = 10;
    for ( unsigned long i = 0; i < 100000; i++) {
        tnow = tnow + 104;
        float value = (rand() % 1000)/100;
        linear.update(value,tnow);
    }
    linear.outputSeconds();
    linear.outputMinutes();
    std::cout << "30s" << linear.means(30,tnow) << " " << linear.stdevs(30, tnow) <<  std::endl;
    std::cout << "1m" << linear.meanm(60, tnow) << " " << linear.stdevm(60, tnow) <<  std::endl;
    std::cout << "5m" << linear.meanm(5, tnow) << " " << linear.stdevm(5, tnow) <<  std::endl;
    std::cout << "15m" << linear.meanm(15, tnow) << " " << linear.stdevm(15, tnow) <<  std::endl;
    float mean30s = linear.means(30,tnow);
    float stdev30s = linear.stdevs(30, tnow);
    if ( fabs(mean30s - 4.5F ) > 0.45F ) {
        std::cout << " Test failed, mean 30s not within expected bounds of no more than 0.45 from 4.5: "  <<  std::endl;
        return false;

    }

    std::cout << " Done test " << std::endl;
    return true;

}

bool testRadianStatistic(void) {
    std::cout << "new RadianStatistic" << std::endl;
    RadianStatistic radians = RadianStatistic();
    std::cout << "Start Loading Radians" << std::endl;    
    unsigned long tnow = 10;
    float value = 0;
    for ( unsigned long i = 0; i < 100000; i++) {
        tnow = tnow + 104;
        float value = (((float)(rand() % (3140/2)))/1000.0F)-(M_PI/8.0F);
        radians.update(value,tnow);
    }
    radians.outputSeconds();
    radians.outputMinutes();
    std::cout << "30s" << radians.means(30,tnow) << " " << radians.stdevs(30, tnow) <<  std::endl;
    std::cout << "1m" << radians.meanm(60, tnow) << " " << radians.stdevm(60, tnow) <<  std::endl;
    std::cout << "5m" << radians.meanm(5, tnow) << " " << radians.stdevm(5, tnow) <<  std::endl;
    std::cout << "15m" << radians.meanm(15, tnow) << " " << radians.stdevm(15, tnow) <<  std::endl;

    float mean30s = radians.means(30,tnow);
    float stdev30s = radians.stdevs(30, tnow);
    if ( fabs(mean30s - 0.412302F ) > 0.04F ) {
        std::cout << " Test failed, mean 30s not within expected bounds of no more than 0.04 radians from 0.412302 " << std::endl;
        return false;

    }
    std::cout << " Test passed " << std::endl;
    return true;

}

bool testDegreeStatistic(void) {

    std::cout << "new DegreesStatistic" << std::endl;
    DegreesStatistic degrees = DegreesStatistic();
    


    std::cout << "Start Loading Degrees" << std::endl;    
    unsigned long tnow = 10;
    for ( unsigned long i = 0; i < 100000; i++) {
        tnow = tnow + 105;
        float value = ((float)(rand() % 3600)/100.0F)-10.0F;
        degrees.update(value,tnow);
    }
    degrees.outputSeconds();
    degrees.outputMinutes();
    std::cout << "30s" << degrees.means(30,tnow) << " " << degrees.stdevs(30, tnow) <<  std::endl;
    std::cout << "1m" << degrees.meanm(60, tnow) << " " << degrees.stdevm(60, tnow) <<  std::endl;
    std::cout << "5m" << degrees.meanm(5, tnow) << " " << degrees.stdevm(5, tnow) <<  std::endl;
    std::cout << "15m" << degrees.meanm(15, tnow) << " " << degrees.stdevm(15, tnow) <<  std::endl;
    float mean30s = degrees.means(30,tnow);
    float stdev30s = degrees.stdevs(30, tnow);
    if ( fabs(mean30s - 8.0F ) > 1.4F ) {
        std::cout << " Test failed, mean 30s not within expected bounds of no more than 1.4 degrees from 8: "  <<  std::endl;
        return false;

    }

    std::cout << " Done test " << std::endl;

    return true;
}



int main() {
    if (testLinearStatistics() &&
        testDegreeStatistic() &&
        testRadianStatistic() ) {
        return 0;
    }
    return 1;
}
#endif



#ifndef ARDUINO
#define TEST 1
#endif


#include "testmocks.h"

/*
#ifdef TEST
#include <iostream>
#define TRACE(x)
#define DEBUG(x) x
#define ERROR(x) x
#define SetN2kTemperature(a,b,c,d,e) std::cout << "Called SetN2kTemperature" << std::endl
#define max(x,y) fmax(x,y)
#define min(x,y) fmin(x,y)
#define analogRead(x)  512
#define N2kDoubleNA -1.0F
typedef struct _tN2kMsg {
    double dummy;
} tN2kMsg;

#else
#define TRACE(x)
#define DEBUG(x)
#define ERROR(x)
#endif
*/

#include <math.h>
#include "enviroMonitor.h"


#ifdef TEST



// unit tests can be run on a desktop.




int main() {
    if (testMonitor()) {
        return 0;
    }
    return 1;
}
#endif



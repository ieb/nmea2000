#ifndef __POLARTEST_H__
#define __POLARTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif


#include "testmocks.h"


#include <math.h>
#include "polar.h"


#ifdef TEST

// Unit tests ============================================================================



#import "pogo1250.h"

bool testPolarPerformance() {
    std::cout << "Testing Polar Performance " <<  std::endl;
    Polar_Performance polarPerf = Polar_Performance((char *)POGO1250_NAME, POGO1250_N_TWA, POGO1250_N_TWS, pogo1250Data_twa, pogo1250Data_tws, pogo1250Data_bsp);
    
    if (!polarPerf.checkPolarData()) {
        std::cout << "FAIL: Expected data to be Ok, reported as not ok. "  <<  std::endl;
        return false;
    }

    std::cout << "TWA = [";
    for(int twa = 0; twa < 1800; twa += 10) {
        std::cout << twa << " "; 
    }
    std::cout <<  "]" << std::endl;
    std::cout << "TWS = [";
        for ( int tws = 0; tws < 600; tws += 10 ) {
        std::cout <<  "]" << tws << " "; 
    }
    std::cout <<  "]" << std::endl;

    std::cout << "BSP = [";
    for(int twa = 0; twa < 1800; twa += 10) {
        for ( int tws = 0; tws < 600; tws += 10 ) {
            uint16_t polarBsp = polarPerf.getBoatSpeed(tws, twa);
            std::cout << polarBsp << " "  ;
        }
        std::cout << ";" << std::endl;
    }
    for(int twa = 1790; twa >= 0; twa -= 10) {
        for ( int tws = 0; tws < 600; tws += 10 ) {
            uint16_t polarBsp = polarPerf.getBoatSpeed(tws, twa);
            std::cout << polarBsp << " "  ;
        }
        std::cout << ";" << std::endl;
    }
    std::cout <<  "]" << std::endl;

    polarPerf.testInterpolate();

/*    for(int twa = 0; twa < 1800; twa += 10) {
        for ( int tws = 0; tws < 600; tws += 10 ) {
            uint16_t polarBsp = polarPerf.getBoatSpeed(tws, twa);
            uint16_t currentBsp = polarBsp/2;
            uint16_t perf = polarPerf.getBoatSpeedPerformance(tws, twa, currentBsp);
            std::cout << twa  << " " << tws << " " << polarBsp << " "  << currentBsp << " " << perf <<  std::endl;
        }
    } */
    std::cout << "Done Testing Polar Performance " <<   std::endl;
    return true;


}

bool Polar_Performance::testInterpolate(void) {

    for ( int i = -1; i < 12; i++) {
        float f =  interpolateBsp((float)i, 0.0F, 10.0F, 0.0F, 20.0F);
        std::cout << i << " " << f << std::endl;
        if ( i < 0 ) {
            if ( f != 0.0F) {
                std::cout << "FAIL: Expected 0 below range "  <<  std::endl;
                return false;
            }
        } else if ( i > 10 ) {
            if ( f != 20.0F) {
                std::cout << "FAIL: Expected 10 above range "  <<  std::endl;
                return false;
            }

        } else {
            if ( f != 2.0F*i) {
                std::cout << "FAIL: expected 2x i "  <<  std::endl;
                return false;
            }

        }
    }
    for ( int i = -1; i < 12; i++) {
        float f =  interpolateBsp((float)i, 0.0F, 10.0F, 0.0F, -20.0F);
        std::cout << i << " " << f << std::endl;
        if ( i < 0 ) {
            if ( f != 0.0F) {
                std::cout << "FAIL: Expected 0 below range "  <<  std::endl;
                return false;
            }
        } else if ( i > 10 ) {
            if ( f != -20.0F) {
                std::cout << "FAIL: Expected 10 above range "  <<  std::endl;
                return false;
            }

        } else {
            if ( f != -(2.0F*i)) {
                std::cout << "FAIL: expected 2x i "  <<  std::endl;
                return false;
            }

        }
    }
    for ( int i = -1; i < 12; i++) {
        std::cout << i << " " << interpolateBsp((float)i, 1.86F, 9.6F, 0.0F, 20.0F) << std::endl;
    }
    return true;
}

void calcTrueWind(double aws, double awa, double stw, double *tws, double *twa, bool debug=false) {
    if ( debug ) std::cout << "Input awa " << awa << " aws " << aws << " stw " << stw << std::endl;
    while ( awa > 180 ) awa = awa - 360;
    while ( awa < -180) awa = awa + 360;
    if ( debug ) std::cout << "Corrected awa " << awa << std::endl;
    double ctws = sqrt((stw*stw+aws*aws)-(2*stw*aws*cos(DegToRad(awa))));
    if ( debug ) std::cout << "ctws " << ctws << "," << ((double)fabs(ctws-aws)) << std::endl;
    double ctwa = 0.0F;
    if ( ctws > 1.0E-3F ) {
        ctwa = (aws*cos(DegToRad(awa))-stw)/ctws;
        if ( ctwa > 0.9999F || ctwa < -0.9999F) {
            ctwa = 0.0F;
            if ( debug ) std::cout << "reset ctwa " << ctwa << std::endl;
        } else {
            ctwa = RadToDeg(acos(ctwa));
        }
    }
    if ( debug ) std::cout << "ctwa " << ctwa << std::endl;
    if ( ctwa != ctwa ) {
        if ( debug ) {
            double a = cos(DegToRad(awa));
            std::cout << "cos(DegToRad(awa)) " <<  a << std::endl;
            a = aws*a;
            std::cout << "aws*cos(DegToRad(awa)) " << a << std::endl;
            a = a-stw;
            std::cout << "aws*cos(DegToRad(awa))-stw " << a << std::endl;
            a = a/ctws;
            std::cout << "(aws*cos(DegToRad(awa))-stw)/ctws " << a << std::endl;
            a = acos(a);
            std::cout << "acos((aws*cos(DegToRad(awa))-stw)/ctws) " << a << std::endl;
        }
    }
    if ( awa < 0) {
        ctwa = -ctwa;
    }
    if ( debug ) std::cout << "final ctwa " << ctwa << std::endl;
    *tws = ctws;
    *twa = ctwa;
}

bool testTrueWind() {
    std::cout << "Testing calcTrueWind" << std::endl;
    std::cout << "aws,awa,stw,tws,twa,deltawa" << std::endl;
    double tws = 0.0F, twa = 0.0F; 
    for (double aws = 0.0; aws < 40; aws +=12.2F) {
        for (double stw = 0.0; stw < 12; stw += 2.3F) {
            for(double awa = -180.0F; awa <= 180.0F; awa+=15.0F) {
                calcTrueWind(aws,awa,stw,&tws,&twa);
                double cawa = awa;
                if ( awa < -180.0F ) {
                    cawa = awa+360.0F;
                } else if ( awa > 180.0F) {
                    cawa = awa-360.0F;
                }
                double diffwa = twa - cawa;
                std::cout << aws << "," << cawa << "," << stw << "," <<  tws << "," << twa << "," << diffwa << std::endl;
                if ( tws != tws || twa != twa ) { // check fr nan.
                    calcTrueWind(aws,awa,stw,&tws,&twa,true);
                    return false;
                }
            }
        }
    }
    return true;
}
bool testACos() {
    std::cout <<  "Testing acos " << std::endl;
    for ( double d = -1; d < 1; d += 0.1 ) {
        double n = acos(d);
        if ( n != n) {
            std::cout << d << "," << n << std::endl;
            return false;
        }
    }
    std::cout <<  "Testing acos OK " << std::endl;
    return true;
}
bool testCos() {
    std::cout <<  "Testing cos " << std::endl;
    for ( double d = -180; d < +180; d += 5 ) {
        double n = cos(DegToRad(d));
        if ( n != n) {
            std::cout << d << "," << n << std::endl;
            return false;
        }
    }
    std::cout <<  "Testing cos OK" << std::endl;
    return true;
}

#endif
#endif



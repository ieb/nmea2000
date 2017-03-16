#define TEST 1

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
#include "polar.h"




Statistic::Statistic() {
    for(int i = 0; i < 60; i++) {
        seconds[i] = 0;
        minutes[i] = 0;        
    }
    islast = 0;
    imlast = 0;
}


/** Public methods ====================================================== */
void Statistic::fill(int s, int e, int size, float *values, float value) {
    if ( e > s ) {
        for(int i = s; i < e-1; i++) {
            values[i] = value;
        }            
    } else {
        for(int i = s; i < size; i++) {
            values[i] = value;
        }            
        for(int i = 0; i < e-1; i++) {
            values[i] = value;
        }            
    }
}

void Statistic::update(float v, unsigned long tnow) {
    v = fromInput(v);
    int isnow = (tnow/1000) % 60;
    if ( isnow != islast ) {
        // fill all buckets between islast and now with the value in islast.
        fill(islast, isnow, 60, seconds, seconds[islast]);
        seconds[isnow] = v;
        islast = isnow;
    } else {
        // accumulate the changes in the current bucket.
        float va[2] = { seconds[isnow], v};
        seconds[isnow] = mean(va,2,0,2);
    }
    int imnow = (tnow/60000) % 60;
    if ( imnow != imlast ) {
        float cmean = mean(seconds, 60, 0, 60);
        // minute went forwards compared to last time,
        // get the mean and fill all betwene imnow and imlast
        fill(imlast, imnow, 60, minutes, minutes[imlast]);
        // might be more valid to interpolate between then and now.
        minutes[imnow] = cmean;
        imlast = imnow;
    }

}



float Statistic::means(int nseconds, unsigned long tnow) {
    if ( nseconds > 60) {
        nseconds = 60;
    }
    int ito = (tnow/1000) % 60;
    int ifrom = ito - nseconds;
    if (ifrom < 0 ) {
        ifrom = ifrom + 60;
    }
    DEBUG(std::cout << "(" << ifrom << ":" << ito << ")");
    return toOutput(mean(seconds, nseconds, ifrom, ito));
}



float Statistic::meanm(int nminutes, unsigned long tnow) {
    if ( nminutes > 60) {
        nminutes = 60;
    }
    int ito = (tnow/1000) % 60;
    int ifrom = ito - nminutes;
    if (ifrom < 0 ) {
        ifrom = ifrom + 60;
    }
    DEBUG(std::cout << "(" << ifrom << ":" << ito << ")");
    return toOutput(mean(minutes, nminutes, ifrom, ito));
}

float Statistic::stdevs(int nseconds, unsigned long tnow) {
    if ( nseconds > 60) {
        nseconds = 60;
    }
    int ito = (tnow/1000) % 60;
    int ifrom = ito - nseconds;
    if (ifrom < 0 ) {
        ifrom = ifrom + 60;
    }
    DEBUG(std::cout << "(" << ifrom << ":" << ito << ")");
    return toOutput(stdev(minutes, nseconds, ifrom, ito));
}
float Statistic::stdevm(int nminutes, unsigned long tnow) {
    if ( nminutes > 60) {
        nminutes = 60;
    }
    int ito = (tnow/1000) % 60;
    int ifrom = ito - nminutes;
    if (ifrom < 0 ) {
        ifrom = ifrom + 60;
    }
    DEBUG(std::cout << "(" << ifrom << ":" << ito << ")");
    return toOutput(stdev(minutes, nminutes, ifrom, ito));
}


/** Private methods ====================================================== */



float Statistic::mean(float *values, int ntotal, int s, int e) {
    float m = 0;
    int i = s, n = 0;
    while( n < ntotal) {
        m = m + values[i];
        n++;
        i++;
        if ( i == e ) {
            break;
        } else if (i == 60) {
            i = 0;
        }
    }
    if (n == 0) {
        return 0;
    }
    return m/n;
}


float Statistic::stdev(float *values, int ntotal, int s, int e) {
    float cmean = mean(values, ntotal, s, e);
    float m = 0, x = 0;
    int i = s, n = 0;
    while( n < ntotal) {
        x = values[i] - cmean;
        m = m+(x*x);
        n++;
        i++;
        if ( i == e ) {
            break;
        } else if (i == 60) {
            i = 0;
        }
    }
    if (n == 0) {
        return 0;
    }
    return sqrt(m/n);
}


RadianStatistic::RadianStatistic() {
}

float RadianStatistic::mean(float *values, int ntotal, int s, int e) {
    float sv = 0, cv = 0;
    int i = s, n = 0;
    while( n < ntotal) {
        sv = sv+sin(values[i]);
        cv = cv+cos(values[i]);
        i++;
        n++;
        if ( i == e ) {
            break;
        } else if (i == 60) {
            i = 0;
        }
    }
    if (n == 0) {
        return 0;
    }
    return atan2(sv/n,cv/n);    
}



float RadianStatistic::stdev(float *values, int ntotal, int s, int e) {
    float sv = 0, cv = 0;
    int i = s, n = 0;
    while( n < ntotal) {
        sv = sv+sin(values[i]);
        cv = cv+cos(values[i]);
        i++;
        n++;
        if ( i == e ) {
            break;
        } else if (i == 60) {
            i = 0;
        }
    }
    if (n == 0) {
        return 0;
    }
    sv = sv/n;
    cv = cv/n;
    return sqrt(-log(sv*sv+cv*cv));    
}


DegreesStatistic::DegreesStatistic() {
}

float DegreesStatistic::fromInput(float v) {
    return v*(M_PI/180.0F);
}

float DegreesStatistic::toOutput(float v) {
    return v*(180.0F/M_PI);
}

Polar_Performance::Polar_Performance(char *polar_name,
    int ntwa_rows, 
    int ntws_columns,
    const uint16_t *twa_rows,
    const uint16_t *tws_columns,
    const uint16_t *bspdata) {
    ntwa = ntwa_rows;
    ntws = ntws_columns;
    tws = tws_columns;
    twa = twa_rows;
    bsp = bspdata;
    checkPolarData();
}

/**
 * 
 */
uint8_t Polar_Performance::checkPolarData(void) {
    dataOk = POLAR_DATA_OK;
    if (tws[0] != 0 ) {
        ERROR(std::cout << "TWS column 0 is not 0 " << std::endl);
        dataOk = POLAR_DATA_TWS_ZERO_ERROR;
    } 
    if (twa[0] != 0 ) {
        ERROR(std::cout << "TWA row 0 is not 0 " << std::endl);
        dataOk = POLAR_DATA_TWA_ZERO_ERROR;
    } 
    if (twa[ntwa] != POLAR_ENDMARK ) {
        ERROR(std::cout << "TWA array length not right " << std::endl);
        dataOk = POLAR_DATA_TWA_LENGTH_ERROR;
    }
    if (tws[ntws] != POLAR_ENDMARK ) {
        ERROR(std::cout << "TWS array length not right " << std::endl);
        dataOk = POLAR_DATA_TWS_LENGTH_ERROR;
    }
    if (bsp[ntws*ntwa] != POLAR_ENDMARK ) {
        ERROR(std::cout << "BSP array length not right " << std::endl);
        dataOk = POLAR_DATA_BSP_LENGTH_ERROR;
    }
    for ( int i = 1; i < ntws; i++) {
        if( tws[i-1] > tws[i] ) {
            ERROR(std::cout << "TWS array not ordered " << tws[i-1] << ">" << tws[i] << std::endl);
            dataOk = POLAR_DATA_TWS_NOT_ORDERED;
        }
    }
    for ( int i = 1; i < ntwa; i++) {
        if( twa[i-1] > twa[i] ) {
            ERROR(std::cout << "TWA array not ordered " << twa[i-1] << ">" << twa[i] << std::endl);
            dataOk = POLAR_DATA_TWA_NOT_ORDERED;
        }
    }

    if ( dataOk  != POLAR_DATA_OK ) {
        ERROR(std::cout << "TWS ");
        for ( int i = 0; i <= ntws; i++) {
            ERROR(std::cout << i << ":" << tws[i] << ",");
        }
        ERROR(std::cout << std::endl);
        ERROR(std::cout << "TWA ");
        for ( int i = 0; i <= ntwa; i++) {
            ERROR(std::cout << i << ":" << twa[i] << ",");
        }
        ERROR(std::cout << std::endl);
        ERROR(std::cout << "BSP " << std::endl);
        for ( int i = 0; i <= ntwa; i++) {
            for ( int j = 0; j <= ntws; i++) {
                ERROR(std::cout << bsp[BSP_INDEX(i,j,ntws)] << ",");
            }
            ERROR(std::cout << std::endl);
        }
    }
    ERROR(std::cout << "Polar Data Ok " << std::endl);
    return dataOk == POLAR_DATA_OK;
}

int Polar_Performance::findGreater(uint16_t val, const uint16_t *values, int nvalues) {
    if ( val <= values[0] ) {
        return 0;
    }
    if ( val > values[nvalues-1] ) {
        return nvalues-1;
    }
    int i = 1;
    while( i < nvalues) {
        if ( values[i] > val ) break;
        i++;
    }
    TRACE(std::cout << val << ":" << values[i] << std::endl);
    return i;
}

int Polar_Performance::lowerRange(int val, int nvalues) {
    if (val == 0) {
        return 0;
    } else if (val >= nvalues ) {
        return nvalues-2;
    }
    return val - 1;
}

int Polar_Performance::upperRange(int val, int nvalues) {
    if (val == 0) {
        return 1;
    } else if (val >= nvalues ) {
        return nvalues-1;
    }
    return val;
}


float Polar_Performance::interpolateBsp(float val, float vall, float valh, float bspl, float bsph) {
    float r = 0.0F;
    TRACE(std::cout << "Interpolating " <<  bspl << ":" << bsph << " for " << val << " between " << vall << ":" << valh);
    if ( val >= valh) {
        r = bsph;
    } else if ( val <= vall ) {
        r = bspl;
    } else if ( (valh - vall) < 1.0E-8F) {
         r = bspl+(bsph-bspl)*((val-vall)/1.0E-8F);
    } else {
        r = bspl+(bsph-bspl)*((val-vall)/(valh-vall));        
    }
    TRACE(std::cout <<  " = " << r << std::endl);
    TRACE(std::cout << bspl << ":" << (bsph-bspl) << ":" << (val-vall) << ":" << (valh-vall) << std::endl);
    return r;
}

float Polar_Performance::getRealBoatSpeed(uint16_t ctws, uint16_t ctwa) {
    int itwah = findGreater(ctwa, twa, ntwa);
    int itwsh = findGreater(ctws, tws, ntws);
    int itwal = lowerRange(itwah, ntwa);
    int itwsl = lowerRange(itwsh, ntws);
    itwah = upperRange(itwah, ntwa);
    itwsh = upperRange(itwsh, ntws);

    TRACE(std::cout << "TWA " << twa[itwal] << ":" << ctwa << ":" << twa[itwah] << std::endl);
    TRACE(std::cout << "TWS " << tws[itwsl] << ":" << ctws << ":" << tws[itwsh] << std::endl);
    TRACE(std::cout << "BSP " << bsp[BSP_INDEX(itwal,itwsl,ntws)] 
        << ":" << bsp[BSP_INDEX(itwal,itwsh,ntws)] 
        << ":" << bsp[BSP_INDEX(itwah,itwsl,ntws)] 
        << ":" << bsp[BSP_INDEX(itwah,itwsh,ntws)]
        << std::endl);


    // the 4 points surrounding the value required denoted by itwal, itwah, itwsl, itwsh
    // interpolate the speed at the twa for the upper and lower tws.

    float bsp_twsl = interpolateBsp((float)ctwa,
        (float)twa[itwal],
        (float)twa[itwah],
        (float)bsp[BSP_INDEX(itwal,itwsl,ntws)],
        (float)bsp[BSP_INDEX(itwah,itwsl,ntws)]);
    float bsp_twsh = interpolateBsp((float)ctwa,
        (float)twa[itwal],
        (float)twa[itwah],
        (float)bsp[BSP_INDEX(itwal,itwsh,ntws)],
        (float)bsp[BSP_INDEX(itwah,itwsh,ntws)]);
    return interpolateBsp((float)ctws,
        (float)tws[itwsl],
        (float)tws[itwsh],
        bsp_twsl,
        bsp_twsh);
}

uint16_t Polar_Performance::getBoatSpeed(uint16_t ctws, uint16_t ctwa) {
    return (uint16_t) getRealBoatSpeed(ctws, ctwa);
}
uint16_t Polar_Performance::getBoatSpeedPerformance(uint16_t ctws, uint16_t ctwa, uint16_t cbsp) {
    uint16_t targetBoatSpeed = getRealBoatSpeed(ctws, ctwa);
    if ( targetBoatSpeed == 0) {
        return 100;
    }
    return (uint16_t)(100.0*(cbsp/targetBoatSpeed));

}

#ifdef TEST

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


int main() {
    if (testLinearStatistics() &&
        testDegreeStatistic() &&
        testRadianStatistic() &&
        testPolarPerformance() ) {
        return 0;
    }
    return 1;
}
#endif



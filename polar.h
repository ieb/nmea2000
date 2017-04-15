
#ifndef __POLAR_PERF_H__
#define __POLAR_PERF_H__

#include <stdint.h>

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

#define BSP_INDEX(r,c,rs)   c+r*rs

#define POLAR_ENDMARK 30543 // magic number to indicate the end of each array, checked at initalisation, must not be part of the dataset.



typedef enum {
        POLAR_DATA_OK = 0x00,
        POLAR_DATA_TWS_ZERO_ERROR = 0x01,
        POLAR_DATA_TWA_ZERO_ERROR = 0x02,
        POLAR_DATA_TWA_LENGTH_ERROR = 0x04,
        POLAR_DATA_TWS_LENGTH_ERROR = 0x08,
        POLAR_DATA_BSP_LENGTH_ERROR = 0x10,
        POLAR_DATA_TWS_NOT_ORDERED = 0x20,
        POLAR_DATA_TWA_NOT_ORDERED = 0x40,
        POLAR_DATA_NO_DATA = 0x80
        } polarErrorCodes;



/* Unified sensor driver for the accelerometer */
class Polar_Performance {
  public:
    Polar_Performance(char *polar_name,
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
     * get the polar boatspeed for a given tws and twa
     * bsp and tws are in 1/10ths of a kn.
     */
    float getBoatSpeed(float ctws, float ctwa) {
        return getRealBoatSpeed(ctws, ctwa);
    }
    /**
     * Get the polar performance for a given tws, twa and real bsp. 
     * bsp and tws are in 1/10ths of a kn.
     * result is a % 0. > 100% is possible.
     */
    float getBoatSpeedPerformance(float ctws, float ctwa, float cbsp) {
        float targetBoatSpeed = getRealBoatSpeed(ctws, ctwa);
        if ( targetBoatSpeed == 0.0F) {
            return 100.0F;
        }
        return (100.0F*(cbsp/targetBoatSpeed));
    }


/**
 * 
 */
    uint8_t checkPolarData(void) {
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
#ifdef TEST
    bool testInterpolate(void);
#endif


 private:
    char *name;
    int ntws;
    int ntwa;
    uint8_t dataOk;
    const uint16_t *tws;
    const uint16_t *twa;
    const uint16_t *bsp;



    int findGreater(uint16_t val, const uint16_t *values, int nvalues) {
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

    int lowerRange(int val, int nvalues) {
        if (val == 0) {
            return 0;
        } else if (val >= nvalues ) {
            return nvalues-2;
        }
        return val - 1;
    }

    int upperRange(int val, int nvalues) {
        if (val == 0) {
            return 1;
        } else if (val >= nvalues ) {
            return nvalues-1;
        }
        return val;
    }


    // in 1/10ths of a kn. ie 1 = 1/10th 10 == 1kn.
    float interpolateBsp(float val, float vall, float valh, float bspl, float bsph) {
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

    // in kn.
    float getRealBoatSpeed(float fctws, float fctwa) {
        uint16_t ctws = (uint16_t)(fctws*10.0F), ctwa = (uint16_t)(fctwa*10.0F);
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
        return 0.1F*interpolateBsp((float)ctws,
            (float)tws[itwsl],
            (float)tws[itwsh],
            bsp_twsl,
            bsp_twsh);
    }




};

#ifdef TEST
typedef struct tN2kMsg_s {
    double dummyForTesting;
} tN2kMsg;
#define SetN2kEngineDynamicParam(a, b, c, d, e, f, g, h,  i, j, k, l )std::cout << "Called SetN2kEngineDynamicParam" << std::endl
#define SetN2kEngineParamRapid(a, b, c) std::cout << "Called SetN2kEngineParamRapid" << std::endl
#define N2kDoubleNA 1
#define N2kInt8NA 1
#define DegToRad(v) ((v)/180.0*3.1415926535897932384626433832795)
#define RadToDeg(v) ((v)*180.0/3.1415926535897932384626433832795)
#else
#include <N2kMessages.h>
#endif

#include "statistic.h"
#define PolarMonitor_MESSAGES  127489L,  127488L


class PolarMonitor {

public:
    PolarMonitor(Polar_Performance *polarPerformance, 
        Statistic *twaStatistic, 
        Statistic *twsStatistic, 
        Statistic *bspStatistic,
        int8_t windowS = 15 // the window to average over in s
        ) {
        _twaStatistic = twaStatistic;
        _twsStatistic = twsStatistic;
        _bspStatistic = bspStatistic;
        _polarPerformance = polarPerformance;
        periods = windowS;
    }


    void calcTrueWind(double aws, double awa, double stw, double *tws, double *twa, bool debug=false) {
        while ( awa > 180 ) awa = awa - 360;
        while ( awa < -180) awa = awa + 360;
        double ctws = sqrt((stw*stw+aws*aws)-(2*stw*aws*cos(DegToRad(awa))));
        double ctwa = 0.0F;
        if ( ctws > 1.0E-3F ) {
            ctwa = (aws*cos(DegToRad(awa))-stw)/ctws;
            if ( ctwa > 0.9999F || ctwa < -0.9999F) {
                ctwa = 0.0F;
            } else {
                ctwa = RadToDeg(acos(ctwa));
            }
        }
        if ( awa < 0) {
            ctwa = -ctwa;
        }
        *tws = ctws;
        *twa = ctwa;
    }


    void read(unsigned long tnow) {
        meanBSP = _bspStatistic->means(periods, tnow);
        stdevBSP = _bspStatistic->stdevs(periods, tnow);
        meanAWS = _twsStatistic->means(periods, tnow);
        stdevAWS = _twsStatistic->stdevs(periods, tnow);
        meanAWA = _twaStatistic->means(periods, tnow);
        stdevAWA = _twaStatistic->stdevs(periods, tnow);
        calcTrueWind(meanAWS, meanAWA, meanBSP, &meanTWS, &meanTWA);
        stdevTWS = stdevAWS; // this is not strictly correct, but good enough.
        stdevTWA = stdevAWA;
        targetBSP = _polarPerformance->getBoatSpeed(meanTWS, meanTWA);
        performance = _polarPerformance->getBoatSpeedPerformance(meanTWS, meanTWA, meanBSP);
    }




    void fillUsingEnginDynamicMessage(tN2kMsg &N2kMsg) {
        SetN2kEngineDynamicParam(N2kMsg, 1, 
            N2kDoubleNA, // double EngineOilPress, 
            N2kDoubleNA, // double EngineOilTemp, 
            N2kDoubleNA, // double EngineCoolantTemp, 
            N2kDoubleNA, // double AltenatorVoltage,
            N2kDoubleNA, // double FuelRate, 
            N2kDoubleNA, // double EngineHours, 
            N2kDoubleNA, // double EngineCoolantPress=N2kDoubleNA, 
            N2kDoubleNA, // double EngineFuelPress=N2kDoubleNA, 
            (int8_t) performance, // int8_t EngineLoad=N2kInt8NA,
            N2kInt8NA  // int8_t EngineTorque=N2kInt8NA, 
            );
            //           bool flagCheckEngine=false,       bool flagOverTemp=false,         bool flagLowOilPress=false,         bool flagLowOilLevel=false, 
            //           bool flagLowFuelPress=false,      bool flagLowSystemVoltage=false, bool flagLowCoolantLevel=false,     bool flagWaterFlow=false, 
            //           bool flagWaterInFuel=false,       bool flagChargeIndicator=false,  bool flagPreheatIndicator=false,    bool flagHighBoostPress=false, 
            //           bool flagRevLimitExceeded=false,  bool flagEgrSystem=false,        bool flagTPS=false,                 bool flagEmergencyStopMode=false, 
            //           bool flagWarning1=false,          bool flagWarning2=false,         bool flagPowerReduction=false,      bool flagMaintenanceNeeded=false, 
            //           bool flagEngineCommError=false,   bool flagSubThrottle=false,      bool flagNeutralStartProtect=false, bool flagEngineShuttingDown=false
    }

    void fillUsingEnginRapidMessage(tN2kMsg &N2kMsg) {
        SetN2kEngineParamRapid(N2kMsg, 
            1, // unsigned char EngineInstance, 
            performance // double EngineSpeed,
            ); 
            // double EngineBoostPressure=N2kDoubleNA, int8_t EngineTiltTrim=N2kInt8NA) {
    }
    void fillUsingPolarPerformanceMessage(tN2kMsg &N2kMsg) {
        // TODO, find a custom message PGN that can be used.
    }


private:
    double performance;
    double targetBSP;
    double meanBSP;
    double stdevBSP;
    double meanTWS;
    double stdevTWS;
    double meanTWA;
    double stdevTWA;
    double meanAWS;
    double stdevAWS;
    double meanAWA;
    double stdevAWA;
    int8_t periods;
    Polar_Performance *_polarPerformance;
    Statistic *_twaStatistic;
    Statistic *_twsStatistic; 
    Statistic *_bspStatistic;

};





#endif


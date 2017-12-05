
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

// 100 slots for wind speed
#define N_SPEEDS 100
// 180 slots for angles
#define N_DEGREES 180
// 0.5kn per slot for windspeed
#define SPEED_TO_INDEX(x) (int)(2.0F*(float)x);
// 1 degree per slot for angle, however, 0 and 1 are treated as the same.
#define ANGLE_TO_INDEX(x) x-1
#define INDEX_TO_ANGLE(x) x+1

#define MAX_POLAR_FILE_TWA 36
#define MAX_POLAR_FILE_TWS 20

// Storage for reading a polar from SD card. 2*(36+1+20+1+36*20+1) = 1558 bytes

#define POLAR_ENDMARK 30543 // magic number to indicate the end of each array, checked at initalisation, must not be part of the dataset.

// storage space for the precalculated polar. Resoution is 1 degree, 0.5kn aws
// Data is stored as a uint8 giving 0.1kn accuracy with a max speed of 25.5kn.
// once the interpolated polar is built, it can be used by lookup and scan.
uint8_t interpolatedPolar[180*100];

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


typedef struct  {
    float polarstw;  // m/s
    float polarstwRatio; // %
    float polarvmg; // m/s
    float vmg;      // m/s
    float polarvmgRatio; // %
    float targetvmg; // m/s
    float targetstw; // m/s
    float targettwa; // Radians
    float tackHeadingTrue; // Radians
    float tackTrackTrue; // Radians
    float tackTrackMagnetic; // Radians
    float tackHeadingMagnetic; // Radians
} performance_data_t;


/* Unified sensor driver for the accelerometer */
class Polar_Performance {
  public:
    Polar_Performance(char *polar_name,
        int ntwa_rows,
        int ntws_columns,
        uint16_t *twa_rows,
        uint16_t *tws_columns,
        uint16_t *bspdata) {
        ntwa = ntwa_rows;
        ntws = ntws_columns;
        tws = tws_columns;
        twa = twa_rows;
        bsp = bspdata;
    }


    void init() {
        if ( checkPolarData() ) {
            interpolatedPolarAvailable = false;
            unsigned long start = millis();
            loadInterpolatedPolar();
            LOG(F("Polar load took "));
            LOGC((millis()-start));
            LOGN(" ms");
        }
    }

    void updateConfiguration(Configuration *config) {
        Stream *polarStream = config->openPolar();
        if ( polarStream == NULL) {
            interpolatedPolarAvailable = false;
            loadInterpolatedPolar();
            return;
        }
        char commandBuffer[10];
        char lastChar;

        uint16_t ntwa, ntws;
        uint16_t rtwa[MAX_POLAR_FILE_TWA+1];
        uint16_t rtws[MAX_POLAR_FILE_TWS+1];
        uint16_t rstw[MAX_POLAR_FILE_TWA*MAX_POLAR_FILE_TWS+1];
        int state = 0, cbp = 0;
        while (polarStream->available() > 0 && (lastChar = polarStream->read()) != -1 && state != 0x07) {
          switch(lastChar) {
            case ' ':
              commandBuffer[cbp++] = 0;
              cbp = 0;
              if ( strncmp("twa",commandBuffer,3) == 0 ) {
                ntwa = readNumbers(polarStream, MAX_POLAR_FILE_TWA, rtwa);
                state = state | 0x01;
                LOG("Polar twa table contains ");
                LOGC(ntwa);
                LOGN(" elements.");
              } else if ( strncmp("tws",commandBuffer,3) == 0 ) {
                ntws =  readNumbers(polarStream, MAX_POLAR_FILE_TWS, rtws);
                state = state | 0x02;
                LOG("Polar tws table contains ");
                LOGC(ntws);
                LOGN(" elements.");
              } else if ( state > 0 && strncmp("stw",commandBuffer,3) == 0 ) {
                uint16_t nr = readNumbers(polarStream, MAX_POLAR_FILE_TWA*MAX_POLAR_FILE_TWS, rstw);
                if ( nr != ntwa*ntws)  {
                    LOG("Error: Expected polar stw table to be ");
                    LOGC((ntwa*ntws));
                    LOGC(" elements but found ");
                    LOGC(nr);
                    LOGN(" elements.");
                  } else {
                    LOG("Polar stw table contains ");
                    LOGC(nr);
                    LOGN(" elements.");
                    state = state | 0x04;
                }
              }
              break;
            default:
              if (lastChar >= Space && lastChar < Del && cbp < MAX_LINE_LENGTH) {
                  commandBuffer[cbp++] = lastChar;
              }
              break;
          }
        }

        if ( state == 0x07 ) {

            uint16_t *stws = tws, *stwa = twa, *sstw = bsp;
            int sntwa = ntwa, sntws = ntws;
            this->ntwa = ntwa;
            this->ntws = ntws;
            rtws[ntws] = POLAR_ENDMARK;
            rtwa[ntwa] = POLAR_ENDMARK;
            rstw[ntwa*ntws] = POLAR_ENDMARK;
            tws = rtws;
            twa = rtwa;
            bsp = rstw;
            if ( checkPolarData() ) {
                interpolatedPolarAvailable = false;
                loadInterpolatedPolar();
            } else {
                LOGLN("Error in polar data, not loading.");
            }
            ntwa = sntws;
            ntws = sntwa;
            tws = stws;
            twa = stwa;
            bsp = sstw;

        }
        config->closePolar();
  }

  int readNumbers(Stream *from, int n, uint16_t *p) {
    char lastChar;
    char buf[10];
    bool inArray = false;
    int i = 0, j = 0;
    while (from->available() > 0 && (lastChar = from->read()) != -1 && j < n ) {
        if ( lastChar == '[' ) {
            inArray = true;
        } else if ( lastChar == ']' ) {
            break;
        } else if ( inArray ) {
            if ( i < 9 && lastChar >= 0 && lastChar <= 9) {
                buf[i++] = lastChar;
            } else if ( i > 0 ) {
                buf[i] = 0;
                p[j++] = atoi(buf);
                i = 0;
            }
        }
    }
    return j;
  }


  // All paramters are in SI Units.
  void fillPerformanceData(float ctws, float ctwa, float cstw, float headingTrue, float magneticVariation, float leeway,  performance_data_t *perfdata) {
    perfdata->polarstw = knotsToMs(getTargetSTW(msToKnots(ctws), RadToDeg(ctwa)));
    if ( perfdata->polarstw < 1E-3 ) {
        perfdata->polarstwRatio = 100.0F; 
    } else {
        perfdata->polarstwRatio = 100.0F*cstw/perfdata->polarstw; 
    }
    perfdata->polarvmg = perfdata->polarstw*cos(ctwa);
    perfdata->vmg = cstw*cos(ctwa);
    if ( fabs(perfdata->polarvmg) > 1E-3 ) {
        perfdata->polarvmgRatio = 100.0F*perfdata->vmg/perfdata->polarvmg;
    }

    // find the target for best vmg on this tack.
    perfdata->targetvmg = 0.0F;
    bool downwind = fabs(ctwa) > PI/2;
    int twsstart = polarIndex(msToKnots(ctws),0); 
    for ( int i = 0; i < N_DEGREES; i++) {
        float ttwa = DegToRad((float)INDEX_TO_ANGLE(i));
        float tstw = knotsToMs(0.1F*(float)(interpolatedPolar[i+twsstart]));
        float vmg = tstw*cos(ttwa);
        if ( ((downwind) && (vmg < perfdata->targetvmg)) || 
            ((!downwind) && (vmg > perfdata->targetvmg)) ) {
            perfdata->targetvmg = vmg;
            perfdata->targetstw = tstw;
            perfdata->targettwa = ttwa;
        }
    }
    // adjust for port and starboard.
    if (ctwa < 0.0F) {
        perfdata->targettwa = -perfdata->targettwa;
    }
    // calculate the angles on the next tack.
    perfdata->tackHeadingTrue = fixAngle(headingTrue-ctwa-perfdata->targettwa);
    perfdata->tackTrackTrue = fixAngle(headingTrue-ctwa-perfdata->targettwa-2*leeway);
    perfdata->tackTrackMagnetic = perfdata->tackTrackTrue+ magneticVariation;
    perfdata->tackHeadingMagnetic = perfdata->tackHeadingTrue+ magneticVariation;


  }






/**
 *
 */
    bool checkPolarData(void) {
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
    uint16_t *tws;
    uint16_t *twa;
    uint16_t *bsp;
    uint8_t interpolatedPolar[N_DEGREES*N_SPEEDS];
    bool interpolatedPolarAvailable;



    uint16_t polarIndex(float ftws, float ftwa) {
        int itws = SPEED_TO_INDEX(ftws);
        int itwa = ANGLE_TO_INDEX(ftwa);
        if (itwa < 1) {
            itwa = 0;
        } else if ( itwa > N_DEGREES) {
            itwa = N_DEGREES;
        }
        if ( itws < 0) {
            itws = 0;
        } else if (itws > N_SPEEDS) { // 50kn
            itws = N_SPEEDS;
        }
        return itws + N_SPEEDS*itwa;
    }



    void loadInterpolatedPolar() {
        float ftws, ftwa, fstw;
        int stw;
        LOGC("twA=[");
        for(int twa = 1; twa <= 180; twa++) {
            ftwa = twa;
            LOGC(" ");
            LOGC(ftwa);
        }
        LOGN("];");
        LOGC("twS=[");
        for ( int tws = 0; tws < 100; tws++) {
            ftws = 0.5F*tws;
            LOGC(" ");
            LOGC(ftws);
        }
        LOGN("];");
        LOGC("STW=[");
        for(int twa = 1; twa <= 180; twa++) {
            for ( int tws = 0; tws < 100; tws++) {
                ftwa = twa;
                ftws = 0.5F*tws;
                fstw = getTargetSTW(ftws, ftwa);
                if ( fstw > 25.5 ) {
                    // could do some compression here, like > 20Kn, use 1Kn steps to get to 30Kn.
                    LOGLN("ERROR: Polar speed > 25.5kn, setting to 25.5kn");
                    stw = 255;
                } else {
                    stw = (int)(10*fstw);
                }
                LOGC(" ");
                LOGC(stw);
                interpolatedPolar[polarIndex(ftws,ftwa)] = stw;
            }
            LOGN(";");
        }
        LOGN("];");
        interpolatedPolarAvailable = true;
        LOGC("STW=[");
        for(int twa = 1; twa <= 180; twa+=10) {
            for ( int tws = 0; tws < 100; tws+=10) {
                ftwa = twa;
                ftws = 0.5F*tws;
                fstw = getTargetSTW(ftws, ftwa);
                LOGC(" ");
                LOGC(fstw);
            }
            LOGN(";");
        }
        LOGN("];");
    }





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
    float getTargetSTW(float fctws, float fctwa) {
        if ( interpolatedPolarAvailable ) {
            return 0.1F*(float)(interpolatedPolar[polarIndex(fctws, fctwa)]);
        } else {
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
            return 0.01F*interpolateBsp((float)ctws,
                (float)tws[itwsl],
                (float)tws[itwsh],
                bsp_twsl,
                bsp_twsh);
        }
    }




};






#endif




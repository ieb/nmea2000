
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






#endif


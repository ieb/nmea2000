
#ifndef __POLAR_PERF_H__
#define __POLAR_PERF_H__

#include <stdint.h>


#define BSP_INDEX(r,c,rs)   c+r*rs

#define POLAR_ENDMARK 30543 // magic number to indicate the end of each array, checked at initalisation, must not be part of the dataset.


class Statistic {
    public:
        /** Constructor for Statistics
         * statsType, the type of statistic being recorded. This controls how the 
         * stdev and mean is calculated and what units the interface talks in.
         */
        Statistic();

        /**
         * Update the statistic at a known time supplied in tnow.
         * v the value of the update.
         * tnow time in ms.
         */
        void update(float v, unsigned long tnow);
        /**
         * get the mean for the past nseconds where nseconds is in the range 1 - 60.
         */
        float means(int nseconds, unsigned long tnow);
        /**
         * get the mean for the past nminutes where nminites is in the range 1 - 60.
         */
        float meanm(int nmins, unsigned long tnow);
        /**
         * get the standard deviation for the past nseconds where nseconds is in the range 1 - 60.
         */
        float stdevs(int nseconds, unsigned long tnow);
        /**
         * get the standard deviation for the past nminutes where nminites is in the range 1 - 60.
         */
        float stdevm(int nmins,  unsigned long tnow);
#ifdef TEST
        void outputSeconds(void);
        void outputMinutes(void);
        float getSecond(int n);
        float getMinute(int n);
#endif
    private:
        int islast;
        int imlast;
        float seconds[60];
        float minutes[60];
        void fill(int s, int e, int size, float *values, float value);
        virtual float mean(float *values, int ntotal, int s, int e);
        virtual float stdev(float *values, int ntotal, int s, int e);
        virtual float toOutput(float v) { return v;};
        virtual float fromInput(float v) { return v;};
};


class RadianStatistic : public Statistic {
    public:
        /** Constructor for Statistics
         * statsType, the type of statistic being recorded. This controls how the 
         * stdev and mean is calculated and what units the interface talks in.
         */
        RadianStatistic();
    private:
        virtual float mean(float *values, int ntotal, int s, int e);
        virtual float stdev(float *values, int ntotal, int s, int e);

};

class DegreesStatistic :  public RadianStatistic {
    public:
        /** Constructor for Statistics
         * statsType, the type of statistic being recorded. This controls how the 
         * stdev and mean is calculated and what units the interface talks in.
         */
        DegreesStatistic();
    private:
        virtual float toOutput(float v);
        virtual float fromInput(float v);

};

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
    Polar_Performance(char *polar_name,  int ntwa_rows, int ntws_columns, const uint16_t *twa_rows, const uint16_t *tws_columns, const uint16_t *bspdata);

    /**
     * get the polar boatspeed for a given tws and twa
     * bsp and tws are in 1/10ths of a kn.
     */
    uint16_t getBoatSpeed(uint16_t ctws, uint16_t ctwa);
    /**
     * Get the polar performance for a given tws, twa and real bsp. 
     * bsp and tws are in 1/10ths of a kn.
     * result is a % 0. > 100% is possible.
     */
    uint16_t getBoatSpeedPerformance(uint16_t ctws, uint16_t ctwa, uint16_t cbsp);

    uint8_t checkPolarData(void);
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

  int findGreater(uint16_t val, const uint16_t *values, int nvalues);
  int lowerRange(int val, int nvalues);
  int upperRange(int val, int nvalues);
  float interpolateBsp(float val, float vall, float valh, float bspl, float bsph);
  float getRealBoatSpeed(uint16_t ctws, uint16_t ctwa);




};







#endif


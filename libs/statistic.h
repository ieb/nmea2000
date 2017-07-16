
#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include <stdint.h>
#include <math.h>


class SimpleStatistic {
public:
    SimpleStatistic() {

    }
    void update(float v, unsigned long tnow) {
        value = v; at = tnow;
    }
    float means(int nseconds, unsigned long tnow) {
        return value;
    }
    float meanm(int nminutes, unsigned long tnow) {
        return value;
    }
    float stdevs(int nseconds, unsigned long tnow) {
        return 0.0F;
    }
    float stdevm(int nseconds, unsigned long tnow) {
        return 0.0F;
    }

private:
    double value;
    unsigned long at;

};

class Statistic {
    public:
        /** Constructor for Statistics
         * statsType, the type of statistic being recorded. This controls how the 
         * stdev and mean is calculated and what units the interface talks in.
         */
        Statistic() {
            for(int i = 0; i < 60; i++) {
                seconds[i] = 0;
                minutes[i] = 0;        
            }
            islast = 0;
            imlast = 0;
        }

        /**
         * Update the statistic at a known time supplied in tnow.
         * v the value of the update.
         * tnow time in ms.
         */
        void update(float v, unsigned long tnow) {
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

        /**
         * get the mean for the past nseconds where nseconds is in the range 1 - 60.
         */
        float means(int nseconds, unsigned long tnow) {
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
        /**
         * get the mean for the past nminutes where nminites is in the range 1 - 60.
         */
        float meanm(int nminutes, unsigned long tnow) {
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
        /**
         * get the standard deviation for the past nseconds where nseconds is in the range 1 - 60.
         */
        float stdevs(int nseconds, unsigned long tnow) {
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

        /**
         * get the standard deviation for the past nminutes where nminites is in the range 1 - 60.
         */
        float stdevm(int nminutes,  unsigned long tnow){
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
        void fill(int s, int e, int size, float *values, float value) {
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


        virtual float mean(float *values, int ntotal, int s, int e) {
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


        virtual float stdev(float *values, int ntotal, int s, int e) {
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
        virtual float toOutput(float v) { return v;};
        virtual float fromInput(float v) { return v;};
};


class RadianStatistic : public Statistic {
    public:
        /** Constructor for Statistics
         * statsType, the type of statistic being recorded. This controls how the 
         * stdev and mean is calculated and what units the interface talks in.
         */
        RadianStatistic() {
        }

    private:
        virtual float mean(float *values, int ntotal, int s, int e) {
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
        virtual float stdev(float *values, int ntotal, int s, int e) {
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

};

class DegreesStatistic :  public RadianStatistic {
    public:
        /** Constructor for Statistics
         * statsType, the type of statistic being recorded. This controls how the 
         * stdev and mean is calculated and what units the interface talks in.
         */
        DegreesStatistic() {
        }


    private:
        virtual float toOutput(float v) {
            return v*(180.0F/M_PI);
        }
        virtual float fromInput(float v) {
            return v*(M_PI/180.0F);
        }

};


class Statistics {
public:
    Statistics() {
        awa = RadianStatistic();
        aws = Statistic();
        twa = RadianStatistic();
        tws = Statistic();
        stw = Statistic();
        pitch  = RadianStatistic();
        roll = RadianStatistic();
        leeway  = RadianStatistic();
    }
    RadianStatistic awa;
    Statistic aws;
    RadianStatistic twa;
    Statistic tws;
    Statistic stw;
    RadianStatistic leeway;
    RadianStatistic pitch;
    RadianStatistic roll;
};


#endif


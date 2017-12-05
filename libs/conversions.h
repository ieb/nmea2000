#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

#define PI (double)3.1415926535897932384626433832795

/*
 * Hz per Kn == *3600 to convert to Hz per Nautical Mile per second.
 * 
 */
inline double hzPerKnToHzPerMPerS(double v) { return v*3600/1852.0; }
inline double hzPerMPerSToHzPerKn(double v) { return v*1852.0/3600; }
inline double knotsToMs(double v) { return v*1852.0/3600.0; }
inline double fixAngle(double d) {
        if ( d > PI ) d = d - PI;
        if ( d < -PI) d = d + PI;
        return d;
    }


#endif
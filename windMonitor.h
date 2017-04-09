
#ifndef __WINDMONITOR_H__
#define __WINDMONITOR_H__

#include <N2kMessages.h>


class WindMonitor {
public:
    WindMonitor(tN2kWindReference windType=N2kWind_Apprent) {
      speed = N2kDoubleNA; // in m/s to convert use msToKnots KnotsToms.
      angle = N2kDoubleNA; // in radians to convert use DegToRad, RadToDeg
      type = windType;
      windSID = 0;
    }
    /**
     * Reads from the internal Stats
     */
    void read() {
#ifdef DEMOMODE
      // for the moment emit demo numbers.
      if ( speed == N2kDoubleNA) {
          speed = 1; // m/s
          angle = M_PI; // radians
      } else {
        speed = max(0.0F, speed+0.01*((rand()%100)-50));
        angle = fmod(M_PI, angle+0.001*((rand()%100)-50));
      }
#else
      /*
       * TODO.
       * read the angular momentum from gyro.
       * read attitude.
       * Read the sin and cosine voltages
       * use atan2 to get an angle

       * adjust for tip vectors on masthead
       *    gyro gives deg/s in horizontal plains.
       *    motion is 2*pi*ml*(ds/360)
       *      ml = mast length,
       *      ds = degrees per second
       *    combined velocity is swrt(vx*vx+vy*vy)
       *    direction of velocity is atan2(vx,vy)
       * vector sum this with measured awa/aws to get real awa/aws.

       * adjust for wind sheer to get 10m height
       * adjust for user calibration.

       * Tilt adjustment is best defined by an adjustment vector vs tilt angle.
       * Tilt angle should be a combination of bth horizontal planes. see http://www.dewi.de/dewi/fileadmin/pdf/publications/Publikations/S09_2.pdf
       30 +8.5%
       28 +8.5%
       26 +8
       24 +7.5
       22 +7
       20 +6
       18 +6
       16 6
       14 6
       12 5
       10 4
       8 3.5
       6 3
       2 2
       0 0
       * adjust to get true aparent wind.
       * what about leyway ?
       */
#endif

      windSID = (windSID+1)%256;
    }

    void fullStatusMessage(tN2kMsg &N2kMsg) {
      SetN2kWindSpeed(N2kMsg, 
          windSID,
          speed,
          angle,
          type
          );
    }
  private:
    double speed; // in m/s to convert use msToKnots KnotsToms.
    double angle; // in radians to convert use DegToRad, RadToDeg
    tN2kWindReference type;


};



#endif
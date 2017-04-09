
#ifndef __BOATSPEEDMONITOR_H__
#define __BOATSPEEDMONITOR_H__

#include <N2kMessages.h>


typedef struct boatSpeedStatus_s {
  double waterSpeed; // in m/s to convert use msToKnots KnotsToms.
  double groundSpeed; // in m/s to convert use msToKnots KnotsToms.
  tN2kSpeedWaterReferenceType SWRT;
} boatSpeedStatus_t;


class BoatSpeedMonitor {
public:
    BoatSpeedMonitor() {
    }
    /**
     * Reads from the internal Stats
     */
    void read(boatSpeedStatus_t *boatSpeed) {
      // for the moment emit demo numbers.
      if ( boatSpeed->waterSpeed == N2kDoubleNA) {
          boatSpeed->waterSpeed = 1; // m/s
      } else {
        boatSpeed->waterSpeed = max(0.0F,boatSpeed->waterSpeed+0.01*((rand()%100)-50));
      }
    }

    void makeBoatSpeed(boatSpeedStatus_t *boatSpeed, tN2kSpeedWaterReferenceType SWRT=N2kSWRT_Paddle_wheel) {
        boatSpeed->waterSpeed = N2kDoubleNA; // m/s relative to water.
        boatSpeed->groundSpeed = N2kDoubleNA; // m/s relative to ground
        boatSpeed->SWRT = SWRT; // reference type
    }

    void fullStatusMessage(tN2kMsg &N2kMsg, unsigned char speedSID, boatSpeedStatus_t *boatSpeed) {
      SetN2kBoatSpeed(N2kMsg, 
          speedSID,
          boatSpeed->waterSpeed,
          boatSpeed->groundSpeed,
          boatSpeed->SWRT
          );
    }

};



#endif

#ifndef __WATERMONITOR_H__
#define __WATERMONITOR_H__

#include "monitors.h"

#ifndef TEST
#include <N2kMessages.h>
#endif


#define WaterMonitor_MESSAGES  130312L


class WaterMonitor {
public:



    WaterMonitor(uint8_t _waterTempADC) {
      _waterTempADC = _waterTempADC;
      sid = 0;
      monitorMode = MONITOR_MODE_ENABLED;
    }

    void setMode(tMontorMode mode) {
      monitorMode = mode;
    }

    bool isEnabled() {
        return (monitorMode == MONITOR_MODE_ENABLED) || (monitorMode == MONITOR_MODE_DEMO);
    }

    /**
     * Reads from the internal Stats
     * The temperature sensor output is not known other ant 1.8v = 0C
     * Needs calibration data.
     */

#define CONVERT_TO_TEMPERATURE(x) (((double)x*0.00592749734822F)-1.8F)

    void read() {
      if (monitorMode == MONITOR_MODE_DEMO) {
        waterTemperature = max(0.0F, waterTemperature+0.01*((rand()%100)-50));
      } else if (monitorMode == MONITOR_MODE_ENABLED ){
        waterTemperature = CONVERT_TO_TEMPERATURE(analogRead(_waterTempADC));
      }
      sid++;
      LOG(F("Water Temperature "));
      LOGC(waterTemperature);
      LOGN(F(" C"));
    }

    bool fillWaterTemperature(tN2kMsg &N2kMsg) {
        if (waterTemperature != N2kDoubleNA) {
            SetN2kTemperature(N2kMsg, sid, 1, N2kts_SeaTemperature,
                                 CToKelvin(waterTemperature));
            return true;
        }
        return false;
    }



  private:
    double waterTemperature; // in C
    uint8_t _waterTempADC;
    uint8_t sid;
    tMontorMode monitorMode;




};



#endif

#ifndef __WATERMONITOR_H__
#define __WATERMONITOR_H__

#include "configuration.h"

#ifndef TEST
#include <N2kMessages.h>
#endif


#define WaterMonitor_MESSAGES  130312L


class WaterMonitor  {
public:



    WaterMonitor(uint8_t _waterTempADC, demo_data_t *demoData) {
      _waterTempADC = _waterTempADC;
      sid = 0;
      sensorsConnected = false;
      demoMode = false;
      this->demoData = demoData;
    }

    bool init() {
      return true;
    }

    /**
     * Reads from the internal Stats
     * The temperature sensor output is not known other ant 1.8v = 0C
     * Needs calibration data.
     */

#define CONVERT_TO_TEMPERATURE(x) (((double)x*0.00592749734822F)-1.8F)

    bool read() {
      if (!sensorsConnected) {
        return false;
      }
      if (demoMode) {
        waterTemperature = demoData->waterTemperature;
      } else {
        waterTemperature = CONVERT_TO_TEMPERATURE(analogRead(_waterTempADC));
      }
      sid++;
      LOG(F("Water Temperature "));
      LOGC(waterTemperature);
      LOGN(F(" C"));
      return true;
    }

    bool fillWaterTemperature(tN2kMsg &N2kMsg) {
        if (waterTemperature != N2kDoubleNA) {
            SetN2kTemperature(N2kMsg, sid, 1, N2kts_SeaTemperature,
                                 CToKelvin(waterTemperature));
            return true;
        }
        return false;
    }




    void updateConfiguration(Configuration *config) {
      sensorsConnected = config->getSensorsEnabled();
      demoMode = config->getDemoMode();
    }


    void dumpRunstate() {
      DUMP(F("Water Temperature "));
      DUMPC(waterTemperature);
      DUMPN(F(" C"));

    }

  private:
    double waterTemperature; // in C
    uint8_t _waterTempADC;
    uint8_t sid;
    bool sensorsConnected;
    bool demoMode;
    demo_data_t *demoData;




};



#endif
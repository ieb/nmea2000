
#ifndef __ENVIROMONITOR_H__
#define __ENVIROMONITOR_H__

#include "configuration.h"


#ifndef TEST
#include <N2kMessages.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#endif

#ifndef NULL
#define NULL 0
#endif



#define EnviroMonitor_MESSAGES 130310L

// Statically declare to avoid memory issues.
Adafruit_BMP085_Unified enviroSensor = Adafruit_BMP085_Unified(18001);
bool enviroSesorEnabled = false;

class EnviroMonitor {
public:
    

    EnviroMonitor() {
      _envSID = 0;
      enviroSesorEnabled = false;
      enviroEnabled = false; 
      demoMode = false;
    }
    void begin() {
        //Serial.print("Checking Enviro sensor ");
        if (!enviroSesorEnabled) {
                LOG(F("Enabling Enviro Sensor"));
            if(enviroSensor.begin()) {
                LOG(F("Enabled Enviro Sensor"));
                enviroSesorEnabled = true;
            }
        } 
        if (!enviroSesorEnabled) {
            LOG(F("Not Enabled Enviro Sensor"));
        }
    }

    void updateConfiguration(Configuration configuration) {
        enviroEnabled = configuration.getFlag(CONFIG_FLAGS_ENNVIRO_ENABLED);
        demoMode = configuration.getFlag(CONFIG_FLAGS_DEMO_ENABLED);
        if (demoMode) {
          temperature = 22.2;
          pressure = 997;          
        }
    }


    bool read() {
      if (!enviroEnabled) {
        return false;
      }
      if (demoMode) {
        LOGN(F("Enviro Read Demo"));
        temperature = min(max(0.0F, temperature+0.01F*((rand()%100)-50)),45.0F);
        pressure = min(max(999.0F, pressure+0.01F*((rand()%100)-50)),1030.0F);
        _envSID++;
        return true;
      } else {
        if ( enviroSesorEnabled ) {
          sensors_event_t event;
          enviroSensor.getEvent(&event);
          enviroSensor.getTemperature(&temperature);
          pressure = event.pressure;
          LOG(F("Enviro Read hardware"));
          LOGC(F("Temperature "));
          LOGC(temperature );
          LOGC(F(" C, Pressure "));
          LOGC(mBarToPascal(pressure));
          LOGC(F(" P, "));
          LOGC(pressure);
          LOGN(" mBar ");
          _envSID++;
          return true;
        }
      }
      LOG(F("Enviro Read failed" ));
      return false;
    }
    void fillStatusMessage(tN2kMsg &N2kMsg) {
      if ( enviroSesorEnabled ) {
          SetN2kOutsideEnvironmentalParameters(N2kMsg,
            _envSID, 
            N2kDoubleNA, 
            CToKelvin(temperature), 
            mBarToPascal(pressure));
      }
    }
private:
    unsigned char _envSID;
    float temperature;
    float pressure;
    bool demoMode;
    bool enviroEnabled;

};



#endif

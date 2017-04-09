
#ifndef __ENVIROMONITOR_H__
#define __ENVIROMONITOR_H__

#include <N2kMessages.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

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
    }
    void begin() {
        if (!enviroSesorEnabled) {
            if(enviroSensor.begin()) {
                enviroSesorEnabled = true;
            }
        }        
    }
    bool read() {
      if ( enviroSesorEnabled ) {
        sensors_event_t event;
        enviroSensor.getEvent(&event);
        enviroSensor.getTemperature(&temperature);
        pressure = event.pressure;
        _envSID = (_envSID+1)%256;
        return true;
      }
      return false;
    }
    void fillStatusMessage(tN2kMsg &N2kMsg) {
      if ( enviroSesorEnabled ) {
          SetN2kPGN130310(N2kMsg,
            _envSID, 
            N2kDoubleNA, 
            CToKelvin(temperature), 
            mBarToPascal(pressure));
      }
    }
private:
    unsigned char _envSID = 0;
    float temperature;
    float pressure;
};



#endif
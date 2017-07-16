#ifndef __COMMANDREADER_H__
#define __COMMANDREADER_H__

#ifndef TEST
#include <Arduino.h>
#include <DueFlashStorage.h>
#endif

#define Escape 0x10
#define CarrageReturn 0x0A
#define NewLine 0x0D
#define Space 0x20
#define Del 0x7F

#define FACTORY_DEFAULT       0b0000000
#define LOGGING_ON            0b0000001
#define SENSORS_ON        0b0000010
#define STATUS_ON        0b0000100
#define LOGGING_OFF_MASK      0b1111110
#define SENSORS_OFF_MASK      0b1111101
#define STATUS_OFF_MASK      0b1111011

#define CONFIG_FLAGS_SENSORS_ENABLED 0b0000010


#define MAX_COMMAND_LENGTH 80

enum tConfigEntries {
                            CONFIG_WATER_SPEED_CAL=0,
                            CONFIG_WIND_SPEED_CAL=1,
                            CONFIG_WIND_OFFSET_CAL=2,
                            CONFIG_MAST_HEIGHT=3,
                            CONFIG_KFACTOR=4,
                            CONFIG_WIND_SPEED_HZ=5,
                            CONFIG_WATER_SPEED_HZ=6,
                            CONFIG_LENGTH=7
                          };



struct tConfigObj {
    uint8_t flags;
    uint16_t configLength;
    float config[CONFIG_LENGTH];
};



class Configuration {
public:

    Configuration(Stream *stream) {
        this->stream = stream;
        cbp = 0;
    }
    void init() {
        if (!loadFromFlash()) {
            factoryReset();
        }
        waitTime = 0;
    }

    bool read() {
        bool result = false;
        while (stream->available() > 0 && (lastChar = stream->read()) != -1 && !result) {
            switch(lastChar) {
                case Escape:
                    configData.flags = configData.flags & LOGGING_OFF_MASK;
                    break;
                case CarrageReturn:
                    commandBuffer[cbp++] = 0;
                    cbp = 0;
                    result = parseCommand();
                    stream->print(F("$>"));
                    break;
                default:
                    if (lastChar >= Space && lastChar < Del && cbp < MAX_COMMAND_LENGTH) {
                        commandBuffer[cbp++] = lastChar;
                    }
                    break;
            }
        }
        return result;
    }

    void countWait(unsigned long freetime) {
        waitTime += freetime;
    } 

    double getConfig(tConfigEntries key) {
        return (double) configData.config[key];
    }

    bool getFlag(uint8_t mask) {
        return ((configData.flags & mask) == mask);
    }

    bool isLoggingOn() {
        return ((configData.flags & LOGGING_ON) == LOGGING_ON);
    }
    bool isStatusOn() {
        return ((configData.flags & STATUS_ON) == STATUS_ON);
    }

    bool areSensorsEnabled() {
        return ((configData.flags & SENSORS_ON) == SENSORS_ON);
    }


private:
    Stream *stream;
    char lastChar;
    uint8_t cbp;
    char commandBuffer[MAX_COMMAND_LENGTH+1];
    tConfigObj configData;
    DueFlashStorage dueFlashStorage;
    unsigned long waitTime;


    bool parseCommand() {
        stream->print(F("Got ["));
        stream->print(commandBuffer);
        stream->println(F("]"));
            
            
      if ( strncmp(commandBuffer,"log on", 6) == 0 ) {
        configData.flags = configData.flags | LOGGING_ON;
        return false;
      } else if ( strncmp(commandBuffer,"log off", 7) == 0 ) {
        configData.flags = configData.flags & LOGGING_OFF_MASK;
        return false;
      } else if (  strncmp(commandBuffer,"sensors on", 10) == 0 ) {
        configData.flags = configData.flags | SENSORS_ON;
      } else if (  strncmp(commandBuffer,"sensors off", 11) == 0 ) {
        configData.flags = configData.flags & SENSORS_OFF_MASK;
      } else if (  strncmp(commandBuffer,"status on", 9) == 0 ) {
        configData.flags = configData.flags | STATUS_ON;
      } else if (  strncmp(commandBuffer,"status off", 10) == 0 ) {
        configData.flags = configData.flags & STATUS_OFF_MASK;
      } else if (  strncmp(commandBuffer,"waterspeedcal ", 14) == 0 ) {
        setConfig(CONFIG_WATER_SPEED_CAL, atof(&commandBuffer[14]));
      } else if ( strncmp(commandBuffer,"windspeedcal ", 13) == 0 ) {
        setConfig(CONFIG_WIND_SPEED_CAL, atof(&commandBuffer[13]));
      } else if (  strncmp(commandBuffer,"windangleoffset ", 16) == 0 ) {
        setConfig(CONFIG_WIND_OFFSET_CAL, atof(&commandBuffer[16]), true);
      } else if (  strncmp(commandBuffer,"mastheight ", 11) == 0 ) {
        setConfig(CONFIG_MAST_HEIGHT, atof(&commandBuffer[11]));
      } else if (  strncmp(commandBuffer,"kfactor ", 8) == 0 ) {
        setConfig(CONFIG_KFACTOR, atof(&commandBuffer[8]));
      } else if (  strncmp(commandBuffer,"windspeedHz ", 12) == 0 ) {
        setConfig(CONFIG_WIND_SPEED_HZ, atof(&commandBuffer[12]));
      } else if (  strncmp(commandBuffer,"waterspeedHz ", 13) == 0 ) {
        setConfig(CONFIG_WATER_SPEED_HZ, atof(&commandBuffer[13]));
      } else if (  strncmp(commandBuffer,"save", 4) == 0 ) {
        saveToFlash();
      } else if (  strncmp(commandBuffer,"load", 4) == 0 ) {
        loadFromFlash();
      } else if (  strncmp(commandBuffer,"reset", 5) == 0 ) {
        factoryReset();
      } else if (  strncmp(commandBuffer,"status", 6) == 0 ) {
        stream->println(F("Status "));
        if (isLoggingOn()) {
            stream->println(F("logging on "));
        } else {
            stream->println(F("logging off "));
        }
        if (areSensorsEnabled()) {
            stream->println(F("reading sensors direct"));
        } else {
            stream->println(F("reading bus for sensors"));
        }

        stream->print(F("Water speed user calibration factor:"));stream->println(configData.config[CONFIG_WATER_SPEED_CAL]);
        stream->print(F("Wind speed user calibration factor:"));stream->println(configData.config[CONFIG_WIND_SPEED_CAL]);
        stream->print(F("Wind angle offset:"));stream->println(configData.config[CONFIG_WIND_OFFSET_CAL]);
        stream->print(F("Mast height:"));stream->println(configData.config[CONFIG_MAST_HEIGHT]);
        stream->print(F("Leyway K Factor:"));stream->println(configData.config[CONFIG_KFACTOR]);
        stream->print(F("Water Speed pulse Hz per Kn:"));stream->println(configData.config[CONFIG_WATER_SPEED_HZ]);
        stream->print(F("Wind Speed pulse Hz per Kn:"));stream->println(configData.config[CONFIG_WIND_SPEED_HZ]);
        // dump status.
        return false;
      } else if (  (strncmp(commandBuffer,"?", 1) == 0 ) || (strncmp(commandBuffer,"help", 4) == 0)) {
        stream->println(F("Commands "));
        stream->println(F("log on, turn logging on"));
        stream->println(F("log off, turn logging off, alternatively press Esc"));
        stream->println(F("sensors on|off, turn sensor moniting on or off"));
        stream->println(F("waterspeedcal <float>, water speed calibration factor (default: 1.0)"));
        stream->println(F("windspeedcal <float>, wind speed calibration factor (default: 1.0) "));
        stream->println(F("windangleoffset <float>, wind angle offset in degrees (default: 0.0) "));
        stream->println(F("mastheight <float>, height of sensor above center of rotation on mast in m "));
        stream->println(F("kfactor <float>, leeway calculation factor, defailt 11.0 "));
        stream->println(F("windspeedHz <float>, frequency of pulses per kn from the wind speed sensor"));
        stream->println(F("waterspeedHz <float>, frequency of pulses per kn from the water speed sensor"));
        stream->println(F("status, dump the current settings"));
        stream->println(F("status on|off, log status messages"));
        stream->println(F("reset, perform a factory reset"));
        stream->println(F("save, save config to flash"));
        stream->println(F("load, load config from flash"));
        stream->println(F("?|help, this message."));
        return false;
      } else {
        return false;
      }
      return true;
    }

    void setConfig(tConfigEntries key, float v, bool zeroOk=false) {
        if ( zeroOk || v > 0.0F ) {
            configData.config[key] = v;
        }
    }


    void factoryReset() {
        uint16_t reset = 0;
        writeFlash(0,reset);
        writeFlash(sizeof(uint16_t),reset);
        configData.flags = FACTORY_DEFAULT;
        configData.config[CONFIG_WATER_SPEED_CAL] = 1.0F;
        configData.config[CONFIG_WIND_SPEED_CAL] = 1.0F;
        configData.config[CONFIG_WIND_OFFSET_CAL] = 0.0F;
        configData.config[CONFIG_MAST_HEIGHT] = 17.5F;
        configData.config[CONFIG_KFACTOR] = 12.0F;
        configData.config[CONFIG_WIND_SPEED_HZ] = 1.045F,
        configData.config[CONFIG_WATER_SPEED_HZ] = 5.5F;

    }




    /**
     * Flash structure is 
     * uint16_t size of Flash 2xuint16_t+sizeof(ConfigObj)
     * ConfigObj
     * uint16_t checksum of 0 to end of ConfigObj.
     */
    bool saveToFlash() {
        uint16_t configSize = sizeof(tConfigObj)+2*sizeof(uint16_t);
        if ( configSize > 512 ) {
            stream->println(F("Failed to save, no space left "));
            return false;
        }
        writeFlash(0, configSize);
        writeFlash(sizeof(uint16_t), configData);
        uint16_t checksum = crc16EEprom(0, sizeof(uint16_t)+sizeof(tConfigObj));
        writeFlash(sizeof(uint16_t)+sizeof(tConfigObj),checksum);
        stream->println(F("Saved"));
        return true;
    }
    bool loadFromFlash() {
        uint16_t configSize = 0;
        readFlash(0, configSize);
        if ( configSize != (sizeof(tConfigObj)+2*sizeof(uint16_t))) {
            stream->println(F("No config in flash"));
            return false;
        }
        uint16_t checksum = 0;
        readFlash(sizeof(uint16_t)+sizeof(tConfigObj), checksum);
        if ( checksum != crc16EEprom(0, sizeof(uint16_t)+sizeof(tConfigObj))) {
            stream->println(F("Checksum failed"));
            return false;
        }
        readFlash(sizeof(uint16_t), configData);
        stream->println(F("Loaded config"));
        return true;
    }


    inline uint16_t crc16(char *a, uint16_t startb, uint16_t endb) {
      uint16_t crc = 0;
      for (uint16_t i = startb; i < endb; i++) {
        crc = crc16_update(crc, a[i]);;
      }
      return crc;
   }

   inline uint16_t crc16EEprom( uint16_t startb, uint16_t endb ) {
      uint16_t crc = 0;
      for (uint16_t i = startb; i < endb; i++) {
        crc = crc16_update(crc, dueFlashStorage.read(i));
      }
      return crc;
   }

   inline uint16_t crc16_update(uint16_t crc, uint8_t a) {
      int i;     
      crc ^= a;
      for (i = 0; i < 8; ++i) {
           if (crc & 1)
             crc = (crc >> 1) ^ 0xA001;
           else
             crc = (crc >> 1);
      }   
      return crc;
   }

   template<typename T> T &readFlash(uint16_t idx, T &t) {
    uint8_t *ptr = (uint8_t *) &t;
    for ( int count = sizeof(T); count; count-- ) {
        *ptr++ = dueFlashStorage.read(idx++);
    }
    return t;
   }

   template<typename T> const T &writeFlash(uint16_t idx, const T &t) {
        const uint8_t *ptr = (const uint8_t *) &t;
        for ( int count = sizeof(T); count; count-- ) {
            dueFlashStorage.write(idx++,*ptr++);
        }
        return t;
    }
};

#endif

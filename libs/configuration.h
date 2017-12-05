#ifndef __COMMANDREADER_H__
#define __COMMANDREADER_H__

#ifndef TEST
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <DueFlashStorage.h>
#include <N2kMessages.h>
#include "freemem.h"
#else
#define PROGMEM
#endif

#include "demo.h"

#define Escape 0x10
#define CarrageReturn 0x0A
#define NewLine 0x0D
#define Space 0x20
#define Del 0x7F



#define MAX_CALIBRATIONS 10
#define MAX_LINE_LENGTH 80
#define MAX_FIELDS 5

#define FACTORY_DEFAULT       0b00010110


#define CONFIG_RESERVED1                 0b0000001
#define CONFIG_FLAGS_SENSORS_ENABLED     0b0000010
#define CONFIG_FLAGS_DEMO_ENABLED        0b0000100
#define CONFIG_FLAGS_BATTERIES_CONNECTED 0b0001000
#define CONFIG_FLAGS_PERFORMANCE_ENABLED 0b0010000
#define CONFIG_FLAGS_ENNVIRO_ENABLED     0b0100000
#define CONFIG_FLAGS_IMU_CALIBRATED      0b1000000


const char CFG_DEMO_ENABLE[]  PROGMEM = {"demo.enable"};
const char CFG_DEMO_DISABLE[]  PROGMEM = {"demo.disable"}; 
const char CFG_WIND_ANGLE_DAMPING[]  PROGMEM = {"wind.angle.damping"}; 
const char CFG_WIND_SIN_RANGE[]  PROGMEM = {"wind.sin.range"}; 
const char CFG_WIND_COS_RANGE[]  PROGMEM = {"wind.cos.range"}; 
const char CFG_WIND_ANGLE_OFFSET[]  PROGMEM = {"wind.angle.offset"}; 
const char CFG_WIND_SPEED_DAMPING[]  PROGMEM = {"wind.speed.damping"}; 
const char CFG_WIND_SPEED_CALLIBRATION[]  PROGMEM = {"wind.speed.calibration"}; 
const char CFG_WATER_SPEED_DAMING[]  PROGMEM = {"water.speed.damping"}; 
const char CFG_WATER_SPEED_CALIBRATION[]  PROGMEM = {"water.speed.calibration"}; 
const char CFG_BOAT_MASTHEIGHT[]  PROGMEM = {"boat.mastheight"}; 
const char CFG_BOAT_KFACTOR[]  PROGMEM = {"boat.kfactor"}; 
const char CFG_LOG_ENABLE[]  PROGMEM = {"log.enable"}; 
const char CFG_LOG_DISABLE[]  PROGMEM = {"log.disable"}; 
const char CFG_STATUS_ENABLE[]  PROGMEM = {"status.enable"}; 
const char CFG_STATUS_DISABLE[]  PROGMEM = {"status.disable"}; 
const char CFG_SENSOR_ENABLE[]  PROGMEM = {"sensor.enable"}; 
const char CFG_SENSOR_DISABLE[]  PROGMEM = {"sensor.disable"}; 
const char CFG_PERF_ENABLE[]  PROGMEM = {"perf.enable"}; 
const char CFG_PERF_DISABLE[]  PROGMEM = {"perf.disable"}; 
const char CFG_WIND_SPEED_CALIBRATION_RESET[] PROGMEM = {"wind.reset"};
const char CFG_WATER_SPEED_CALIBRATION_RESET[] PROGMEM = {"water.reset"};
const char DEMO_WIND_ANGLE[] PROGMEM = {"demo.wind.angle"};
const char DEMO_WIND_SPEED[] PROGMEM = {"demo.wind.speed"};
const char DEMO_WATER_SPEED[] PROGMEM = {"demo.water.speed"};
const char DEMO_WATER_TEMP[] PROGMEM = {"demo.water.temp"};
const char DEMO_PITCH[] PROGMEM = {"demo.pitch"};
const char DEMO_ROLL[] PROGMEM = {"demo.roll"};
const char DEMO_GYRO_PITCH[] PROGMEM = {"demo.gyro.pitch"};
const char DEMO_GYRO_ROLL[] PROGMEM = {"demo.gyro.roll"};
const char CFG_POLAR[] PROGMEM = {"polar"};
const char CFG_IMU_ACCEL_OFFSET[] PROGMEM = {"imu.accel.offset"};
const char CFG_IMU_GYRO_OFFSET[] PROGMEM = {"imu.gyro.offset"};
const char CFG_IMU_MAG_OFFSET[] PROGMEM = {"imu.mag.offset"};
const char CFG_IMU_RADIUS[] PROGMEM = {"imu.radius"};
const char CFG_IMU_CAL_ENABLE[] PROGMEM = {"imu.cal.enable"};
const char CFG_IMU_CAL_DISABLE[] PROGMEM = {"imu.cal.disable"};


const char TESTPATTERN[] PROGMEM = {
  "log.disable\n" 
  "status.disable\n" 
  "demo.enable\n" 
  "demo.wind.angle,32.1\n" 
  "demo.wind.speed,12.4\n" 
  "demo.water.speed,7.1\n" 
  "demo.water.temp,16.1\n" 
  "demo.pitch,5.1\n" 
  "demo.roll,12.1\n" 
  "demo.gyro.pitch,0.1\n" 
  "demo.gyro.roll,0.2\n" 
  "wind.sin.range,0,4001\n" 
  "wind.cos.range,0,4002\n" 
  "wind.angle.offset,0\n" 
  "wind.angle.damping,1\n" 
  "wind.speed.damping,2\n" 
  "water.speed.damping,3\n" 
  "water.reset\n" 
  "water.speed.calibration,0,5.5\n" 
  "water.speed.calibration,10,5.4\n" 
  "water.speed.calibration,20,5.3\n" 
  "water.speed.calibration,30,5.1\n" 
  "wind.reset\n" 
  "wind.speed.calibration,0,1.5\n" 
  "wind.speed.calibration,10,1.4\n" 
  "wind.speed.calibration,20,1.3\n" 
  "wind.speed.calibration,30,1.2\n" 
  "boat.mastheight,17.5\n" 
  "boat.kfactor,12.3\n" 
  "imu.accel.offset,65532,0,35\n"
  "imu.gyro.offset,65535,65534,0\n"
  "imu.mag.offset,338,65211,126\n"
  "imu.radius,1000,778\n"
  "imu.cal.enable\n"
  "status\n"
};



#define CONFIG_NO_ACTION 0
#define CONFIG_UPDATE_RUNTIME 1
#define CONFIG_CURRENT_STATUS 2
#define CONFIG_IMU_CALIBRATION 3



DueFlashStorage dueFlashStorage;


struct tConfigObj {
    uint8_t flags;
    float mastheight;
    float kfactor;
    uint8_t windSpeedDamping;
    uint8_t windAngleDamping;
    uint8_t waterSpeedDamping;
    int windAngleMin[2];
    int windAngleMax[2];
    float windAngleOffset;
    uint8_t nwindSpeedCalibrations;
    uint8_t nwaterSpeedCalibrations;
    float windSpeedFrequencies[MAX_CALIBRATIONS];
    float windSpeedPulsesPerM[MAX_CALIBRATIONS];
    float waterSpeedFrequencies[MAX_CALIBRATIONS];
    float waterSpeedPulsesPerM[MAX_CALIBRATIONS];
    uint16_t accel_offset_x;
    uint16_t accel_offset_y;
    uint16_t accel_offset_z;
    uint16_t gyro_offset_x;
    uint16_t gyro_offset_y;
    uint16_t gyro_offset_z;
    uint16_t mag_offset_x;
    uint16_t mag_offset_y;
    uint16_t mag_offset_z;
    uint16_t accel_radius;
    uint16_t mag_radius;

    char polarFile[12];

};




class Configuration {
public:
  Configuration(Stream *controlStream, demo_data_t *demoData) {
    this->controlStream = controlStream;
    this->demoData = demoData;
    cbp = 0;
    logging = true;
    status = false;
    polarStreamOpen = false;
  }


  void init() {
    if (!loadFromFlash()) {
        factoryReset();
    }
  }


  uint8_t read() {
    uint8_t result = CONFIG_NO_ACTION;
    char lastChar;
    while (controlStream->available() > 0 && (lastChar = controlStream->read()) != -1 && !result) {
      switch(lastChar) {
        case Escape:
          logging = false;
          break;
        case CarrageReturn:
          commandBuffer[cbp++] = 0;
          cbp = 0;
          result = parseCommand();
          controlStream->print(F("$>"));
          break;
        default:
          if (lastChar >= Space && lastChar < Del && cbp < MAX_LINE_LENGTH) {
              commandBuffer[cbp++] = lastChar;
          }
          break;
      }
    }
    return result;
  }

  File *openPolar() {
    if ( strlen(config.polarFile) > 0 ) {
      if (!SD.begin(4)) {
        INFOLN(F("Initialization from SD Card failed, reverting to factory settings."));
        return 0;
      }
      closePolar();
      polarStream = SD.open(config.polarFile);
      polarStreamOpen = true;
      return &polarStream;
    }
    return 0;
  }

  void closePolar() {
    if (polarStreamOpen) {
      polarStream.close();
      polarStreamOpen = false;
    }
  }


  bool isStatusOn() {
    return status;
  }
  bool isLoggingOn() {
    return logging;
  }

  void enableLogging() {
    logging = true;
  }
  void disableLogging() {
    logging = false;
  }

  bool getDemoMode() {
    return ((config.flags&CONFIG_FLAGS_DEMO_ENABLED) == CONFIG_FLAGS_DEMO_ENABLED);
  }
  bool getPerformanceEnabled() {
    return ((config.flags&CONFIG_FLAGS_PERFORMANCE_ENABLED) == CONFIG_FLAGS_PERFORMANCE_ENABLED);
  }
  bool getSensorsEnabled() {
    return ((config.flags&CONFIG_FLAGS_SENSORS_ENABLED) == CONFIG_FLAGS_SENSORS_ENABLED);
  }
  bool getImuCalEnabled() {
    return ((config.flags&CONFIG_FLAGS_IMU_CALIBRATED) == CONFIG_FLAGS_IMU_CALIBRATED);
  }
  void setImuCalEnabled(bool isset) {
    if ( isset ) {
      config.flags = config.flags | CONFIG_FLAGS_IMU_CALIBRATED;

    } else {
      config.flags = config.flags & ~CONFIG_FLAGS_IMU_CALIBRATED;

    }
  }

  void getIMUAccelOffset(uint16_t *accel_offset_x, uint16_t *accel_offset_y, uint16_t *accel_offset_z) {
    accel_offset_x[0] = config.accel_offset_x;
    accel_offset_y[0] = config.accel_offset_y;
    accel_offset_z[0] = config.accel_offset_z;
  }
  void getIMUGyroOffset(uint16_t *gyro_offset_x, uint16_t *gyro_offset_y, uint16_t *gyro_offset_z) {
    gyro_offset_x[0] = config.gyro_offset_x;
    gyro_offset_y[0] = config.gyro_offset_y;
    gyro_offset_z[0] = config.gyro_offset_z;
  }
  void getIMUMagOffset(uint16_t *mag_offset_x, uint16_t *mag_offset_y, uint16_t *mag_offset_z) {
    mag_offset_x[0] = config.mag_offset_x;
    mag_offset_y[0] = config.mag_offset_y;
    mag_offset_z[0] = config.mag_offset_z;
  }
  void getIMURadius(uint16_t *accel_radius, uint16_t *mag_radius) {
    accel_radius[0] = config.accel_radius;
    mag_radius[0] = config.mag_radius;
  }


  void setIMUAccelOffset(uint16_t accel_offset_x, uint16_t accel_offset_y, uint16_t accel_offset_z) {
    config.accel_offset_x = accel_offset_x;
    config.accel_offset_y = accel_offset_y;
    config.accel_offset_z = accel_offset_z;
  }
  void setIMUGyroOffset(uint16_t gyro_offset_x, uint16_t gyro_offset_y, uint16_t gyro_offset_z) {
    config.gyro_offset_x = gyro_offset_x;
    config.gyro_offset_y = gyro_offset_y;
    config.gyro_offset_z = gyro_offset_z;
  }
  void setIMUMagOffset(uint16_t mag_offset_x, uint16_t mag_offset_y, uint16_t mag_offset_z) {
    config.mag_offset_x = mag_offset_x;
    config.mag_offset_y = mag_offset_y;
    config.mag_offset_z = mag_offset_z;
  }
  void setIMURadius(uint16_t accel_radius, uint16_t mag_radius) {
    config.accel_radius = accel_radius;
    config.mag_radius = mag_radius;
  }

  float getMastHeight() {
    return config.mastheight;
  }
  float getKFactor() {
    return config.kfactor;
  }
  uint8_t getWindSpeedDamping() {
    return config.windSpeedDamping;
  }
  uint8_t getWaterSpeedDamping() {
    return config.waterSpeedDamping;
  }
  uint8_t getWindAngleDamping() {
    return config.windAngleDamping;
  }
  int *getWindAngleMin() {
    return &config.windAngleMin[0];
  }
  int *getWindAngleMax() {
    return &config.windAngleMax[0];
  }
  float getWindAngleOffset() {
    return config.windAngleOffset;
  }
  float *getWindSpeedFrequencies() {
    return &config.windSpeedFrequencies[0];
  }
  float *getWindSpeedPulsePerM() {
    return &config.windSpeedPulsesPerM[0];
  }
  uint8_t getWindSpeedCalibrations() {
    return config.nwindSpeedCalibrations;
  }
  float *getWaterSpeedFrequencies() {
    return &config.waterSpeedFrequencies[0];
  }
  float *getWaterSpeedPulsePerM() {
    return &config.waterSpeedPulsesPerM[0];
  }
  uint8_t getWaterSpeedCalibrations() {
    return config.nwaterSpeedCalibrations;
  }


  void testPatterns() {
    int patternLen = strlen(TESTPATTERN);
    for ( int i = 0, l = 0; i < patternLen; i++ ) {
      if ( TESTPATTERN[i] == '\n') {
        strncpy(commandBuffer, &TESTPATTERN[l], i-l);
        commandBuffer[i-l] = 0;
        l = i+1;
        parseCommand();
        if ( !commandProcessed ) {
          controlStream->println(F("ERROR ^^^^^^^^^^^^^^ "));
        }    
      }
    } 
  }


private:
  Stream *controlStream;
  tConfigObj config;
  char commandBuffer[MAX_LINE_LENGTH];
  uint8_t cbp;
  bool logging;
  bool status;
  demo_data_t *demoData;
  bool commandProcessed;
  File polarStream;
  bool polarStreamOpen;


  void dumpFlag(uint8_t flag,  const __FlashStringHelper* msg) {
    controlStream->print(msg);
    if ( (config.flags&flag) == flag) {
      controlStream->println(F("on"));
    } else {
      controlStream->println(F("off"));
    }
  }

  uint8_t parseCommand() {
    //controlStream->print(F("Got ["));
    //controlStream->print(commandBuffer);
    //controlStream->println(F("]"));
    float fields[5];
    uint8_t ret = CONFIG_NO_ACTION;
    commandProcessed = true;
    status = false;
    
    if (processConfiurationLine(commandBuffer,0)) {
      status = false;
    } else if ( strncmp(commandBuffer,CFG_LOG_ENABLE, 5) == 0 ) {
      logging = true;
    } else if ( strncmp(commandBuffer,CFG_LOG_DISABLE, 5) == 0 ) {
      logging = false;
    } else if (  strncmp(commandBuffer,CFG_STATUS_ENABLE, 8) == 0 ) {
      status = true;
    } else if (  strncmp(commandBuffer,CFG_STATUS_DISABLE, 8) == 0 ) {
      status = false;
    } else if (  strncmp(commandBuffer,DEMO_WIND_ANGLE, strlen(DEMO_WIND_ANGLE)) == 0 ) {
      if (parseToFloats(commandBuffer,fields, 5)  == 1) {
        demoData->windAngle = DegToRad(fields[0]);
      } else {
         controlStream->println(F("Error input format: demo.wind.angle,n"));
      }
    } else if (  strncmp(commandBuffer,DEMO_WIND_SPEED, strlen(DEMO_WIND_SPEED)) == 0 ) {
      if (parseToFloats(commandBuffer,fields, 5)  == 1) {
        demoData->windSpeed = knotsToMs(fields[0]);
      } else {
        controlStream->println(F("Error input format: demo.wind.speed,n"));
      }
    } else if (  strncmp(commandBuffer,DEMO_WATER_SPEED, strlen(DEMO_WATER_SPEED)) == 0 ) {
      if (parseToFloats(commandBuffer,fields, 5)  == 1) {
        demoData->waterSpeed = knotsToMs(fields[0]);
      } else {
        controlStream->println(F("Error input format: demo.water.speed,n"));
      }
    } else if (  strncmp(commandBuffer,DEMO_WATER_TEMP, strlen(DEMO_WATER_TEMP)) == 0 ) {
      if (parseToFloats(commandBuffer,fields, 5)  == 1) {
        demoData->waterTemperature = fields[0];
      } else {
        controlStream->println(F("Error input format: demo.water.temp,n"));
      }
    } else if (  strncmp(commandBuffer,DEMO_PITCH, strlen(DEMO_PITCH)) == 0 ) {
      if (parseToFloats(commandBuffer,fields, 5)  == 1) {
        demoData->orientation.y = DegToRad(fields[0]);
      } else {
        controlStream->println(F("Error input format: demo.pitch,n"));
      }
    } else if (  strncmp(commandBuffer,DEMO_ROLL, strlen(DEMO_ROLL)) == 0 ) {
      if (parseToFloats(commandBuffer,fields, 5)  == 1) {
        demoData->orientation.z = DegToRad(fields[0]);
      } else {
        controlStream->println(F("Error input format: demo.roll,n"));
      }
    } else if (  strncmp(commandBuffer,DEMO_GYRO_PITCH, strlen(DEMO_GYRO_PITCH)) == 0 ) {
      if (parseToFloats(commandBuffer,fields, 5)  == 1) {
        demoData->gyro.y = DegToRad(fields[0]);
      } else {
        controlStream->println(F("Error input format: demo.gyro.pitch,n"));
      }
    } else if (  strncmp(commandBuffer,DEMO_GYRO_ROLL, strlen(DEMO_GYRO_ROLL)) == 0 ) {
      if (parseToFloats(commandBuffer,fields, 5)  == 1) {
        demoData->gyro.z = DegToRad(fields[0]);
      } else {
        controlStream->println(F("Error input format: demo.gyro.roll,n"));
      }
    } else if (  strncmp(commandBuffer,"test", 4) == 0 ) {
      testPatterns();
    } else if (  strncmp(commandBuffer,"save.file", 6) == 0 ) {
      saveToFile(&commandBuffer[11]);
    } else if (  strncmp(commandBuffer,"load.file", 6) == 0 ) {
      loadFromFile(&commandBuffer[11]);
    } else if (  strncmp(commandBuffer,"load.config", 6) == 0 ) {
      controlStream->println(F("Updating current configuration...."));
      ret = CONFIG_UPDATE_RUNTIME;
    } else if (  strncmp(commandBuffer,"imu.cal.read", 12) == 0 ) {
      controlStream->println(F("Reading IMU Calibration...."));
      ret = CONFIG_IMU_CALIBRATION;
    } else if (  strncmp(commandBuffer,"save", 4) == 0 ) {
      saveToFlash();
    } else if (  strncmp(commandBuffer,"load", 4) == 0 ) {
      loadFromFlash();
    } else if (  strncmp(commandBuffer,"reset", 5) == 0 ) {
      factoryReset();
    } else if (  strncmp(commandBuffer,"runstate", 8) == 0 ) {
      ret = CONFIG_CURRENT_STATUS;
    } else if (  strncmp(commandBuffer,"mem", 3) == 0 ) {
      dumpMem();
    } else if (  strncmp(commandBuffer,"status", 6) == 0 ) {
      controlStream->println(F("Status "));
      if (logging) {
          controlStream->println(                      F("logging          : on"));
      } else {
          controlStream->println(                      F("logging          : off"));
      }
      dumpFlag(CONFIG_FLAGS_SENSORS_ENABLED,    F("Direct Sensors   : "));
      dumpFlag(CONFIG_FLAGS_BATTERIES_CONNECTED,F("Battery Sensors  : "));
      dumpFlag(CONFIG_FLAGS_PERFORMANCE_ENABLED,F("Performance calcs: "));
      dumpFlag(CONFIG_FLAGS_ENNVIRO_ENABLED,    F("Enviro Sensors   : "));
      dumpFlag(CONFIG_FLAGS_IMU_CALIBRATED,     F("IMU Calibrated   : "));
      dumpFlag(CONFIG_FLAGS_DEMO_ENABLED,       F("Demo             : "));

      outputConfig(controlStream);
      outputDemoData(controlStream);
    } else if (  (strncmp(commandBuffer,"?", 1) == 0 ) || (strncmp(commandBuffer,"help", 4) == 0)) {
      controlStream->println(F("Commands: "));
      controlStream->println(F("log.(enable|disable)          turn logging on or off, alternatively press Esc to turn off"));
      controlStream->println(F("status.(enable|disable)       log status messages"));
      controlStream->println(F("demo.(enable|disable)         enable demo"));
      controlStream->println(F("demo.(enable|disable)         enable demo"));
      controlStream->println(F("demo.wind.angle,n             set the demo wind angle in degress"));
      controlStream->println(F("demo.wind.speed,n             set the demo wind speed in kn"));
      controlStream->println(F("demo.water.speed,n            set the demo water speed in kn"));
      controlStream->println(F("demo.water.temp,n             set the demo water temp in C"));
      controlStream->println(F("demo.pitch,n                  set the demo pitch angle in degrees"));
      controlStream->println(F("demo.roll,n                   set the demo roll angle in degress"));
      controlStream->println(F("demo.gyro.pitch,n             set the demo gyro pitch in degrees/s"));
      controlStream->println(F("demo.gyro.roll,n              set the demo gyro roll in degrees/s"));
      controlStream->println(F("wind.angle.damping,n          set wind angle damping to n (int)"));
      controlStream->println(F("wind.sin.range,l,h            set sin range to l(int) to h(int) 12bit"));
      controlStream->println(F("wind.cos.range,l,h            set cos range to l(int) to h(int) 12bit"));
      controlStream->println(F("wind.angle.offset,o           wind angle offset to o(float degrees)"));
      controlStream->println(F("wind.speed.damping,n          set wind speed damping to n (int)"));
      controlStream->println(F("wind.reset                    wind speed calibration reset"));    
      controlStream->println(F("wind.speed.calibration,f,p    set wind speed calibration at f(float Kn) to p(float Hz/kn)"));
      controlStream->println(F("water.speed.damping,n         set water speed damping to n (int)"));
      controlStream->println(F("water.reset                   water speed calibration reset"));    
      controlStream->println(F("water.speed.calibration,f,p   set water speed calibration at f(float Kn) to p(float Hz/kn)"));
      controlStream->println(F("boat.mastheight,h             set mast height to h(float m)"));
      controlStream->println(F("boat.kfactor,k                set kfactor to k(float)"));
      controlStream->println(F("status                        dump the current settings"));
      controlStream->println(F("runstate                      Get each component to dump the current runtime status."));
      controlStream->println(F("reset                         perform a factory reset"));
      controlStream->println(F("save.file,f                   save config to file f on the attached SD card."));
      controlStream->println(F("load.file,f                   load config from file f on the attached SD card."));
      controlStream->println(F("load.config                   Load current config into runtime."));
      controlStream->println(F("save                          save config to flash"));
      controlStream->println(F("load                          load config from flash"));
      controlStream->println(F("test                          run self tests"));
      controlStream->println(F("polar,f                       Load the polar in file f on the SD card."));
      controlStream->println(F("mem                           Dump memory status."));
      controlStream->println(F("imu.accel.offset,x,y,z        Calibrate IMU Accel x,y,z uint16_t"));
      controlStream->println(F("imu.gyro.offset,x,y,z         Calibrate IMU Gyro x,y,z uint16_t"));
      controlStream->println(F("imu.mag.offset,x,y,z          Calibrate IMU Mag x,y,z uint16_t"));
      controlStream->println(F("imu.radius,accel,mag          Calibrate IMU Radius accel,mag uint16_t"));
      controlStream->println(F("imu.cal.(enable|disable)      Enable or Disable calibration data."));
      controlStream->println(F("imu.cal.read                  Read IMU calibration if fully calibrated."));
      controlStream->println(F("?|help this message."));
    } else {
      commandProcessed = false;
      controlStream->print(F("ERROR: Command not recognised :"));
      controlStream->println(commandBuffer);
    }
    return ret;
  }


    int32_t dynamic_ram;
    int32_t static_ram;
    int32_t stack_used;
    int32_t free;

  void dumpMem() {
      meminfo_t info;
      freeMemory(&info);
      controlStream->print(F("Dynamic Ram used  :")); 
      controlStream->println(info.dynamic_ram);
      controlStream->print(F("Static Ram used   :")); 
      controlStream->println(info.static_ram);
      controlStream->print(F("Stack used        :")); 
      controlStream->println(info.stack_used);
      controlStream->print(F("Free              :")); 
      controlStream->println(info.free);      
  }



  void factoryReset() {
    config.flags = FACTORY_DEFAULT;
    config.windSpeedDamping = 3;
    config.windAngleDamping = 3;
    config.waterSpeedDamping = 3;
    config.windAngleMin[0] = 0;
    config.windAngleMin[1] = 0;
    config.windAngleMax[0] = (uint16_t)4096;
    config.windAngleMax[1] = (uint16_t)4096;
    config.windAngleOffset = 0.0F;
    config.nwindSpeedCalibrations = 1;
    config.nwaterSpeedCalibrations = 1;
    config.windSpeedFrequencies[0] = 0.0F;
    config.windSpeedPulsesPerM[0] = hzPerKnToHzPerMPerS(1.045);
    config.waterSpeedFrequencies[0] = 0.0F;
    config.waterSpeedPulsesPerM[0] = hzPerKnToHzPerMPerS(5.5);
    config.polarFile[0] = 0;
    INFOLN(F("Factory reset done."));
  }

  bool saveToFile(char *configFileName) {
    if (!SD.begin(4)) {
      INFOLN(F("Initialization from SD Card failed, reverting to factory settings."));
      return false;
    }
    File configStream = SD.open(configFileName, FILE_WRITE);
    if (! configStream) {
      return false;
    } else {
      outputConfig(&configStream);
      configStream.close();
      return true;
    }
  }



  void outputDemoData(Stream *configStream) {
    configStream->print(DEMO_ROLL);
    configStream->print(",");
    configStream->println(RadToDeg(demoData->orientation.z));
    configStream->print(DEMO_PITCH);
    configStream->print(",");
    configStream->println(RadToDeg(demoData->orientation.y));
    configStream->print(DEMO_GYRO_ROLL);
    configStream->print(",");
    configStream->println(RadToDeg(demoData->gyro.z));
    configStream->print(DEMO_GYRO_PITCH);
    configStream->print(",");
    configStream->println(RadToDeg(demoData->gyro.y));
    configStream->print(DEMO_WIND_ANGLE);
    configStream->print(",");
    configStream->println(RadToDeg(demoData->windAngle));
    configStream->print(DEMO_WIND_SPEED);
    configStream->print(",");
    configStream->println(msToKnots(demoData->windSpeed));
    configStream->print(DEMO_WATER_SPEED);
    configStream->print(",");
    configStream->println(msToKnots(demoData->waterSpeed));
    configStream->print(DEMO_WATER_TEMP);
    configStream->print(",");
    configStream->println(demoData->waterTemperature);
  }

  void outputConfig(Stream *configStream) {
    if (getDemoMode()) {
      configStream->println(CFG_DEMO_ENABLE);
    } else {
      configStream->println(CFG_DEMO_DISABLE);
    }
    if (getPerformanceEnabled()) {
      configStream->println(CFG_PERF_ENABLE);
    } else {
      configStream->println(CFG_PERF_DISABLE);
    }
    if (getSensorsEnabled()) {
      configStream->println(CFG_SENSOR_ENABLE);
    } else {
      configStream->println(CFG_SENSOR_DISABLE);
    }
    configStream->print(CFG_BOAT_MASTHEIGHT);
    configStream->print(",");
    configStream->println(config.mastheight);
    configStream->print(CFG_BOAT_KFACTOR);
    configStream->print(",");
    configStream->println(config.kfactor);
    configStream->print(CFG_WIND_ANGLE_DAMPING);
    configStream->print(",");
    configStream->println(config.windAngleDamping);
    configStream->print(CFG_WIND_SIN_RANGE);
    configStream->print(",");
    configStream->print(config.windAngleMin[0]);
    configStream->print(",");
    configStream->println(config.windAngleMax[0]);
    configStream->print(CFG_WIND_COS_RANGE);
    configStream->print(",");
    configStream->print(config.windAngleMin[1]);
    configStream->print(",");
    configStream->println(config.windAngleMax[1]);
    configStream->print(CFG_WIND_ANGLE_OFFSET);
    configStream->print(",");
    configStream->println(config.windAngleOffset);

    configStream->print(CFG_WIND_SPEED_DAMPING);
    configStream->print(",");
    configStream->println(config.windSpeedDamping);
    for ( int i = 0; i < config.nwindSpeedCalibrations; i++) {
      configStream->print(CFG_WIND_SPEED_CALLIBRATION);
      configStream->print(",");
      configStream->print(getCalibSpeedKn(config.windSpeedFrequencies[i],config.windSpeedPulsesPerM[i]));
      configStream->print(",");
      configStream->println(hzPerMPerSToHzPerKn(config.windSpeedPulsesPerM[i]));
    }
    for ( int i = 0; i < config.nwindSpeedCalibrations; i++) {
      configStream->print(F("Raw data,"));
      configStream->print(",");
      configStream->print(config.windSpeedFrequencies[i]);
      configStream->print(",");
      configStream->println(config.windSpeedPulsesPerM[i]);
    }
    configStream->print(CFG_WATER_SPEED_DAMING);
    configStream->print(",");
    configStream->println(config.waterSpeedDamping);
    for ( int i = 0; i < config.nwaterSpeedCalibrations; i++) {
      configStream->print(CFG_WATER_SPEED_CALIBRATION);
      configStream->print(",");
      configStream->print(getCalibSpeedKn(config.waterSpeedFrequencies[i],config.waterSpeedPulsesPerM[i]));
      configStream->print(",");
      configStream->println(hzPerMPerSToHzPerKn(config.waterSpeedPulsesPerM[i]));
    }
    for ( int i = 0; i < config.nwaterSpeedCalibrations; i++) {
      configStream->print(F("Raw data"));
      configStream->print(",");
      configStream->print(config.waterSpeedFrequencies[i]);
      configStream->print(",");
      configStream->println(config.waterSpeedPulsesPerM[i]);
    }
    if (getImuCalEnabled()) {
      configStream->println(CFG_IMU_CAL_ENABLE);
    } else {
      configStream->println(CFG_IMU_CAL_DISABLE);
    }
    configStream->print(CFG_IMU_ACCEL_OFFSET);
    configStream->print(",");
    configStream->print(config.accel_offset_x);
    configStream->print(",");
    configStream->print(config.accel_offset_y);
    configStream->print(",");
    configStream->println(config.accel_offset_z);
    configStream->print(CFG_IMU_GYRO_OFFSET);
    configStream->print(",");
    configStream->print(config.gyro_offset_x);
    configStream->print(",");
    configStream->print(config.gyro_offset_y);
    configStream->print(",");
    configStream->println(config.gyro_offset_z);
    configStream->print(CFG_IMU_MAG_OFFSET);
    configStream->print(",");
    configStream->print(config.mag_offset_x);
    configStream->print(",");
    configStream->print(config.mag_offset_y);
    configStream->print(",");
    configStream->println(config.mag_offset_z);
    configStream->print(CFG_IMU_RADIUS);
    configStream->print(",");
    configStream->print(config.accel_radius);
    configStream->print(",");
    configStream->println(config.mag_radius);
    configStream->print(CFG_POLAR);
    configStream->print(",");
    configStream->println(config.polarFile);

  }


  int parseToFloats(char *readBuffer, float *fields, int maxfields) {
    return parseNumbers(readBuffer, fields, 0, true, maxfields);
  }


  int parseToInts(char *readBuffer, int32_t *ifields, int maxfields) {
    return parseNumbers(readBuffer, 0, ifields, false, maxfields);
  }



  int parseNumbers(char *readBuffer, float *fields, int32_t *ifields, bool asFloats, int maxfields) {



    int nfld = 0, cmdlen = strlen(readBuffer), fstart = 0;
    // replace , with a 0 and parse fields to floats.
    for ( int i = 0; i < cmdlen && nfld < maxfields; i++) {
      if (readBuffer[i] == ',' || readBuffer[i] == 0) {
        //controlStream->print("Field detected ");
        //controlStream->println(i);
        readBuffer[i] = 0;
        if (fstart > 0) {
          if ( asFloats ) {
            fields[nfld++] = atof(&readBuffer[fstart]);                    
          } else {
            ifields[nfld++] = atol(&readBuffer[fstart]);                    
            controlStream->print("Loaded field [");
            controlStream->print(&readBuffer[fstart]);
            controlStream->print("]");
          }
        }
        fstart = i+1;
        if ( i != cmdlen ) {
          readBuffer[i] = ',';
        }
      }
    }
    if ( fstart > 0 && fstart < cmdlen) {
      if ( asFloats ) {
        fields[nfld++] = atof(&readBuffer[fstart]);
      } else {
        ifields[nfld++] = atol(&readBuffer[fstart]);
        controlStream->print("Loaded last field [");
        controlStream->print(&readBuffer[fstart]);
        controlStream->print("]");
      } 
    }
    //controlStream->print("fstart  ");
    //controlStream->println(fstart);
    //controlStream->print("cmdlen  ");
    //controlStream->println(cmdlen);
    //controlStream->print("Fields  ");
    //controlStream->println(nfld);
    return nfld;

  }

  bool processConfiurationLine(char *readBuffer,  int lineno) {
    float fields[MAX_FIELDS];
    int nfld = parseToFloats(readBuffer, fields, MAX_FIELDS);
    // save the data.
    //INFO("Parse config line [");
    //INFO(readBuffer);
    //INFON("]");
    if ( strncmp(CFG_DEMO_ENABLE, readBuffer, 6) == 0 ) {
      config.flags = config.flags | CONFIG_FLAGS_DEMO_ENABLED;
    } else if ( strncmp(CFG_DEMO_DISABLE, readBuffer,6) == 0 ) {
      config.flags = config.flags & ~CONFIG_FLAGS_DEMO_ENABLED;
    } else if ( strncmp(CFG_PERF_ENABLE, readBuffer,6) == 0 ) {
      config.flags = config.flags | CONFIG_FLAGS_PERFORMANCE_ENABLED;
    } else if ( strncmp(CFG_PERF_DISABLE, readBuffer,6) == 0 ) {
      config.flags = config.flags & ~CONFIG_FLAGS_PERFORMANCE_ENABLED;
    } else if ( strncmp(CFG_SENSOR_ENABLE, readBuffer,8) == 0 ) {
      config.flags = config.flags | CONFIG_FLAGS_SENSORS_ENABLED;
    } else if ( strncmp(CFG_SENSOR_DISABLE, readBuffer,8) == 0 ) {
      config.flags = config.flags & ~CONFIG_FLAGS_SENSORS_ENABLED;
    } else if ( strncmp(CFG_BOAT_MASTHEIGHT, readBuffer,10) == 0 ) {
      if(nfld != 1) {
        INFO(F("Format Error boat.mastheight,mastheight at line:"));
        INFON(lineno);
      } else {
        config.mastheight = fields[0];
      }
    } else if ( strncmp(CFG_BOAT_KFACTOR, readBuffer, 8) == 0 ) {
      if(nfld != 1) {
        INFO(F("Format Error boat.kfactor,kfactor at line:"));
        INFON(lineno);
      } else {
        config.kfactor = fields[0];
      }
    } else if ( strncmp(CFG_WIND_ANGLE_DAMPING, readBuffer,13) == 0 ) {
      if(nfld != 1) {
        INFO(F("Format Error wind.angle.damping,damping at line:"));
        INFON(lineno);
      } else {
        config.windAngleDamping = (uint8_t)fields[0];
      }
    } else if ( strncmp(CFG_WIND_SIN_RANGE, readBuffer,12) == 0 ) {
      if(nfld != 2) {
        INFO(F("Format Error wind.sin.range,min,max at line:"));
        INFON(lineno);
      } else {
        config.windAngleMin[0] = (uint16_t)fields[0];
        config.windAngleMax[0] = (uint16_t)fields[1];
      }

    } else if ( strncmp(CFG_WIND_COS_RANGE, readBuffer, 12) == 0 ) {
      if(nfld != 2) {
        INFO(F("Format Error wind.cos.range,min,max at line:"));
        INFON(lineno);
      } else {
        config.windAngleMin[1] = (uint16_t)fields[0];
        config.windAngleMax[1] = (uint16_t)fields[1];
      }
    } else if ( strncmp(CFG_WIND_ANGLE_OFFSET, readBuffer, 13) == 0 ) {
      if(nfld != 1) {
        INFO(F("Format Error wind.angle.offset,offset at line:"));
        INFON(lineno);
      } else {
        config.windAngleOffset = fields[0];
      }
    } else if ( strncmp(CFG_WIND_SPEED_DAMPING, readBuffer, 14) == 0 ) {
      if(nfld != 1) {
        INFO(F("Format Error wind.speed.damping,damping at line:"));
        INFON(lineno);
      } else {
        config.windSpeedDamping = (uint8_t)fields[0];
      }
    } else if ( strncmp(CFG_WIND_SPEED_CALIBRATION_RESET, readBuffer, 10) == 0 ) {
      config.nwindSpeedCalibrations = 0;
    } else if ( strncmp(CFG_WIND_SPEED_CALLIBRATION, readBuffer, 15) == 0 ) {
      if(nfld != 2) {
        INFO(F("Format Error wind.speed.calibration,speedKn,hzPerKn at line:"));
        INFON(lineno);
      } else {
        config.windSpeedFrequencies[config.nwindSpeedCalibrations] = fields[0]*fields[1]; // calibration frequency.
        config.windSpeedPulsesPerM[config.nwindSpeedCalibrations] = hzPerKnToHzPerMPerS(fields[1]); // *1852 == m/h  / 3600 = m/s
        config.nwindSpeedCalibrations++;
      }
    } else if ( strncmp(CFG_WATER_SPEED_DAMING, readBuffer, 15) == 0 ) {
      if(nfld != 1) {
        INFO(F("Format Error water.speed.damping,damping at line:"));
        INFON(lineno);
      } else {
        config.waterSpeedDamping = (uint8_t)fields[0];
      }
    } else if ( strncmp(CFG_WATER_SPEED_CALIBRATION_RESET, readBuffer, 11) == 0 ) {
      config.nwaterSpeedCalibrations = 0;
    } else if ( strncmp(CFG_WATER_SPEED_CALIBRATION, readBuffer, 16) == 0 ) {
      if(nfld != 2) {
        INFO(F("Format Error water.speed.calibration,speedKn,hzPerKn at line:"));
        INFON(lineno);
      } else {
        config.waterSpeedFrequencies[config.nwaterSpeedCalibrations] = fields[0]*fields[1];
        config.waterSpeedPulsesPerM[config.nwaterSpeedCalibrations] = hzPerKnToHzPerMPerS(fields[1]); 
        config.nwaterSpeedCalibrations++;
      }
    } else if ( strncmp(CFG_POLAR, readBuffer, 5) == 0 ) {
      config.polarFile[0] = 0;
      int i =  strlen(readBuffer);
      if ( i > 6) {
        strcpy(config.polarFile, &readBuffer[6]);
      }
    } else if ( strncmp(CFG_IMU_ACCEL_OFFSET, readBuffer, 16) == 0 ) {
      int32_t ifields[3];
      nfld = parseToInts(readBuffer, ifields, 3);
      if(nfld != 3) {
        INFO(F("Format Error imu.accel.offset,x,y,z"));
        INFON(lineno);
      } else {
        INFOLN(readBuffer);
        INFO(F("imu.accel.offset,"));
        INFOC(ifields[0]);
        INFOC(F(","));
        INFOC(ifields[1]);
        INFOC(F(","));
        INFON(ifields[2]);
        config.accel_offset_x = ifields[0];
        config.accel_offset_y = ifields[1];
        config.accel_offset_z = ifields[2];
      }
    } else if ( strncmp(CFG_IMU_GYRO_OFFSET, readBuffer, 15) == 0 ) {
      int32_t ifields[3];
      nfld = parseToInts(readBuffer, ifields, 3);
      if(nfld != 3) {
        INFO(F("Format Error imu.gyro.offset,x,y,z"));
        INFON(lineno);
      } else {
        INFOLN(readBuffer);
        INFO(F("imu.gyro.offset,"));
        INFOC(ifields[0]);
        INFOC(F(","));
        INFOC(ifields[1]);
        INFOC(F(","));
        INFON(ifields[2]);
        config.gyro_offset_x = ifields[0];
        config.gyro_offset_y = ifields[1];
        config.gyro_offset_z = ifields[2];
      }
    } else if ( strncmp(CFG_IMU_MAG_OFFSET, readBuffer, 14) == 0 ) {
      int32_t ifields[3];
      nfld = parseToInts(readBuffer, ifields, 3);
      if(nfld != 3) {
        INFO(F("Format Error imu.mag.offset,x,y,z"));
        INFON(lineno);
      } else {
        INFOLN(readBuffer);
        INFO(F("imu.mag.offset,"));
        INFOC(ifields[0]);
        INFOC(F(","));
        INFOC(ifields[1]);
        INFOC(F(","));
        INFON(ifields[2]);
        config.mag_offset_x = ifields[0];
        config.mag_offset_y = ifields[1];
        config.mag_offset_z = ifields[2];
      }
    } else if ( strncmp(CFG_IMU_RADIUS, readBuffer, 10) == 0 ) {
      int32_t ifields[2];
      nfld = parseToInts(readBuffer, ifields, 2);
      if(nfld != 2) {
        INFO(F("Format Error imu.radius,accel,mag"));
        INFON(lineno);
      } else {
        INFOLN(readBuffer);
        INFO(F("imu.radius,"));
        INFOC(ifields[0]);
        INFOC(F(","));
        INFON(ifields[1]);
        config.accel_radius = ifields[0];
        config.mag_radius = ifields[1];
      }
    } else if ( strncmp(CFG_IMU_CAL_ENABLE, readBuffer,10) == 0 ) {
      config.flags = config.flags | CONFIG_FLAGS_IMU_CALIBRATED;
    } else if ( strncmp(CFG_IMU_CAL_DISABLE, readBuffer,10) == 0 ) {
      config.flags = config.flags & ~CONFIG_FLAGS_IMU_CALIBRATED;
    } else {
      return false;
    }
    return true;
  }

  double getCalibSpeedKn(double f, double hpms ) {
      if ( hpms < 1E-8 ) {
        return 999.0F;  
      } else {
        return msToKnots(f/hpms); 
      }
  }



  bool loadFromFile(char *configFileName) {
    if (!SD.begin(4)) {
      INFOLN(F("Initialization from SD Card failed, reverting to factory settings."));
      return false;
    }
    // re-open the file for reading:
    File configStream = SD.open(configFileName);
    if ( !configStream ) {
      // if the file didn't open, print an error:
      INFO(F("error opening "));
      INFON(configFileName);
      return false;
    }
    int lineno = 0, fbp=0;
    char readBuffer[MAX_LINE_LENGTH+1];
    char lastChar;
    // read from the file until there's nothing else in it:
    while (configStream.available() && (lastChar = configStream.read() != -1)) {
      switch(lastChar) {
        case NewLine:
          readBuffer[fbp++] = 0;
          lineno++;
          processConfiurationLine(readBuffer, lineno);
          fbp = 0;
          break;
        default:
          if (lastChar >= Space && lastChar < Del && fbp < MAX_LINE_LENGTH) {
            readBuffer[fbp++] = lastChar;
          }
      }
    }
    configStream.close();
    INFO(F("Loaded config with  "));
    INFO(lineno);
    INFON(F(" lines."));
    return true;
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
        INFOLN(F("Failed to save, no space left "));
        return false;
    }
    writeFlash(0, configSize);
    writeFlash(sizeof(uint16_t), config);
    uint16_t checksum = crc16EEprom(0, sizeof(uint16_t)+sizeof(tConfigObj));
    writeFlash(sizeof(uint16_t)+sizeof(tConfigObj),checksum);
    INFOLN(F("Saved"));
    return true;
  }
  bool loadFromFlash() {
    uint16_t configSize = 0;
    readFlash(0, configSize);
    if ( configSize != (sizeof(tConfigObj)+2*sizeof(uint16_t))) {
        INFOLN(F("No config in flash"));
        return false;
    }
    uint16_t checksum = 0;
    readFlash(sizeof(uint16_t)+sizeof(tConfigObj), checksum);
    if ( checksum != crc16EEprom(0, sizeof(uint16_t)+sizeof(tConfigObj))) {
        INFOLN(F("Checksum failed"));
        return false;
    }
    readFlash(sizeof(uint16_t), config);
    INFOLN(F("Loaded config"));
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

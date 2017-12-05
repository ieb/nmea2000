
#ifndef __IMUSENSOR_H__
#define __IMUSENSOR_H__

#ifndef TEST
#include <N2kMessages.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <SPI.h>
#include <SD.h>

#endif

#include "demo.h"
#include "configuration.h"

#define PI (double)3.1415926535897932384626433832795





class IMUSensor {
public:
  IMUSensor(Adafruit_BNO055 *bno, demo_data_t *demoData) {
    this->bno = bno;
    this->demoData = demoData;
    this->enabled = false;
    this->demoMode = false;
    this->fullyCalibrated = false;
  }

  bool init() {
    if (enabled) {
      return true;
    }
    if(!bno->begin()) {
      INFOLN(F("No BNo055 sensor present on I2C check wiring."));
      return false;
    } else {
      INFOLN(F("Found BNo055"));

      uint8_t system_status, self_test_results, system_error;
      system_status = self_test_results = system_error = 0;
      bno->getSystemStatus(&system_status, &self_test_results, &system_error);
      if ( system_status != 0x05) {
        INFO(F("BNo055 Fusion algorithm not running. (see section 4.3.58) status:"));
        INFON2(system_status, HEX);
      }
      if ( self_test_results != 0x0F) {
        INFO(F("BNo055 Self tests failed with status :"));
        INFON2(self_test_results, HEX);
      }
      if ( system_error != 0x00) {
        INFO(F("BNo055 system error (see section 4.3.59) :"));
        INFON2(system_error, HEX);
      }



      enabled = true;
      delay(1000);
      bno->setExtCrystalUse(true);
      return true;
    }
  }



  void updateConfiguration(Configuration *config) {
    if(!init() ) {
      return;
    }
    demoMode =  config->getDemoMode();
    dumpRunstate();

    if ( config->getImuCalEnabled()) {
      config->getIMUAccelOffset(&(calibrationData.accel_offset_x), &(calibrationData.accel_offset_y), &(calibrationData.accel_offset_z));
      config->getIMUGyroOffset(&(calibrationData.gyro_offset_x), &(calibrationData.gyro_offset_y), &(calibrationData.gyro_offset_z));
      config->getIMUMagOffset(&(calibrationData.mag_offset_x), &(calibrationData.mag_offset_y), &(calibrationData.mag_offset_z));
      config->getIMURadius(&(calibrationData.accel_radius), &(calibrationData.mag_radius));
      displayCalibrationData();
      bno->setSensorOffsets(calibrationData);
      bno->setMode(Adafruit_BNO055::OPERATION_MODE_NDOF);
      INFOLN(F("IMU Loaded Calibration."));
      delay(1000);
      bno->getSensorOffsets(calibrationData);
      displayCalibrationData();
      fullyCalibrated = true;
    } else {
      fullyCalibrated = false;
      INFOLN(F("IMU Calibrating .... "));
      sensors_event_t event;
      bno->getEvent(&event);
      delay(100);        
      while ( ! bno->isFullyCalibrated() ) {
        bno->getEvent(&event);
        INFO(F("x,"));
        INFOC(event.orientation.x);
        INFOC(F(",y,"));
        INFOC(event.orientation.y);
        INFOC(F(",z,"));
        INFOC(event.orientation.z);
        /* Get the four calibration values (0..3) */
        /* Any sensor data reporting 0 should be ignored, */
        /* 3 means 'fully calibrated" */
        uint8_t system, gyro, accel, mag;
        system = gyro = accel = mag = 0;
        bno->getCalibration(&system, &gyro, &accel, &mag);

        /* The data should be ignored until the system calibration is > 0 */
        if (!system) {
          INFOC("! ");
        } else {
          INFOC("  ");

        }

        /* Display the individual values */
        INFOC(F("Sys:"));
        INFOC2(system, DEC);
        INFOC(F(" G:"));
        INFOC2(gyro, DEC);
        INFOC(F(" A:"));
        INFOC2(accel, DEC);
        INFOC(F(" M:"));
        INFON2(mag, DEC);
        delay(100);        
      }
      bno->getSensorOffsets(calibrationData);
      displayCalibrationData();
      fullyCalibrated = true;
      INFOLN(F("IMU Calibration complete, please save data......................................"));      
      dumpRunstate();
    }
  }

  void   displayCalibrationData(void) {
    INFO(F("Accelerometer: "));
    INFOC(calibrationData.accel_offset_x); INFOC(F(" "));
    INFOC(calibrationData.accel_offset_y); INFOC(F(" "));
    INFOC(calibrationData.accel_offset_z);

    INFOC(F("\nGyro: "));
    INFOC(calibrationData.gyro_offset_x); INFOC(F(" "));
    INFOC(calibrationData.gyro_offset_y); INFOC(F(" "));
    INFOC(calibrationData.gyro_offset_z);

    INFOC(F("\nMag: "));
    INFOC(calibrationData.mag_offset_x); INFOC(F(" "));
    INFOC(calibrationData.mag_offset_y); INFOC(F(" "));
    INFOC(calibrationData.mag_offset_z);

    INFOC(F("\nAccel Radius: "));
    INFOC(calibrationData.accel_radius);

    INFOC(F("\nMag Radius: "));
    INFON(calibrationData.mag_radius);
  }

  bool readConfiguration(Configuration *config) {
    if ( fullyCalibrated ){
      config->setIMUAccelOffset(calibrationData.accel_offset_x, calibrationData.accel_offset_y, calibrationData.accel_offset_z);
      config->setIMUGyroOffset(calibrationData.gyro_offset_x, calibrationData.gyro_offset_y, calibrationData.gyro_offset_z);
      config->setIMUMagOffset(calibrationData.mag_offset_x, calibrationData.mag_offset_y, calibrationData.mag_offset_z);
      config->setIMURadius(calibrationData.accel_radius, calibrationData.mag_radius);
      config->setImuCalEnabled(true);
    }
    return fullyCalibrated;
  }

  void dumpRunstate(void) {
      sensor_t sensor;
      bno->getSensor(&sensor);
      Adafruit_BNO055::adafruit_bno055_rev_info_t rev_info;
      bno->getRevInfo(&rev_info);
      INFOLN(F("IMU Status ------------------------------------"));
      INFO(F("Sensor:       ")); INFON(sensor.name);
      INFOC(F("Driver Ver:   ")); INFON(sensor.version);
      INFOC(F("Unique ID:    ")); INFON(sensor.sensor_id);
      INFOC(F("Max Value:    ")); INFON(sensor.max_value); 
      INFOC(F("Min Value:    ")); INFON(sensor.min_value); 
      INFOC(F("Resolution:   ")); INFON(sensor.resolution); 
      INFOC(F("accel_rev:   ")); INFON(rev_info.accel_rev); 
      INFOC(F("mag_rev:   ")); INFON(rev_info.mag_rev); 
      INFOC(F("gyro_rev:   ")); INFON(rev_info.gyro_rev); 
      INFOC(F("bl_rev:   ")); INFON(rev_info.bl_rev); 
      INFOC(F("sw_rev:   ")); INFON(rev_info.sw_rev); 
      /* Get the system status values (mostly for debugging purposes) */
      uint8_t system_status, self_test_results, system_error;
      system_status = self_test_results = system_error = 0;
      bno->getSystemStatus(&system_status, &self_test_results, &system_error);
      /* Display the results in the Serial Monitor */
      INFO(F("System Status: 0x"));
      INFON2(system_status, HEX);
      INFO(F("Self Test:     0x"));
      INFON2(self_test_results, HEX);
      INFOC(F("System Error:  0x"));
      INFON2(system_error, HEX);
      INFO(F("CalStatus: "));
      /* Get the four calibration values (0..3) */
      /* Any sensor data reporting 0 should be ignored, */
      /* 3 means 'fully calibrated" */
      uint8_t system, gstat, astat, mstat;
      system = gstat = astat = mstat = 0;
      bno->getCalibration(&system, &gstat, &astat, &mstat);

      /* The data should be ignored until the system calibration is > 0 */
      if (!system) {
        INFOC("! ");
      } else {
        INFOC("  ");

      }

      /* Display the individual values */
      INFOC(F("Sys:"));
      INFOC2(system, DEC);
      INFOC(F(" G:"));
      INFOC2(gstat, DEC);
      INFOC(F(" A:"));
      INFOC2(astat, DEC);
      INFOC(F(" M:"));
      INFON2(mstat, DEC);
      INFO(F("Fully Calibrated: "));
      if ( bno->isFullyCalibrated()) {
        INFON(F("yes"));
      } else {
        INFON(F("no"));
      }
      INFOLN(F("IMU Status ------------------------------------"));
      INFO(F("x,"));
      INFOC(orientation.x);
      INFOC(F(",y,"));
      INFOC(orientation.y);
      INFOC(F(",z,"));
      INFOC(orientation.z);
      INFOC(F(",gx,"));
      INFOC(gyro.x);
      INFOC(F(",gy,"));
      INFOC(gyro.y);
      INFOC(F(",gz,"));
      INFON(gyro.z);    

  }


  void read() {
    if ( demoMode ) {
      orientation.x = demoData->orientation.x;
      orientation.y = demoData->orientation.y;
      orientation.z = demoData->orientation.z;
      gyro.x = demoData->gyro.x;
      gyro.y = demoData->gyro.y;
      gyro.z = demoData->gyro.z;
      LOGLN(F("IMU in Demo mode"));
    } else {

      if ( fullyCalibrated ) {
        imu::Vector<3> v = bno->getVector(Adafruit_BNO055::VECTOR_EULER);
        orientation.x = v.x();
        orientation.y = v.y();
        orientation.z = v.z();
        // g was allocated on the stack, this feels really dodgy accessing the stack after
        // a return.
        imu::Vector<3> g = bno->getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
        gyro.x = g.x();
        gyro.y = g.y();
        gyro.z = g.z();

        LOG(F("x,"));
        LOGC(orientation.x);
        LOGC(F(",y,"));
        LOGC(orientation.y);
        LOGC(F(",z,"));
        LOGC(orientation.z);
        LOGC(F(",gx,"));
        LOGC(gyro.x);
        LOGC(F(",gy,"));
        LOGC(gyro.y);
        LOGC(F(",gz,"));
        LOGC(gyro.z);        
        LOGLN(F(" "));
        // output was in degress, convert to radians.
        orientation.x = DegToRad(orientation.x);
        orientation.y = DegToRad(orientation.y);
        orientation.z = DegToRad(orientation.z);
        gyro.x = DegToRad(gyro.x);
        gyro.y = DegToRad(gyro.y);
        gyro.z = DegToRad(gyro.z);

        adjustOrientation();
      } else {
        orientation.x = 0.0F;
        orientation.y = 0.0F;
        orientation.z = 0.0F;
        gyro.x = 0.0F;
        gyro.y = 0.0F;
        gyro.z = 0.0F;
        LOGLN(F("IMU Calibration incomplete."));
      }
    }
  }
  void adjustOrientation() {
    // TODO, implement axis rotation.
  }
  double getPitch() {
    return orientation.y;
  }
  double getRoll() {
    return orientation.z;
  }
  double getGyroPitch() {
    return gyro.y;
  }
  double getGyroRoll() {
    return gyro.z;
  }
private:
  sensors_vec_t orientation;
  sensors_vec_t gyro; 
  Adafruit_BNO055 *bno;
  adafruit_bno055_offsets_t calibrationData;
  demo_data_t *demoData;
  bool demoMode;
  bool enabled;
  bool fullyCalibrated;



};




#endif
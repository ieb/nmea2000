#ifndef __MOTION_SENSOR_H__
#define __MOTION_SENSOR_H__


// tested by multiSensor.cp

#ifndef TEST
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303D_U.h>
#include <Adafruit_L3GD20_U.h>
#endif

Adafruit_LSM303D_Accel_Unified accel = Adafruit_LSM303D_Accel_Unified(30301);
Adafruit_L3GD20_Unified       gyro  = Adafruit_L3GD20_Unified(20);

#define PI_F 3.1415926535897932384626433832795
/**
 * Uses a 10DOF sensor with LSM303D, L3GD20 chips on standard i2C addresses. The Z axis is vertical.
 * The x axis pointing forwards and the y axis pointing sideways.
 * call read to read the values, then the data will be in orientation.roll, orientation.pitch, orientation.headding
 * in radians. 
 * Gyro contains gyro_event.gyro.x, y, z in radians/s and a timestamp gyro_event.timestamp 
 * Accell contains accell_event.acceleration.x, y, x in m/s inclding gravity and accell_event.timestamp
 * Mag contains mag_event.magnetic.x, y, z and mag_event.timestamp
 */
class MotionSensor {
public:
    sensors_event_t accel_event;
    sensors_event_t gyro_event;
    sensors_vec_t   orientation;

    MotionSensor() {

    }
    void begin() {
        accel.begin();
        gyro.begin();
    }

    /**
     * Read all the motion sensors returning true if all read ok, false if one did not respond.
     */
    bool read() {
        if ( !accel.getEvent(&accel_event) ) {
            return false;
        }

        // From ADAFruit 10Dof.
          double t_pitch;
          double t_roll;
          double t_heading;
          double signOfZ = accel_event.acceleration.z >= 0 ? 1.0F : -1.0F;


          /* roll: Rotation around the longitudinal axis (the plane body, 'X axis'). -90<=roll<=90    */
          /* roll is positive and increasing when moving downward                                     */
          /*                                                                                          */
          /*                                 y                                                        */
          /*             roll = atan(-----------------)                                               */
          /*                          sqrt(x^2 + z^2)                                                 */
          /* where:  x, y, z are returned value from accelerometer sensor                             */

          t_roll = accel_event.acceleration.x * accel_event.acceleration.x + accel_event.acceleration.z * accel_event.acceleration.z;
          orientation.roll = atan2(accel_event.acceleration.y, sqrt(t_roll));

          /* pitch: Rotation around the lateral axis (the wing span, 'Y axis'). -180<=pitch<=180)     */
          /* pitch is positive and increasing when moving upwards                                     */
          /*                                                                                          */
          /*                                 x                                                        */
          /*            pitch = atan(-----------------)                                               */
          /*                          sqrt(y^2 + z^2)                                                 */
          /* where:  x, y, z are returned value from accelerometer sensor                             */

          t_pitch = accel_event.acceleration.y * accel_event.acceleration.y + accel_event.acceleration.z * accel_event.acceleration.z;
          orientation.pitch = atan2(accel_event.acceleration.x, signOfZ * sqrt(t_pitch));

        if (!gyro.getEvent(&gyro_event) ) {
            return false;
        }
        return true;
    }

private:


};

#endif
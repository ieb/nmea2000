#include <Arduino.h>
#include "motionSensor.h"


MotionSensor motionSensor = MotionSensor();

void setup() {
    Serial.begin(115200);
    motionSensor.begin();

}

inline double RadToDeg(double v) { return v*180.0/3.1415926535897932384626433832795; }


void loop() {
    unsigned long rs = millis();
    motionSensor.read();
    unsigned long es = millis();
    Serial.print(es-rs);
    Serial.print(",");
    Serial.print(RadToDeg(motionSensor.orientation.pitch));
    Serial.print(",");
    Serial.print(RadToDeg(motionSensor.orientation.roll));
    Serial.print(",");
    Serial.print(motionSensor.gyro_event.gyro.x);
    Serial.print(",");
    Serial.print(motionSensor.gyro_event.gyro.y);
    Serial.print(",");
    Serial.print(motionSensor.gyro_event.gyro.z);
    Serial.print(",");
    Serial.print(motionSensor.accel_event.acceleration.x);
    Serial.print(",");
    Serial.print(motionSensor.accel_event.acceleration.y);
    Serial.print(",");
    Serial.println(motionSensor.accel_event.acceleration.z);
    delay(500);
}


  
  



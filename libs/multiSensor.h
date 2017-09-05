
#ifndef __MULTISENSOR_H__
#define __MULTISENSOR_H__

#ifndef TEST
#include <N2kMessages.h>
#endif

#include "statistic.h"
#include "motionSensor.h"
#include "configuration.h"

#define PI (double)3.1415926535897932384626433832795

volatile unsigned long wind_period = 0;
// eventHandlerToRecord the time between rising edges.
// Looking at the cuircuit diagram of the WindVane it looks like it is a pull down 
// cuircuit via an opti isplator. It is clamped with 2 diodes to 0-8V - the diode votage
// drop.
void windMonitorReadEvent() {
  static unsigned long period_millis = 0;
  static unsigned long last_period_millis = 0;
  last_period_millis = period_millis;
  period_millis = millis();
  wind_period = period_millis - last_period_millis;
}

volatile unsigned long water_period = 0;
// eventHandlerToRecord the time between rising edges.
void waterMonitorReadEvent() {
  static unsigned long period_millis = 0;
  static unsigned long last_period_millis = 0;
  last_period_millis = period_millis;
  period_millis = millis();
  water_period = period_millis - last_period_millis;
}

class MultiSensor {
public:

 
    MultiSensor(Statistics *statistics,
      MotionSensor *motionSensor,
      bool sensorsConnected = false,
      uint8_t windSinAdc = 0,
      uint8_t windCosAdc = 0,
      uint8_t windSpeedPin = 0,
      uint8_t waterSpeedPin = 0,
      double mastHeight = 17.5,
      double Kfactor = 12,
      double userWindSpeedCalibrationFactor = 1.0F,
      double userWindOffsetRadians = 0.0F,
      double awsHzPerKnot = 1.045F,
      double userWaterSpeedCalibrationFactor = 1.0F,
      double stwHzPerKnot = 5.5F) {
      this->windSinAdc = windSinAdc;
      this->windCosAdc = windCosAdc;
      this->windSpeedPin = windSpeedPin;
      this->waterSpeedPin = windSpeedPin;
      this->mastHeight = mastHeight;
      this->Kfactor = Kfactor;  // needs to be converted from kn^2/degrees in knots^2/radians
      this->statistics = statistics;
      this->motionSensor = motionSensor;
      this->sensorsConnected = sensorsConnected;
      aws = 0; // kn in m/s
      awa = PI; // radians

      pitch = 0;
      roll = 0;
      gyro_x = 0;
      gyro_y = 0;
      measuredWindSpeed = 0; // kn in m/s
      stw = 0; // kn in m/s
      sensorSID = 0;
      measuredWindAngle = PI; // radians
      awsHzPerKn = awsHzPerKnot;
      userCalWindSpeed = userWindSpeedCalibrationFactor;
      userCalWindOffset = userWindOffsetRadians;
      userCalWaterSpeed = userWaterSpeedCalibrationFactor;
      stwHzPerKn = stwHzPerKnot;
      if ( this->windSpeedPin > 0 ) {
        attachInterrupt(digitalPinToInterrupt(this->windSpeedPin),windMonitorReadEvent,RISING);
      }
      if ( this->waterSpeedPin > 0 ) {
        attachInterrupt(digitalPinToInterrupt(this->waterSpeedPin),waterMonitorReadEvent,RISING);        
      }
      attidudeEnabled = false;
      demoMode = false;
    }



    void updateConfiguration(Configuration configuration) {
        sensorsConnected = configuration.getFlag(CONFIG_FLAGS_SENSORS_ENABLED);
        demoMode = configuration.getFlag(CONFIG_FLAGS_DEMO_ENABLED);
        attidudeEnabled = configuration.getFlag(CONFIG_FLAGS_ATTITUDE_ENABLED);
        if (sensorsConnected) {
          attidudeEnabled = true;
        }
        userCalWindSpeed = configuration.getConfig(CONFIG_WIND_SPEED_CAL);
        userCalWindOffset = DegToRad(configuration.getConfig(CONFIG_WIND_OFFSET_CAL));
        userCalWaterSpeed = configuration.getConfig(CONFIG_WATER_SPEED_CAL);
        mastHeight = configuration.getConfig(CONFIG_MAST_HEIGHT);
        Kfactor = configuration.getConfig(CONFIG_KFACTOR);
        awsHzPerKn  = configuration.getConfig(CONFIG_WIND_SPEED_HZ);
        stwHzPerKn = configuration.getConfig(CONFIG_WATER_SPEED_HZ);
    }


    /**
     * Reads from the internal Stats
     */
    bool read() {
        // read the current value of the sensors.
        if (sensorsConnected) {
          readSensors();
          unsigned long tnow = millis();
          applyCorrections();
          statistics->aws.update(aws, tnow);
          statistics->awa.update(awa, tnow);
          statistics->stw.update(stw, tnow);
          statistics->tws.update(tws, tnow);
          statistics->twa.update(twa, tnow);          
          statistics->leeway.update(leeway, tnow);
          statistics->pitch.update(pitch, tnow);
          statistics->roll.update(roll, tnow);
        } else if ( attidudeEnabled ) {
          readAttitude();
          unsigned long tnow = millis();
          stw = statistics->stw.means(5, tnow);
          aws = statistics->aws.means(5, tnow);
          awa = statistics->awa.means(5, tnow);
          tws = statistics->tws.means(5, tnow);
          twa = statistics->twa.means(5, tnow);
          statistics->leeway.update(leeway, tnow);
          statistics->pitch.update(pitch, tnow);
          statistics->roll.update(roll, tnow);
          measuredWindAngle = 0;
          measuredWindSpeed = 0;
          measuredWaterSpeed = 0;
        } else {
          return false;
        }
        sensorSID++;
        LOG(F("Multi Sensor measuredWindAngle:"));
        LOGC(measuredWindAngle);
        LOGC(F(", measuredWindSpeed:"));
        LOGC(measuredWindSpeed);
        LOGC(F(", measuredWaterSpeed:"));
        LOGC(measuredWaterSpeed);
        LOGC(F(", AWS:"));
        LOGC(aws);
        LOGC(F(", AWA:"));
        LOGC(awa);
        LOGC(F(", TWS:"));
        LOGC(tws);
        LOGC(F(", TWA:"));
        LOGC(twa);
        LOGC(F(",  STW:"));
        LOGC(stw);
        LOGC(F(",  leeway:"));
        LOGC(leeway);
        LOGC(F(",  pitch:"));
        LOGC(pitch);
        LOGC(F(",  roll:"));
        LOGC(roll);
        LOGN(F(" "));
        return true;
    }



    void fullStatusMessage(tN2kMsg &N2kMsg) {
      SetN2kWindSpeed(N2kMsg, 
          sensorSID,
          aws,
          awa,
          type
          );
    }
  private:
    double stw; // in m/s.
    double aws; // in m/s.
    double awa; // in radians
    double tws; // in m/s.
    double twa; // in radians
    double mastHeadWindSpeed; // in m/s
    double mastHeadWindAngle; // in radians
    double measuredWindSpeed; // in m/s
    double measuredWindAngle; // in radians
    double measuredWaterSpeed; // in m/s
    double awsHzPerKn; // WindSpeed sensor Hz per kn, derived from sending fake signal to i60 1.045F Hz/kn
    double userCalWindSpeed; // factor to apply to speed before other corrections, 1.0 is no correction.
    double userCalWindOffset; // Angle offset of wind direction in radians.
    double userCalWaterSpeed; // actor to apply to  water speed before other corrections, 1.0 is no correction.
    double mastHeight; // mast height in m
    double Kfactor; // imperical K factor.
    double stwHzPerKn;
    double pitch;
    double roll;
    double gyro_x;
    double gyro_y;
    double leeway;
    double angleOfHeal;
    tN2kWindReference type;
    Statistics *statistics;
    MotionSensor *motionSensor;
    uint8_t windSinAdc;
    uint8_t windCosAdc;
    uint8_t windSpeedPin;
    uint8_t waterSpeedPin;
    uint8_t sensorSID;
    bool sensorsConnected;
    bool attidudeEnabled;
    bool demoMode;

    inline double knotsToms(double v) { return v*1852.0/3600.0; }

    /**
     * return the corrected aparent wind angle in radians.
     */
    void readSensors() {
        readWindSpeed();
        readWaterSpeed();
        readSinCos(); // read the voltages.
        readAttitude();
    }

    void applyCorrections() {

        // all data is aquired, now apply corrections.


        if ( fabs(pitch-(PI/2)) < 0.001 || fabs(roll-(PI/2)) < 0.001 ) {
          // almost 90, should never happen on a boat.
          angleOfHeal = PI/2;
        } else {
          double tanpitch = tan(pitch);
          double tanroll = tan(roll);
          angleOfHeal = atan(sqrt(tanpitch*tanpitch+tanroll*tanroll));
        }


      stw = measuredWindSpeed*userCalWindSpeed;
      // this is a non linear correction from a table.
      stw = correctWaterSpeedForHeal(stw);



        // we read transducer speed and direction refereced the mast
        // correct the speed reading for user correction as this was measured mast vertical and still.
        // correct the angle reading for offsets again as this was static and vertical.
        // correct speed errors due to heal relative to the ideal model (not to horizontal)
        // convert this into x,y,z components referenced to the mast
        // remove angular motion from the x,y,x components.
        // project into global x,y,z co-ordinates.



      offsetAdjustments();

      correctMotion();

      // we now have a corrected wind apparent wind speed and wind angle, next calculate leeway, and adjust
      calcLeeway();
      calcTrueWind();
      correctUpwash();
      correctShear();
    }

    void offsetAdjustments() {
      mastHeadWindSpeed = measuredWindSpeed*userCalWindSpeed;
      mastHeadWindSpeed = correctSpeedForHeal(mastHeadWindSpeed);
      mastHeadWindAngle = correctAngleForUser(measuredWindAngle);
    }

    void correctUpwash() {
      // TODO: Upwash table or calc
    }
    void correctShear() {
      // TODO: Shear table or calc
    }


    void correctMotion() {

      // convert to a wind vector relative to the masthead
      // X being forwards, Y being sideways, matching the 10Dof sensor.
      // these are in m/s
      double measuredX = mastHeadWindSpeed*cos(mastHeadWindAngle);
      double measuredY = mastHeadWindSpeed*sin(mastHeadWindAngle);

      // remove mast tip velocity which is relative to the mast tip.
      double mastX = measuredX + gyro_x * mastHeight;
      double mastY = measuredY + gyro_y * mastHeight;

      // project the masthead co-ordinates to global co-ordinates, taking into 
      // account angle of heal in x and y planes.
      double gX = mastX*cos(pitch);
      double gY = mastY*cos(roll);

      // calculate the global Velocity and angle.
      aws = sqrt(gX*gX+gY*gY);
      if ( aws < 1E-3 ) {
        awa = 0;
      } else {
        awa = atan2(gY/aws, gX/aws);
      }


    }

    double fixAngle(double d) {
        if ( d > PI ) d = d - PI;
        if ( d < -PI) d = d + PI;
        return d;
    }


    // a standard derivation of twa, tws given awa, aws, stw and leeway.
    void calcTrueWind() {

        awa  = fixAngle(awa);
        double stw_lee = stw*cos(leeway);
        double awa_lee = awa;
        if ( awa_lee > 0 ) {
          awa_lee = awa_lee +  leeway;
        } else {
          awa_lee = awa_lee +  leeway;         
        }
        // this should be a noop, but just in case the leeway downwind caused something wierd.
        awa_lee = fixAngle(awa_lee);

        double ctws = sqrt((stw_lee*stw_lee+aws*aws)-(2*stw_lee*aws*cos(awa_lee)));
        double ctwa = 0.0F;
        if ( ctws > 1.0E-3F ) {
            ctwa = (aws*cos(awa_lee)-stw_lee)/ctws;
            if ( ctwa > 0.9999F || ctwa < -0.9999F) {
                ctwa = 0.0F;
            } else {
                ctwa = acos(ctwa);
            }
        }
        if ( awa_lee < 0) {
            ctwa = -ctwa;
        }
        tws = ctws;
        twa = ctwa;
    }

    void calcLeeway() {
      // using the standard formula an alternative is to use a KalmanFilter.
      // see http://robotsforroboticists.com/kalman-filtering/  and http://vm2330.sgvps.net/~syrftest/images/library/20150805142512.pdf
      // Grouprama. 
      // This comes from Pedrick see http://www.sname.org/HigherLogic/System/DownloadDocumentFile.ashx?DocumentFileKey=5d932796-f926-4262-88f4-aaca17789bb0
      // Also in that paper
      // Upwash angle in degees = UK*cos(awa)*cos(3*MstoKn(aws)*PI/180)
      // for aws < 30 and awa < 90. UK  =15 for masthead and 5 for fractional
      if (stw < 1E-3) {
        leeway = 0;
      } else {
        leeway = Kfactor * roll / (stw * stw);
      }
    }


    double correctWaterSpeedForHeal(double v) {
      // this needs to look up the speed relative to angle, taling into account any twist in the sensor due to heal.
      // the correction may also depend on the absolute speed measured.
      return v;
    }

    double correctAngleForUser(double v) {
      // user corrections for angle deal the transducer missalignment.
      // this is an offset.
      return v+userCalWindOffset;
    }


    double correctSpeedForHeal(double v) {
      // correct for angle of heal, this is determined by experiment. Anenomiters have a very non linear 
      // behavior. Ultrasound sensors tend to follow a much more idealised model. The idealised model
      // assumes the annenomiter follows a cosine rule. This function corrects the annenomiter reading
      // to match the cosine rule, first by correcting to the horizontal and then
      // correcting to give the masthead spped in the masthead co-ordinates.From
      // see http://www.dewi.de/dewi/fileadmin/pdf/publications/Publikations/S09_2.pdf, figure 9 A100LM-ME1 
      // which appears to be close to most marine annenomiters, abov
      if ( angleOfHeal > 0.174533) { // > 10 degress + 3%
        return v*1.03;
      } else if ( angleOfHeal > 0.139626) { // >8 degrees +2%
        return v*1.02;
      } else if ( angleOfHeal > 0.10472 ) { // 6 degrees +1%
        return v*1.01;
      }
      // now the speed is corrected for angle of heal reative horizontal.
      // apply the cosine rule to correct for angle of heal.
      return v*cos(angleOfHeal);
    }

    /**
     * The wind sensor puts out 2 voltages between 2 and 6v centered on 4v. One is a cosine of the wind angle, the other
     * other is a sin of the wind angle. The sensor is powered by 8v. Assuming the voltage measurement goes through a voltage divider of 
     * 1K/4K7, there will be a current of 1mA at 6V which should be low enough to reduce the risk of poor quality connections. The resistance
     * should be high enough so that the cable doesnt pick up interference.
     * 1023 == 5v == 5*5.7/4.7 == 6.0638 
     * 1 == (5*5.7/4.7)/1023 == 0.00592749734822 V/bit
     */

#define CONVERT_WIND_TO_SINCOS(x) (((double)x*0.00592749734822F)-4.0F)/2.0F

    void readSinCos() {
      if (demoMode) {
        measuredWindAngle = fixAngle(measuredWindAngle+0.001*((rand()%100)-50));

      } else {
        if ( windSinAdc > 0 && windCosAdc > 0 ) {
          double sinWind = CONVERT_WIND_TO_SINCOS(analogRead(windSinAdc));
          double cosWind = CONVERT_WIND_TO_SINCOS(analogRead(windCosAdc));
          measuredWindAngle = atan2(sinWind, cosWind);                  
        } else {
          measuredWindAngle = 0;
        }
      }
    }

    void readWindSpeed() {
      if (demoMode) {
        measuredWindSpeed = max(0.0F, measuredWindSpeed+0.01*((rand()%100)-50));
      } else {
        LOGLN("Wind monitor is enabled.");
        if ( windSpeedPin > 0 && wind_period > 0) {
          // 1.045Hz per Kn of wind speed.
          // wind_period is the time in ms between pulses. 
          // this is determined using an interrupt handler timing between rising edges.
          // the motion is angular so correct for the motion before correcting for the angle
          // of heal.
          // F = 1/wind_period, 5.5Hz per kn, kn = F/1.045 = 1000/(1.045*wind_period), wind_period is in ms.
          measuredWindSpeed = knotsToms(1000.0F/(awsHzPerKn*(double)wind_period));
        } else {
          // If we have no period, assume 0 wind speed rather than > 900Kn (1000/1.045)
          // but, still correct for motion etc.
          measuredWindSpeed = 0;
        }
      }
    }

    void readWaterSpeed() {
      if (demoMode) {
        measuredWaterSpeed = max(0.0F, measuredWaterSpeed+0.01*((rand()%100)-50));
      } else {
        if ( waterSpeedPin > 0 && water_period > 0) {
          // 5.5Hz per Kn of water speed.
          // water_period is the time in ms between pulses. 
          // this is determined using an interrupt handler timing between rising edges.
          // the motion is angular so correct for the motion before correcting for the angle
          // of heal.
          // F = 1/water_period, 5.5Hz per kn, kn = F/5.5 = 1000/(5.5*water_period), water_period is in ms.
          measuredWaterSpeed = knotsToms(1000.0F/(stwHzPerKn*(double)water_period));
        } else {
          // If we have no period, assume 0 wind speed rather than > 181Kn (1000/5.5)
          // but, still correct for motion etc.
          measuredWaterSpeed = 0;
        }
      }
    }

    void readAttitude() {
      if (demoMode) {
        pitch = min(PI/4,max(-PI/4, pitch+(PI/1000)*((rand()%100)-50)));
        roll = min(PI/4,max(-PI/4, roll+(PI/1000)*((rand()%100)-50)));
        gyro_x = min(PI/8,max(-PI/8, gyro_x+(PI/1000)*((rand()%100)-50)));
        gyro_y = min(PI/8,max(-PI/8, gyro_y+(PI/1000)*((rand()%100)-50)));
      } else {
        motionSensor->read();
        pitch = motionSensor->orientation.pitch;
        roll = motionSensor->orientation.roll;
        gyro_x = motionSensor->gyro_event.gyro.x;
        gyro_y = motionSensor->gyro_event.gyro.y;

      }

    }


};



#endif
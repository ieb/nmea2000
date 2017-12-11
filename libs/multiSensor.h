
#ifndef __MULTISENSOR_H__
#define __MULTISENSOR_H__

#ifndef TEST
#include <N2kMessages.h>
#endif


#include "conversions.h"
#include "configuration.h"
#include "statistic.h"
#include "anglesensor.h"
#include "pulsesensor.h"
#include "imusensor.h"




// eventHandlerToRecord the time between rising edges.
// Looking at the cuircuit diagram of the WindVane it looks like it is a pull down 
// cuircuit via an opti isplator. It is clamped with 2 diodes to 0-8V - the diode votage
// drop.
volatile unsigned long wind_period = 0;
volatile unsigned long wind_edges = 0;
void windPulseHandler() {
  static unsigned long wind_period_micros = 0;
  static unsigned long wind_last_period_micros = 0;
  wind_last_period_micros = wind_period_micros;
  wind_period_micros = micros();
  if ( wind_period_micros > wind_last_period_micros ) {
    wind_period = wind_period_micros - wind_last_period_micros;
  }
  wind_edges++;
}

void readWindMonitor(unsigned long *data) {
  data[0] = wind_edges;
  data[1] = wind_period;
}

// eventHandlerToRecord the time between rising edges.
volatile unsigned long water_period = 0;
volatile unsigned long water_edges = 0;
void waterPulseHandler() {
  static unsigned long water_period_micros = 0;
  static unsigned long water_last_period_micros = 0;
  water_last_period_micros = water_period_micros;
  water_period_micros = micros();
  if ( water_period_micros > water_last_period_micros ) {
    water_period = water_period_micros - water_last_period_micros;
  }
  water_edges++;
}

void readWaterMonitor(unsigned long *data) {
  data[0] = water_edges;
  data[1] = water_period;
}


#define MultiSensor_MESSAGES  128259L, 130306L


class MultiSensor {
public:

 
    MultiSensor(
      Statistics *statistics,
      PulseSensor *windSpeedSensor,
      PulseSensor *waterSpeedSensor,
      AngleSensor *windAngleSensor,
      IMUSensor *imuSensor,
      double kfactor = 12,
      double mastHeight = 17.5) {
      this->kfactor = kfactor;
      this->mastHeight = mastHeight;
      this->statistics = statistics;
      this->windSpeedSensor = windSpeedSensor;
      this->waterSpeedSensor = waterSpeedSensor;
      this->imuSensor = imuSensor;
      this->windAngleSensor = windAngleSensor;
      this->sid = 0;
    }

    bool init() {
      bool ok = windSpeedSensor->init() && ok;
      ok = waterSpeedSensor->init() && ok;
      ok = windAngleSensor->init() && ok;
      return ok;
    }

    void updateConfiguration(Configuration *config) {
      demoMode =  config->getDemoMode();

      mastHeight = config->getMastHeight();
      kfactor = config->getKFactor();
      this->waterSpeedSensor->setDemoMode(demoMode);
      this->windSpeedSensor->setDemoMode(demoMode);
      this->windAngleSensor->setDemoMode(demoMode);
      this->windSpeedSensor->setDamping(config->getWindSpeedDamping());
      this->waterSpeedSensor->setDamping(config->getWaterSpeedDamping());
      this->windAngleSensor->setDamping(config->getWindAngleDamping());
      this->windAngleSensor->calibrate(config->getWindAngleMin(),config->getWindAngleMax(),config->getWindAngleOffset());
      this->windSpeedSensor->calibrate(config->getWindSpeedFrequencies(),config->getWindSpeedPulsePerM(),config->getWindSpeedCalibrations());
      this->waterSpeedSensor->calibrate(config->getWaterSpeedFrequencies(),config->getWaterSpeedPulsePerM(),config->getWaterSpeedCalibrations());       
    }

    void dumpRunstate() {
        DUMP(F("MultiSensor: p,"));
        DUMPC(RadToDeg(pitch));
        DUMPC(F(",r,"));
        DUMPC(RadToDeg(roll));
        DUMPC(F(",gp,"));
        DUMPC(RadToDeg(gpitch));
        DUMPC(F(",gr,"));
        DUMPC(RadToDeg(groll));
        DUMPC(F(",mhs,"));
        DUMPC(msToKnots(mastHeadWindSpeed));
        DUMPC(F(",mha,"));
        DUMPC(RadToDeg(mastHeadWindAngle));
        DUMPC(F(",mws,"));
        DUMPC(msToKnots(measuredWaterSpeed));
        DUMPC(F(",awa,"));
        DUMPC(RadToDeg(awa));
        DUMPC(F(",aws,"));
        DUMPC(msToKnots(aws));
        DUMPC(F(",twa,"));
        DUMPC(RadToDeg(twa));
        DUMPC(F(",tws,"));
        DUMPC(msToKnots(tws));
        DUMPC(F(",stw,"));
        DUMPC(msToKnots(stw));
        DUMPC(F(",aroll,"));
        DUMPC(RadToDeg(averageroll));
        DUMPC(F(",leeway,"));
        DUMPN(RadToDeg(leeway));
        this->windSpeedSensor->dumpstate("Wind Speed:");
        this->waterSpeedSensor->dumpstate("Water Speed:");
        this->windAngleSensor->dumpstate("Wind Angle");
 
    }




    /**
     * Reads from the internal Stats
     * Call at whatever frequency the sensors need to be read.
     */
    bool read() {
      this->windSpeedSensor->read();
      this->waterSpeedSensor->read();
      this->windAngleSensor->read();
      this->imuSensor->read();
      this->tnow = millis();
      return true;
    }

    /**
     * Call at whatever frequency the data needs to be emitted.
     */
    bool calculate() {
      // read all the sensors.
      pitch =  this->imuSensor->getPitch();
      roll = this->imuSensor->getRoll();
      gpitch =  this->imuSensor->getGyroPitch();
      groll = this->imuSensor->getGyroRoll();
      mastHeadWindSpeed = this->windSpeedSensor->getSpeed();
      mastHeadWindAngle = this->windAngleSensor->getAngle();
      measuredWaterSpeed = this->waterSpeedSensor->getSpeed();
      averageroll = averageroll + (roll - averageroll)/30;

      return calculate(pitch, roll, averageroll, gpitch, groll, mastHeadWindSpeed, mastHeadWindAngle, measuredWaterSpeed);
    }



    bool calculate(double pitch, double roll, double averageroll, double gpitch, double groll, double mastHeadWindSpeed, double mastHeadWindAngle, double measuredWaterSpeed) {

      double angleOfHeal = PI/2;
      if ( fabs(pitch-(PI/2)) > 0.001 && fabs(roll-(PI/2)) > 0.001 ) {
        double tanpitch = tan(pitch);
        double tanroll = tan(roll);
        angleOfHeal = atan(sqrt(tanpitch*tanpitch+tanroll*tanroll));
      }
      
      // convert to a wind vector relative to the masthead
      // X being forwards, Y being sideways, matching the 10Dof sensor.
      // these are in m/s
      LOG("mha,");
      LOGC(mastHeadWindAngle);
      LOGC(",mhsd,");
      LOGC(RadToDeg(mastHeadWindAngle));
      LOGC(",mhs,");
      LOGC(mastHeadWindSpeed);
      LOGC(",mhsk,");
      LOGN(msToKnots(mastHeadWindSpeed));
      double measuredX = mastHeadWindSpeed*cos(mastHeadWindAngle);
      double measuredY = mastHeadWindSpeed*sin(mastHeadWindAngle);
      LOG("measured,");
      LOGC(measuredX);
      LOGC(",");
      LOGN(measuredY);

      // remove mast tip velocity which is relative to the mast tip.
      double mastX = measuredX + gpitch * mastHeight;
      double mastY = measuredY + groll * mastHeight;
      double finalMastSpeed = sqrt(mastX*mastX+mastY*mastY);
      LOG("mast,");
      LOGC(mastX);
      LOGC(",");
      LOGC(mastY);
      LOGC(",");
      LOGN(finalMastSpeed);



      // Correct the final velocity for any heel.
      aws = correctWindSpeedForHeal(finalMastSpeed, angleOfHeal);
      awa = 0.0F;
      if ( aws > 1E-3 ) {
        awa = atan2(mastY/aws, mastX/aws);
      }
      stw = correctWaterSpeedForHeal(measuredWaterSpeed, angleOfHeal);

      // using the standard formula an alternative is to use a KalmanFilter.
      // see http://robotsforroboticists.com/kalman-filtering/  and http://vm2330.sgvps.net/~syrftest/images/library/20150805142512.pdf
      // Grouprama. 
      // This comes from Pedrick see http://www.sname.org/HigherLogic/System/DownloadDocumentFile.ashx?DocumentFileKey=5d932796-f926-4262-88f4-aaca17789bb0
      // Also in that paper
      // Upwash angle in degees = UK*cos(awa)*cos(3*MstoKn(aws)*PI/180)
      // for aws < 30 and awa < 90. UK  =15 for masthead and 5 for fractional
      leeway = 0.0F;
      // roll needs a very long term average to be used for leeway.

      if (stw > 0.5) {
        leeway = kfactor * averageroll / (stw * stw);
      }

      // Calculate true wind angle.
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
      twa = ctwa;
      tws = ctws;

      // update the stats.
      this->statistics->awa.update(awa,this->tnow);
      this->statistics->aws.update(aws,this->tnow);
      this->statistics->twa.update(twa,this->tnow);
      this->statistics->tws.update(tws,this->tnow);
      this->statistics->stw.update(stw,this->tnow);
      this->statistics->leeway.update(leeway,this->tnow);
      sid++;
      LOG(F("MiltiSendor pitch,"));
      LOGC(RadToDeg(pitch));
      LOGC(F(",roll,"));
      LOGC(RadToDeg(roll));
      LOGC(F(",gyroP,"));
      LOGC(RadToDeg(gpitch));
      LOGC(F(",gyroR,"));
      LOGC(RadToDeg(groll));
      LOGC(F(",mastHeadWindSpeed,"));
      LOGC(msToKnots(mastHeadWindSpeed));
      LOGC(F(",mastHeadWindAngle,"));
      LOGC(RadToDeg(mastHeadWindAngle));
      LOGC(F(",measuredWaterSpeed,"));
      LOGC(msToKnots(measuredWaterSpeed));
      LOGC(F(",awa,"));
      LOGC(RadToDeg(awa));
      LOGC(F(",aws,"));
      LOGC(msToKnots(aws));
      LOGC(F(",twa,"));
      LOGC(RadToDeg(twa));
      LOGC(F(",tws,"));
      LOGC(msToKnots(tws));
      LOGC(F(",stw,"));
      LOGC(msToKnots(stw));
      LOGC(F(",aroll,"));
      LOGC(RadToDeg(averageroll));
      LOGC(F(",leeway,"));
      LOGN(RadToDeg(leeway));
      return true;
    }
    void fillBoatSpeed(tN2kMsg &N2kMsg) {
        SetN2kBoatSpeed(N2kMsg, sid, stw); //
    }
    void fillAparentWind(tN2kMsg &N2kMsg) {
        SetN2kWindSpeed(N2kMsg, sid, aws, fixAnglePositive(awa), N2kWind_Apprent); //
    }
    void fillTrueWind(tN2kMsg &N2kMsg) {
        SetN2kWindSpeed(N2kMsg, sid, tws, fixAnglePositive(twa), N2kWind_True_boat);
    }

    // Leeway
    // https://www.nmea.org/Assets/20170204%20nmea%202000%20leeway%20pgn%20final.pdf
    // 4x per second.
    void fillLeeway(tN2kMsg &N2kMsg) {
        N2kMsg.SetPGN(128000L);
        N2kMsg.Priority=4;
        N2kMsg.AddByte(sid);
        // int16  leeway/1E-4 in radians fro +pi to -pi +ve means leeway to starboard.
        N2kMsg.Add2ByteInt((int16_t)((double)leeway/(double)1.0E-4F));
        // 40 bits = 5 bytes.
        N2kMsg.AddByte(0xff); // Reserved
        N2kMsg.AddByte(0xff); // Reserved
        N2kMsg.AddByte(0xff); // Reserved
        N2kMsg.AddByte(0xff); // Reserved
        N2kMsg.AddByte(0xff); // Reserved
    }

// 130306 - Wind Data
// 127257 - Attitude
// 130577 - Direction Data
// 60928 - ISO Address Claim
// 127258 - Magnetic Variation
// 128259 - Speed
// 130578 - Vessel Speed Components - perhaps leeway.


  private:
    double stw, leeway, awa, aws, tws, twa;
    double mastHeight; // mast height in m
    double kfactor; // imperical K factor.
    double averageroll; // long term average of roll.
    double pitch, roll, gpitch, groll, mastHeadWindSpeed, mastHeadWindAngle, measuredWaterSpeed;

    unsigned long tnow;
    uint8_t sid;
    Statistics *statistics;
    PulseSensor *windSpeedSensor;
    PulseSensor *waterSpeedSensor;
    IMUSensor *imuSensor;
    AngleSensor *windAngleSensor;
    bool demoMode;





    double correctWaterSpeedForHeal(double v, double angleOfHeal) {
      // this needs to look up the speed relative to angle, taling into account any twist in the sensor due to heal.
      // the correction may also depend on the absolute speed measured.
      return v;
    }


    double correctWindSpeedForHeal(double v, double angleOfHeal) {
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




};



#endif
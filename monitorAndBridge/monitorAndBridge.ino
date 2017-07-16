// Demo: NMEA2000 library. Bus listener and sender. 
//   Sends all bus data to serial in Actisense format.
//   Send all data received from serial in Actisense format to the N2kBus.
//   Use this e.g. with NMEA Simulator (see. http://www.kave.fi/Apps/index.html) to send simulated data to the bus.
// Also reads data from i2c and sends it to both the NMEA2000 network and the actisense network.

#define TRACE(x)
#define DEBUG(x)
#define INFO(x)
#define ERROR(x)


#include <Arduino.h>
#include "configuration.h"

Stream *LogStream =  &SerialUSB;
Stream *DataStream = &Serial;
Configuration configuration = Configuration(LogStream);

#define LOG(x) if (configuration.isLoggingOn()) { LogStream->print(F("LOG:")); LogStream->print(x); }
#define LOGLN(x) if (configuration.isLoggingOn()) { LogStream->print(F("LOG:")); LogStream->println(x); }
#define LOGC(x) if (configuration.isLoggingOn()) { LogStream->print(x); }
#define LOGN(x) if (configuration.isLoggingOn()) { LogStream->println(x); }
#define STATUS(x) if (configuration.isStatusOn()) { LogStream->print(x); }



#include <N2kMsg.h>
#include <NMEA2000.h>
#include <ActisenseReader.h>
#include <N2kMessages.h>
#include <polar.h>
#include <pogo1250.h>
#include <events.h>
#include <batteryMonitor.h>
#include <boatMonitor.h>
#include <waterMonitor.h>
#include <multiSensor.h>
#include <enviroMonitor.h>

#define DEMOMODE true

#define NBATTERIES 2

// Due has a total of 12 ADC pins before multiplexing becomes a requirement.
// 6 ADC pins are needed to fully monitor 2 batteries including temp.
#define BATTERY_1_ADCV 1
#define BATTERY_1_ADCA 2
#define BATTERY_1_ADCT 3
#define BATTERY_2_ADCV 4
#define BATTERY_2_ADCA 5
#define BATTERY_2_ADCT 6
// 2 PINS required for wind direction
#define WIND_SIN_ADC 7
#define WIND_COS_ADC 8
// 1 PIN required for water temp
#define WATER_TEMP_ADC 9

// Pulses from the Wind Speed (Check)
#define WIND_SPEED_PIN 11
// Pulses from the Wind Speed (Check)
#define WATER_SPEED_PIN 12





// ---  Example of using PROGMEM to hold Product ID.  However, doing this will prevent any updating of
//      these details outside of recompiling the program.
const tProductInformation MonitorProductInformation PROGMEM={
                                       1300,                        // N2kVersion
                                       100,                         // Manufacturer's product code
                                       "Ian Boston NMEA ECU",    // Manufacturer's Model ID
                                       "1.0.0.13 (2016-09-19)",     // Manufacturer's Software version code
                                       "1.0.0.0 (2015-08-03)",      // Manufacturer's Model version
                                       "00000001",                  // Manufacturer's Model serial code
                                       0,                           // SertificationLevel
                                       1                            // LoadEquivalency
                                      };                                      

// ---  Example of using PROGMEM to hold Configuration information.  However, doing this will prevent any updating of
//      these details outside of recompiling the program.
const tNMEA2000::tProgmemConfigurationInformation MonitorConfigurationInformation PROGMEM={
                                       "Ian Boston, ianboston@gmail.com", // Manufacturer information
                                       "NMEA2000 to Actisense and 10DOF Monitor", // Installation description1
                                       "Accel, pressure, temp, mag monitor" // Installation description2
                                      };

// Sample for due only. Not tested with Mega
#include <due_can.h>
#include <NMEA2000_due.h>

tNMEA2000_due NMEA2000;
tActisenseReader ActisenseReader;

Adafruit_BMP085_Unified       bmp   = Adafruit_BMP085_Unified(18001);
bool bmp_available;

TimedEventQueue timedEventQueue = TimedEventQueue();
EventHandler sendN2kBatteryHandler = EventHandler(&SendN2kBattery);
EventHandler sendN2KEnviroHandler = EventHandler(&SendN2KEnviro);
EventHandler sendN2KpolarHandler = EventHandler(&SendN2KPolar);
EventHandler multiSensorHandler = EventHandler(&RunMultiSensor);

EnviroMonitor enviroMonitor = EnviroMonitor();
// initialise the polar chart for the boat, defined in pogo1250.h
Polar_Performance polarPerformance = Polar_Performance(POGO1250_NAME,
        POGO1250_N_TWA, 
        POGO1250_N_TWS,
        pogo1250Data_twa,
        pogo1250Data_tws,
        pogo1250Data_bsp);

// statistics collect data from monitors.
Statistics statistics = Statistics();



// The Boat monitor calcultes derived values and makes the data available to the system.
BoatMonitor boatMonitor = BoatMonitor(&polarPerformance, 
        &statistics);
/*
 MultiSensor(Statistics *statistics,
      MotionSensor *motionSensor,
      uint8_t windSinAdc,
      uint8_t windCosAdc,
      uint8_t windSpeedPin,
      uint8_t waterSpeedPin,
      double mastHeight = 17.5,
      double Kfactor = 12,
      double userSpeedCalibrationFactor = 1.0F,
      double userWindOffsetRadians = 0.0F,
      double awsHzPerKnot = 1.045F,
      double userWaterSpeedCalibrationFactor = 1.0F,
      double stwHzPerKnot = 5.5F) {
 */ 
MotionSensor motionSensor = MotionSensor();
MultiSensor multiSensor = MultiSensor(&statistics, 
        &motionSensor,
        WIND_SIN_ADC,
        WIND_COS_ADC,
        WIND_SPEED_PIN,
        WATER_SPEED_PIN);
WaterMonitor waterMonitor = WaterMonitor(WATER_TEMP_ADC);



/**
 * BatteryMonitor(int dcInstance, int batCapacity=100,
            int voltageADCPin = 0,
            double convertADCToVolts = 0.0147F, // 3:1 resistor divider, 5V = 15V
            double zeroVOffset = 0.0F, // 3:1 resistor divider, 5V = 15V
            int currentADCPin = 0,
            double convertADCToAmps = 0.196F, // 0V = -100A, 2.5V = 0A, 5V = 100A  
            double zeroAOffset = 100.0F, // 5V = 100A,  
            int temperatureADCPin = 0,
            double convertADCToC = 0.098F, // 0V = 0C, 5V = 100C  
            double zeroCOffset = 0.0F, // 
            tN2kBatNomVolt batNominalVoltage=N2kDCbnv_12v, 
            tN2kBatChem batChemistry=N2kDCbc_LeadAcid,
            tN2kBatType  batType=N2kDCbt_Flooded ) {
  */
BatteryMonitor batteryBank[NBATTERIES] = {
  BatteryMonitor(0, 100, BATTERY_1_ADCV, VOLTAGE_DIVIDER_3TO1, 0.0F, 
                         BATTERY_1_ADCA, ACS758_100BA_ADC_TO_AMPS, ACS758_100BA_ZEROA_OFFSET, 
                         BATTERY_1_ADCT, TEMP_5V100C, TEMP_5V100C_OFFSET, 
                         N2kDCbnv_12v, N2kDCbc_LeadAcid,N2kDCbt_Flooded ),
  BatteryMonitor(1, 300, BATTERY_2_ADCV, VOLTAGE_DIVIDER_3TO1, 0.0F, 
                         BATTERY_2_ADCA, ACS758_100BA_ADC_TO_AMPS, ACS758_100BA_ZEROA_OFFSET, 
                         BATTERY_2_ADCT, TEMP_5V100C, TEMP_5V100C_OFFSET, 
                         N2kDCbnv_12v, N2kDCbc_LeadAcid,N2kDCbt_Flooded )
};
const unsigned long TransmitMessages[] PROGMEM={
  EnviroMonitor_MESSAGES,
  BatteryMonitor_MESSAGES,
  BoatMonitor_MESSAGES,
  WaterMonitor_MESSAGES,
  0};



void setup() {
  Serial.begin(115200);
  SerialUSB.begin(115200);

  enviroMonitor.begin();
  for (int i = 0; i < NBATTERIES; i++ ) {
    batteryBank[i].setMode(MONITOR_MODE_DEMO);
    batteryBank[i].begin();
  }

  enviroMonitor.setMode(MONITOR_MODE_ENABLED);
  waterMonitor.setMode(MONITOR_MODE_DEMO);
  multiSensor.setMode(MONITOR_MODE_DEMO);



  timedEventQueue.addHandler(&sendN2kBatteryHandler);
  timedEventQueue.addHandler(&sendN2KEnviroHandler);
  timedEventQueue.addHandler(&sendN2KpolarHandler);
  timedEventQueue.addHandler(&multiSensorHandler);

  configuration.init();
  updateConfig();
  
  
  NMEA2000.SetProductInformation(&MonitorProductInformation );
  NMEA2000.SetProgmemConfigurationInformation(&MonitorConfigurationInformation );
  NMEA2000.SetDeviceInformation(1,      // Unique number. Use e.g. Serial number.
                                132,    // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25,     // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046    // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );
  NMEA2000.SetForwardStream(DataStream);  // PC output on due Native USB port, the programming port is used for diag and programming.
  // make it listen and be a node, then enable forwarding to make it forward to actisense.
  // N2km_ListenOnly
  // NMEA2000.SetMode(tNMEA2000::N2km_ListenAndSend);
  // ListenAndNode makes the device listen to inbound messages and act as a device announcing itself to the bus.
  // Provided the trancever is working correctly in this mode the device will appear on a Raymarine e7 when scanning the bus for other devices.
  //
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode,22);
  //NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly,22);
  // Set to true to forward messages from the N2KBus to serial.
  NMEA2000.EnableForward(true);
  NMEA2000.SetForwardSystemMessages(true);
  NMEA2000.SetForwardOwnMessages(true);
  
  // NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text
  
  NMEA2000.SetMsgHandler(&HandleNMEAMessage);
  NMEA2000.Open();
  

  

}


void HandleNMEAMessage(const tN2kMsg &N2kMsg) {
  // for the PGNs we are interested in update the statistics.
  unsigned long tnow = millis();
  switch(N2kMsg.PGN) {
    case 130306L: // WindSpeed
      {
        unsigned char SID;
        double WindSpeed;
        double WindAngle;
        tN2kWindReference WindReference;
        ParseN2kWindSpeed(N2kMsg, SID, WindSpeed, WindAngle, WindReference);
        LOG(F("Got Apparent Wind Speed "));
        if (WindReference == N2kWind_Apprent ) {
          // change at some point
          if ( WindSpeed != N2kDoubleNA ) {
            statistics.aws.update(msToKnots(WindSpeed), tnow);          
            LOGC(msToKnots(WindSpeed));
            LOGC(F(" Kn "));
          }
          if ( WindAngle != N2kDoubleNA ) {
            statistics.awa.update(RadToDeg(WindAngle), tnow);
            LOGC(RadToDeg(WindAngle));
            LOGC(F(" degrees  "));
          }
          LOGN(F(" "));
        }
      }
    break;
    case 128259L: // Boat speed.
      {
        LOG(F("Got Boat Speed "));
        unsigned char SID;
        double WaterRefereced; 
        double GroundReferenced; 
        tN2kSpeedWaterReferenceType SWRT;
        ParseN2kBoatSpeed(N2kMsg, SID, WaterRefereced, GroundReferenced,  SWRT);
        if ( WaterRefereced != N2kDoubleNA ) {
          statistics.stw.update(msToKnots(WaterRefereced), tnow);
          LOGC(msToKnots(WaterRefereced));
          LOGC(F(" kn  "));
        }
        LOGN(F(" "));
      }
    break;
  }
}



void loop() {
  NMEA2000.ParseMessages();
  if ( configuration.read() ) {
    updateConfig();
  }
  timedEventQueue.tick(millis());
}

void updateConfig() {
    multiSensor.updateConfiguration(configuration);
}




#define BatteryUpdatePeriod 5000
#define EnviroUpdatePeriod 10000
#define PolarUpdatePeriod 1000

unsigned long RunMultiSensor(unsigned long now) {
  multiSensor.read();
  return now+100;
}

unsigned long SendN2kBattery(unsigned long now) {
  static uint8_t ncalls = 0;
  ncalls = ncalls+1;

    tN2kMsg N2kMsg;
    for (int i = 0; i < NBATTERIES; i++ ) {
     double diff = 0.4F*i;
      batteryBank[i].read();
      batteryBank[i].fillStatusMessage(N2kMsg);
       NMEA2000.SendMsg(N2kMsg);
      if ( ncalls%16 == 0 ) {
        batteryBank[i].fillChargeStatusMessage(N2kMsg);
        NMEA2000.SendMsg(N2kMsg);
      }
      if ( ncalls%30 == 0 ) {
        batteryBank[i].fillBatteryConfigurationMessage(N2kMsg);
        NMEA2000.SendMsg(N2kMsg);        
      }
    }
    return millis() + BatteryUpdatePeriod;
}

unsigned long  SendN2KEnviro(unsigned long now) {
  if(enviroMonitor.read()) {
    LOGLN(F("Sending environment"));
    tN2kMsg N2kMsg;
    enviroMonitor.fillStatusMessage(N2kMsg);
    NMEA2000.SendMsg(N2kMsg);
  }
  return millis() + EnviroUpdatePeriod;
}



unsigned long SendN2KPolar(unsigned long now) {
  LOGLN(F("Polar Performance"));
  boatMonitor.read(now);
  tN2kMsg N2kMsg;
  boatMonitor.fillPolarPerformance(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  boatMonitor.fillTargetBoatSpeed(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  boatMonitor.fillBoatSpeed(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  boatMonitor.fillAparentWind(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  boatMonitor.fillTrueWind(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  return millis() + PolarUpdatePeriod;  
}



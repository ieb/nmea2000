// Demo: NMEA2000 library. Bus listener and sender. 
//   Sends all bus data to serial in Actisense format.
//   Send all data received from serial in Actisense format to the N2kBus.
//   Use this e.g. with NMEA Simulator (see. http://www.kave.fi/Apps/index.html) to send simulated data to the bus.
// Also reads data from i2c and sends it to both the NMEA2000 network and the actisense network.

#include <Arduino.h>
#include <N2kMsg.h>
#include <NMEA2000.h>
#include <ActisenseReader.h>
#include <N2kMessages.h>
#include "polar.h"
#include "pogo1250.h"
#include "events.h"
#include "batteryMonitor.h"
#include "enviroMonitor.h"

inline double knToms(double v) { return v*1852.0/3600.0; }

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
EventHandler sendN2KFakeBoatHandler = EventHandler(&SendFakeBoatData);


EnviroMonitor enviroMonitor = EnviroMonitor();
// initialise the polar chart for the boat, defined in pogo1250.h
Polar_Performance polarPerformance = Polar_Performance(POGO1250_NAME,
        POGO1250_N_TWA, 
        POGO1250_N_TWS,
        pogo1250Data_twa,
        pogo1250Data_tws,
        pogo1250Data_bsp);
Statistic awaStatistic = Statistic();
Statistic awsStatistic = Statistic();
Statistic bspStatistic = Statistic();
PolarMonitor polarMonitor = PolarMonitor(&polarPerformance, 
        &awaStatistic, 
        &awsStatistic, 
        &bspStatistic);

#define NBATTERIES 2

// 6 ADC pins are needed to fully monitor 2 batteries including temp.
#define BATTERY_1_ADCV 1
#define BATTERY_1_ADCA 2
#define BATTERY_1_ADCT 3
#define BATTERY_2_ADCV 4
#define BATTERY_2_ADCA 5
#define BATTERY_2_ADCT 6


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
  0};

void setup() {
  SerialUSB.begin(115200);
  Serial.begin(115200);

  enviroMonitor.begin();
  for (int i = 0; i < NBATTERIES; i++ ) {
    batteryBank[i].begin();
  }

  timedEventQueue.addHandler(&sendN2kBatteryHandler);
  timedEventQueue.addHandler(&sendN2KEnviroHandler);
  timedEventQueue.addHandler(&sendN2KpolarHandler);
  timedEventQueue.addHandler(&sendN2KFakeBoatHandler);

  
  
  NMEA2000.SetProductInformation(&MonitorProductInformation );
  NMEA2000.SetProgmemConfigurationInformation(&MonitorConfigurationInformation );
  NMEA2000.SetDeviceInformation(1,      // Unique number. Use e.g. Serial number.
                                132,    // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25,     // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046    // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );
  NMEA2000.SetForwardStream(&Serial);  // PC output on due programming port
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
  
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text
  
  NMEA2000.SetMsgHandler(&HandleNMEAMessage);
  NMEA2000.Open();
  

  

  // I had problem to use same Serial stream for reading and sending.
  // It worked for a while, but then stopped.
  ActisenseReader.SetReadStream(&SerialUSB);
  ActisenseReader.SetMsgHandler(HandleStreamN2kMsg); 
}

void HandleStreamN2kMsg(const tN2kMsg &N2kMsg) {
  // N2kMsg.Print(&Serial);
  NMEA2000.SendMsg(N2kMsg,-1);
}

void HandleNMEAMessage(const tN2kMsg &N2kMsg) {
  // for the PGNs we are interested in update the statistics.
  unsigned long tnow = millis();
  switch(N2kMsg.PGN) {
    case 130306L: // WindSpeed
      {
        Serial.println("Got Wind Speed");
        unsigned char SID;
        double WindSpeed;
        double WindAngle;
        tN2kWindReference WindReference;
        ParseN2kWindSpeed(N2kMsg, SID, WindSpeed, WindAngle, WindReference);
        if (WindReference == N2kWind_Apprent ) {
          // change at some point
          if ( WindSpeed != N2kDoubleNA ) {
            awsStatistic.update(msToKnots(WindSpeed), tnow);          
          }
          if ( WindAngle != N2kDoubleNA ) {
            awaStatistic.update(RadToDeg(WindAngle), tnow);
          }
        }
      }
    break;
    case 128259L: // Boat speed.
      {
        Serial.println("Got Boat Speed");
        unsigned char SID;
        double WaterRefereced; 
        double GroundReferenced; 
        tN2kSpeedWaterReferenceType SWRT;
        ParseN2kBoatSpeed(N2kMsg, SID, WaterRefereced, GroundReferenced,  SWRT);
        if ( WaterRefereced != N2kDoubleNA ) {
          bspStatistic.update(msToKnots(WaterRefereced), tnow);
        }
      }
    break;
  }
}

void loop() {
  NMEA2000.ParseMessages();
  ActisenseReader.ParseMessages();
  timedEventQueue.tick(millis());
 
}





#define BatteryUpdatePeriod 1000
#define EnviroUpdatePeriod 10000
#define PolarUpdatePeriod 1000
#define BoatDatUpdatePeriod 1000

unsigned long SendN2kBattery(unsigned long now) {
  static uint8_t ncalls = 0;
  ncalls = ncalls+1;

    tN2kMsg N2kMsg;
    for (int i = 0; i < NBATTERIES; i++ ) {
     double diff = 0.4F*i;
      batteryBank[i].setFake(12.8F+diff,1.12F+diff,17.9F+diff);
      batteryBank[i].read();
      batteryBank[i].fillStatusMessage(N2kMsg);
       Serial.print(i);
       Serial.println(":Sending Voltage Status ");
       NMEA2000.SendMsg(N2kMsg);
      if ( ncalls%16 == 0 ) {
        batteryBank[i].fillChargeStatusMessage(N2kMsg);
       Serial.print(i);
        Serial.println(":Sending Charge Status");
        NMEA2000.SendMsg(N2kMsg);
      }
      if ( ncalls%30 == 0 ) {
        batteryBank[i].fillBatteryConfigurationMessage(N2kMsg);
       Serial.print(i);
        Serial.println(":Sending Config Status");
        NMEA2000.SendMsg(N2kMsg);        
      }
    }
    return millis() + BatteryUpdatePeriod;
}

unsigned long  SendN2KEnviro(unsigned long now) {
  if(enviroMonitor.read()) {
    tN2kMsg N2kMsg;
    enviroMonitor.fillStatusMessage(N2kMsg);
    NMEA2000.SendMsg(N2kMsg);
    Serial.println("Sent");
  }
  return millis() + EnviroUpdatePeriod;
}

unsigned long SendFakeBoatData(unsigned long now) {
  tN2kMsg N2kMsg;
  static unsigned char sid=0;
  Serial.println("Sending Fake Boat Data");
  SetN2kBoatSpeed(N2kMsg, sid++, knToms(8.9F));
  NMEA2000.SendMsg(N2kMsg);

  SetN2kWaterDepth(N2kMsg, sid++, 7.5F, 0.3F);
  NMEA2000.SendMsg(N2kMsg);

  SetN2kWindSpeed(N2kMsg, sid++, knToms(22.5F), DegToRad(33.0F), N2kWind_Apprent);
  NMEA2000.SendMsg(N2kMsg);

  SetN2kTemperature(N2kMsg, sid++, 1, N2kts_SeaTemperature,
                     CToKelvin(12.3));
  NMEA2000.SendMsg(N2kMsg);


  return millis() + BoatDatUpdatePeriod;  

}


unsigned long SendN2KPolar(unsigned long now) {
  polarMonitor.read(now);
  tN2kMsg N2kMsg;
  Serial.println("Polar Performance");
  polarMonitor.fillPolarPerformance(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  polarMonitor.fillTargetBoatSpeed(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  return millis() + PolarUpdatePeriod;  
}



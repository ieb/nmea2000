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
#include <conversions.h>
#include <demo.h>

Stream *LogStream =  &Serial;
Stream *DataStream = &SerialUSB;

#define DUMP(x) if (Serial) { LogStream->print(F("D:")); LogStream->print(x); }
#define DUMPLN(x) if (Serial) { LogStream->print(F("D:")); LogStream->println(x); }
#define DUMPC(x) if (Serial) { LogStream->print(x); }
#define DUMPN(x) if (Serial) { LogStream->println(x); }
#define INFO(x) if (Serial) { LogStream->print(F("I:")); LogStream->print(x); }
#define INFOLN(x) if (Serial) { LogStream->print(F("I:")); LogStream->println(x); }
#define INFOC(x) if (Serial) { LogStream->print(x); }
#define INFOC2(x,m) if (Serial) { LogStream->print(x,m); }
#define INFON(x) if (Serial) { LogStream->println(x); }
#define INFON2(x,m) if (Serial) { LogStream->println(x,m); }

// CS for the SD Card
#define SDCARD_CS 7

#include <configuration.h>

demo_data_t demoData;


Configuration configuration = Configuration(LogStream, &demoData);

#define LOG(x) if (Serial && configuration.isLoggingOn()) { LogStream->print(F("L:")); LogStream->print(x); }
#define LOGLN(x) if (Serial && configuration.isLoggingOn()) { LogStream->print(F("L:")); LogStream->println(x); }
#define LOGC(x) if (Serial && configuration.isLoggingOn()) { LogStream->print(x); }
#define LOGN(x) if (Serial && configuration.isLoggingOn()) { LogStream->println(x); }
#define LOGC2(x,m) if (Serial && configuration.isLoggingOn()) { LogStream->print(x, m); }
#define LOGN2(x,m) if (Serial && configuration.isLoggingOn()) { LogStream->println(x, m); }
#define STATUS(x) if (Serial && configuration.isStatusOn()) { LogStream->print(x); }


#include <N2kMsg.h>
#include <NMEA2000_CAN.h>
#include <N2kMessages.h>
#include <polar.h>
#include <pogo1250.h>
#include <events.h>
#include <boatMonitor.h>
#include <waterMonitor.h>
#include <multiSensor.h>
#include <freemem.h>



// 2 PINS required for wind direction
#define WIND_SIN_ADC A0
#define WIND_COS_ADC A1
// 1 PIN required for water temp
#define WATER_TEMP_ADC A2

// Pulses from the Wind Speed (Check)
#define WIND_SPEED_PIN 5
// Pulses from the Wind Speed (Check)
#define WATER_SPEED_PIN 6






// ---  Example of using PROGMEM to hold Product ID.  However, doing this will prevent any updating of
//      these details outside of recompiling the program.
const tNMEA2000::tProductInformation MonitorProductInformation PROGMEM={
                                       1300,                        // N2kVersion
                                       200,                         // Manufacturer's product code
                                       "Boston NMEA2000 Performance Pod",    // Manufacturer's Model ID
                                       "1.0.0.13 (2016-09-19)",     // Manufacturer's Software version code
                                       "1.0.0.0 (2015-08-03)",      // Manufacturer's Model version
                                       "00000001",                  // Manufacturer's Model serial code
                                       0,                           // SertificationLevel
                                       1                            // LoadEquivalency
                                      };                                      

// ---  Example of using PROGMEM to hold Configuration information.  However, doing this will prevent any updating of
//      these details outside of recompiling the program.
const char ProdInfo1[] PROGMEM = "Ian Boston, ianboston@gmail.com"; // Manufacturer information
const char ProdInfo2[] PROGMEM = "Performance Pod"; // Installation description1
const char ProdInfo3[] PROGMEM = "Wind, Water, Motion, Environment, Polars"; // Installation description2




// initialise the polar chart for the boat, defined in pogo1250.h
Polar_Performance polarPerformance = Polar_Performance(POGO1250_NAME,
        POGO1250_N_TWA, 
        POGO1250_N_TWS,
        (uint16_t *)pogo1250Data_twa,
        (uint16_t *)pogo1250Data_tws,
        (uint16_t *)pogo1250Data_bsp);

// statistics collect data from monitors.
Statistics statistics = Statistics();



// The Boat monitor calcultes derived values and makes the data available to the system.
BoatMonitor boatMonitor = BoatMonitor(&polarPerformance, 
        &statistics);


/*******************************************************************

Reads pulses from both water and wind, and reads sin/cos from wind.

MultiSensor
 Statistics *statistics,
      PulseSensor *windSpeedSensor,
      PulseSensor *waterSpeedSensor,
      AngleSensor *windAngleSensor,
      IMUSensor *imuSensor,
      double Kfactor = 12,
      double mastHeight = 17.5) {
 *****************************************************************/


AngleSensor windAngleSensor = AngleSensor(WIND_SIN_ADC, WIND_COS_ADC, &demoData);
PulseSensor windSpeedSensor = PulseSensor(knotsToMs(1.045F), &readWindMonitor, &windPulseHandler, WIND_SPEED_PIN, &(demoData.windSpeed));
PulseSensor waterSpeedSensor = PulseSensor(knotsToMs(5.5F), &readWaterMonitor, &waterPulseHandler, WATER_SPEED_PIN, &(demoData.waterSpeed));
Adafruit_BNO055 bno = Adafruit_BNO055(55);
IMUSensor imuSensor = IMUSensor(&bno, &demoData);

MultiSensor multiSensor = MultiSensor( &statistics, 
        &windSpeedSensor,
        &waterSpeedSensor,
        &windAngleSensor,
        &imuSensor);


/****************************************************************

Reads a voltage representing temperature from a thermistor housed 
in the Airmar speed sensor.

*****************************************************************/

WaterMonitor waterMonitor = WaterMonitor(WATER_TEMP_ADC, &demoData);



/*********************************************************

Collectr together the messages 

**********************************************************/

const unsigned long TransmitMessages[] PROGMEM={
  MultiSensor_MESSAGES,
  BoatMonitor_MESSAGES,
  WaterMonitor_MESSAGES,
  0};



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  
  SerialUSB.begin(115200);


  populateEventQueue();
  configuration.init();
  configuration.disableLogging();
  configuration.testPatterns();   
  configuration.disableLogging();
  updateConfig();
  polarPerformance.init();
  imuSensor.init();
  multiSensor.init();
  boatMonitor.init();
  waterMonitor.init();
  initCanBus();
}




void loop() {
  // process NMEA2000 events.
  NMEA2000.ParseMessages();

  // process configuration events.
 uint8_t r = configuration.read();
 switch(r) {
  case CONFIG_UPDATE_RUNTIME:
    updateConfig();
    break;
  case CONFIG_CURRENT_STATUS:
    dumpRunstate();
    break;
  case CONFIG_IMU_CALIBRATION:
    calibrateImu();
    break;
  }

  // process timed events.
  tickTock();
}

/**********************************************************

Initialse Can0

***********************************************************/
void initCanBus() {

  NMEA2000.SetProductInformation(&MonitorProductInformation );
  NMEA2000.SetProgmemConfigurationInformation(ProdInfo1,ProdInfo2,ProdInfo3);
  NMEA2000.SetDeviceInformation(1,      // Unique number. Use e.g. Serial number.
                                132,    // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25,     // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2048    // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );

// class:60, function: 155, manufacture: 135 is Airmar speed sensor.
// class:25, function 132, manufacturer: 2048 is custom


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
        int Index=0, nparams=0;
  switch(N2kMsg.PGN) {
    case 128259: // boat speed
      break;
    case 128267: // depth transducer
      break;
    case 130310: // env params
      break;
    case 128275: // distance
      break;
    case 130846: // set alarm state
      break;
    case 59904: // ISO Request(
      unsigned long RequestedPGN;
      ParseN2kPGNISORequest(N2kMsg, RequestedPGN);
      LOG(F("ISO Requested PGN "));LOGN(RequestedPGN);
      // 126996
      break;
    case 61184: // transducer setup
    case 65368: // ??
      break;
    case 126208: // NMEA Group Request FN
         LOG(F("Group FN Request  DataLen "));
         LOGC(N2kMsg.DataLen);
         LOGC(F(",FNCOde,"));
         LOGC(N2kMsg.GetByte(Index));
         LOGC(F(",Requested PGN,"));
         char b[3];
         uint32_t v;
         b[0] = N2kMsg.GetByte(Index);
         b[1] = N2kMsg.GetByte(Index);
         b[2] = N2kMsg.GetByte(Index);
         memcpy(&v, &b[0], 3);
         LOGC(v);
         LOGC(F(",Transmission Interval,"));
         LOGC(N2kMsg.Get4ByteUInt(Index));
         LOGC(F(",Transmission Interval Offset,"));
         LOGC(N2kMsg.Get2ByteUInt(Index));
         LOGC(F(",Number of Parameters,"));
         nparams = N2kMsg.GetByte(Index);
         LOGN(nparams);
         // 5 bytes left   2 and 1
         // LOG:Group FN Request  DataLen LOG:16LOG:,FNCOde,LOG:0LOG:,Requested PGN,LOG:130846LOG:,Transmission Interval,LOG:4294967295LOG:,Transmission Interval Offset,LOG:65535LOG:,Number of Parameters,2


          
          // AirMar emit 065285


//      OG:Got Message:LOG:60928
//LOG:ISO Requested PGN 60928 // address claim
//LOG:ISO Requested PGN 126996 // product info
//LOG:Got Message:LOG:127258 // magnetic variation
//LOG:Got Message:LOG:130919 // unknown
//LOG:Got Message:LOG:130919
//LOG:ISO Requested PGN 126996 // product info


      break;
    default:
      LOG(F("Got Message:"));
      LOGLN(N2kMsg.PGN);
  }

  // nothing done with inbound messages for the moment.
  // may want to process configuration messages.
}

/**
 * Airmar
 * 128267
 */

void calibrateImu() {
  if ( imuSensor.readConfiguration(&configuration) ) {
    INFOLN("IMU Sensor Calibration retrieved");
  } else {
    INFOLN("IMU Sensor Calibration not complete.");
  }
}

void updateConfig() {
  imuSensor.updateConfiguration(&configuration);
  multiSensor.updateConfiguration(&configuration);
  boatMonitor.updateConfiguration(&configuration);
  waterMonitor.updateConfiguration(&configuration);
}

void dumpRunstate() {
  imuSensor.dumpRunstate();
  multiSensor.dumpRunstate();
  boatMonitor.dumpRunstate();
  waterMonitor.dumpRunstate();
}


/****************************************************

Timed events. 
Handlers maintain a slot in a queue specifying when
their next event should be fired, each time their
handler is filred, the handler specifies the next time.
The loop calls ticktock() which processes the queue.

*****************************************************/

#define EnviroUpdatePeriod 10000
#define PolarUpdatePeriod 5000


#define MULTISENSOR_READ_PERIOD 200
#define MULTISENSOR_CALC_PERIOD 500

// Sense at 10Hz
unsigned long RunMultiSensor(unsigned long now) {
  multiSensor.read();
  return now+MULTISENSOR_READ_PERIOD;
}

// Calculate and send data at 5Hz
unsigned long RunMultiSensorCalc(unsigned long now) {
  multiSensor.calculate();
  tN2kMsg N2kMsg;
  multiSensor.fillBoatSpeed(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  multiSensor.fillAparentWind(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  multiSensor.fillTrueWind(N2kMsg);
  NMEA2000.SendMsg(N2kMsg);
  return now+MULTISENSOR_CALC_PERIOD;
}


unsigned long  SendN2KEnviro(unsigned long now) {
  if(waterMonitor.read()) {
    LOGLN(F("Sending waterm temp"));
    tN2kMsg N2kMsg;
    waterMonitor.fillWaterTemperature(N2kMsg);
    NMEA2000.SendMsg(N2kMsg);
   }
  return millis() + EnviroUpdatePeriod;
}



unsigned long SendN2KPolar(unsigned long now) {
  if (boatMonitor.read(now) ) {
    LOGLN(F("Polar Performance"));
    tN2kMsg N2kMsg;
    boatMonitor.fillPolarPerformance(N2kMsg);
    NMEA2000.SendMsg(N2kMsg);
    boatMonitor.fillTargetBoatSpeed(N2kMsg);
    NMEA2000.SendMsg(N2kMsg);
  }
  return millis() + PolarUpdatePeriod;  
}

unsigned long LogLine(unsigned long now) {
  static uint32_t fm = 0;
  static bool blink = false;
  blink = !blink;
  digitalWrite(LED_BUILTIN, blink?HIGH:LOW);


  meminfo_t info;
  if ( freeMemory(&info) != fm ) {
    INFO(F("FreeMemory now ")); 
    INFON(info.free);
    fm = info.free; 
  } 
  if ( fm < 20000 ) {
    INFO(F("<<<<<<<<<<<<<<<<< WARNING LOW MEMORY ")); 
    INFON(info.free);
  }
  return millis() + 1000L;
}

TimedEventQueue timedEventQueue = TimedEventQueue();
EventHandler sendN2KEnviroHandler = EventHandler(&SendN2KEnviro);
EventHandler sendN2KpolarHandler = EventHandler(&SendN2KPolar);
EventHandler multiSensorHandler = EventHandler(&RunMultiSensor);
EventHandler logHandler = EventHandler(&LogLine);
EventHandler sendMultisensor = EventHandler(&RunMultiSensorCalc);

void populateEventQueue() {
  
  timedEventQueue.addHandler(&sendN2KEnviroHandler);
  timedEventQueue.addHandler(&sendN2KpolarHandler);
  timedEventQueue.addHandler(&multiSensorHandler);
  timedEventQueue.addHandler(&logHandler);
  timedEventQueue.addHandler(&sendMultisensor);

}

void tickTock() {
  timedEventQueue.tick(millis());
}



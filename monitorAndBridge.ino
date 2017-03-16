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

void setup() {
  SerialUSB.begin(115200);
  Serial.begin(115200);
  NMEA2000.SetProductInformation(&MonitorProductInformation );
  NMEA2000.SetProgmemConfigurationInformation(&MonitorConfigurationInformation );
  NMEA2000.SetDeviceInformation(1,      // Unique number. Use e.g. Serial number.
                                25,    // Device function=Battery. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                130,     // Device class=Electrical Generation. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046    // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );
  NMEA2000.SetForwardStream(&Serial);  // PC output on due programming port
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndSend);
//  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text
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

void loop() {
  NMEA2000.ParseMessages();
  ActisenseReader.ParseMessages();
  SendN2kBattery();
}

#define BatUpdatePeriod 1000


void SendN2kBattery() {
  static unsigned long TempUpdated=millis();
  tN2kMsg N2kMsg;

  if ( TempUpdated+BatUpdatePeriod<millis() ) {
    TempUpdated=millis();
    /*****************************************************************************
    // Battery Status
    // Input:
    //  - BatteryInstance       BatteryInstance.
    //  - BatteryVoltage        Battery voltage in V
    //  - BatteryCurrent        Current in A 
    //  - BatteryTemperature    Battery temperature in °K. Use function CToKelvin, if you want to use °C.
    //  - SID                   Sequence ID.
    void SetN2kPGN127508(tN2kMsg &N2kMsg, unsigned char BatteryInstance, double BatteryVoltage, double BatteryCurrent=N2kDoubleNA,
                         double BatteryTemperature=N2kDoubleNA, unsigned char SID=1);
    
    inline void SetN2kDCBatStatus(tN2kMsg &N2kMsg, unsigned char BatteryInstance, double BatteryVoltage, double BatteryCurrent=N2kDoubleNA,
                         double BatteryTemperature=N2kDoubleNA, unsigned char SID=1) {
    */
                     
    SetN2kDCBatStatus(N2kMsg,1,1.87,5.12,35.12,1);
    NMEA2000.SendMsg(N2kMsg);
    /*****************************************************************************
    // DC Detailed Status
    // Input:
    //  - SID                   Sequence ID. If your device is e.g. boat speed and heading at same time, you can set same SID for different messages
    //                          to indicate that they are measured at same time.
    //  - DCInstance            DC instance.  
    //  - DCType                Defines type of DC source. See definition of tN2kDCType
    //  - StateOfCharge         % of charge
    //  - StateOfHealth         % of heath
    //  - TimeRemaining         Time remaining in minutes
    //  - RippleVoltage         DC output voltage ripple in V
    void SetN2kPGN127506(tN2kMsg &N2kMsg, unsigned char SID, unsigned char DCInstance, tN2kDCType DCType,
                         unsigned char StateOfCharge, unsigned char StateOfHealth, double TimeRemaining, double RippleVoltage);
    
    inline void SetN2kDCStatus(tN2kMsg &N2kMsg, unsigned char SID, unsigned char DCInstance, tN2kDCType DCType,
                         unsigned char StateOfCharge, unsigned char StateOfHealth, double TimeRemaining, double RippleVoltage) {
    */
    SetN2kDCStatus(N2kMsg,1,1,N2kDCt_Battery,56,92,38500,0.012);
    NMEA2000.SendMsg(N2kMsg);
    /*****************************************************************************
    // Battery Configuration Status
    // Note this has not yet confirmed to be right. Specially Peukert Exponent can have in
    // this configuration values from 1 to 1.504. And I expect on code that I have to send
    // value PeukertExponent-1 to the bus.
    // Input:
    //  - BatteryInstance       BatteryInstance.
    //  - BatType               Type of battery. See definition of tN2kBatType
    //  - SupportsEqual         Supports equalization. See definition of tN2kBatEqSupport
    //  - BatNominalVoltage     Battery nominal voltage. See definition of tN2kBatNomVolt
    //  - BatChemistry          Battery See definition of tN2kBatChem
    //  - BatCapacity           Battery capacity in Coulombs. Use AhToCoulombs, if you have your value in Ah.
    //  - BatTemperatureCoeff   Battery temperature coefficient in %
    //  - PeukertExponent       Peukert Exponent
    //  - ChargeEfficiencyFactor Charge efficiency factor
    void SetN2kPGN127513(tN2kMsg &N2kMsg, unsigned char BatInstance, tN2kBatType BatType, tN2kBatEqSupport SupportsEqual,
                         tN2kBatNomVolt BatNominalVoltage, tN2kBatChem BatChemistry, double BatCapacity, int8_t BatTemperatureCoefficient,
            double PeukertExponent, int8_t ChargeEfficiencyFactor);
    
    inline void SetN2kBatConf(tN2kMsg &N2kMsg, unsigned char BatInstance, tN2kBatType BatType, tN2kBatEqSupport SupportsEqual,
                         tN2kBatNomVolt BatNominalVoltage, tN2kBatChem BatChemistry, double BatCapacity, int8_t BatTemperatureCoefficient,
            double PeukertExponent, int8_t ChargeEfficiencyFactor) {
    */
    SetN2kBatConf(N2kMsg,1,N2kDCbt_Gel,N2kDCES_Yes,N2kDCbnv_12v,N2kDCbc_LeadAcid,AhToCoulomb(420),53,1.251,75);
    NMEA2000.SendMsg(N2kMsg);
    // Serial.print(millis()); Serial.println(", Battery send ready");
  }
}

void SendN2KEnviro() {
  static unsigned long TempUpdated=millis();
  tN2kMsg N2kMsg;

  if ( TempUpdated+BatUpdatePeriod<millis() ) {
    TempUpdated=millis();

    //*****************************************************************************
    // Environmental parameters
    // Note that in PGN 130311 TempInstance is as TempSource in PGN 130312. I do not know why this
    // renaming is confusing.
    // Pressure has to be in pascal. Use function mBarToPascal, if you like to use mBar
    // Input:
    //  - SID                   Sequence ID. 
    //  - TempInstance          see tN2kTempSource
    //  - Temperature           Temperature in °K. Use function CToKelvin, if you want to use °C.
    //  - HumidityInstance      see tN2kHumiditySource.
    //  - Humidity              Humidity in %
    //  - AtmosphericPressure   Atmospheric pressure in Pascals. Use function mBarToPascal, if you like to use mBar
    // Output:
    //  - N2kMsg                NMEA2000 message ready to be send.
    void SetN2kPGN130311(tN2kMsg &N2kMsg, unsigned char SID, tN2kTempSource TempInstance, double Temperature,
                         tN2kHumiditySource HumidityInstance=N2khs_Undef, double Humidity=N2kDoubleNA, double AtmosphericPressure=N2kDoubleNA);
    
    inline void SetN2kEnvironmentalParameters(tN2kMsg &N2kMsg, unsigned char SID, tN2kTempSource TempInstance, double Temperature,
                         tN2kHumiditySource HumidityInstance=N2khs_Undef, double Humidity=N2kDoubleNA, double AtmosphericPressure=N2kDoubleNA) {
                          
    */       
    SetN2kEnvironmentalParameters(N2kMsg,1,CToKelvin(21.2),1,68,mBarToPascal(998));
    NMEA2000.SendMsg(N2kMsg);
  }
}




#ifndef __BATTERYMONITOR_H__
#define __BATTERYMONITOR_H__

#include <N2kMessages.h>

#ifndef NULL
#define NULL 0
#endif

#ifdef TEST
#define DEMOMODE 1
#endif

#define BatteryMonitor_MESSAGES 127508L, 127506L, 127513L

// Arduino ADC is 0.0049V / unit.


// 185mV/A with OA at 2.5V  (x * 0.0049F/0.185F) == 
// Ov would be -2.5/0.185A, although the sensor stops at -5A.
#define ACS712_5BA_ADC_TO_AMPS 0.02648648648F
#define ACS712_5BA_ZEROA_OFFSET 13.5135135135

// 66mV/A with OA at 2.5V  (x * 0.0049F/0.066F) == 
// Ov would be -2.5/0.066 A, although the sensor stops at -30A.
#define ACS712_30BA_ADC_TO_AMPS 0.07424242424F
#define ACS712_30BA_ZEROA_OFFSET 37.8787878788F

// 20mV/A with OA at 2.5V  (x * 0.0049F/0.020F) == 
// Ov would be -2.5/0.020 A, although the sensor stops at -100A.
#define ACS758_100BA_ADC_TO_AMPS 0.245F
#define ACS758_100BA_ZEROA_OFFSET 125.0F

#define VOLTAGE_DIVIDER_3TO1 0.0147F
#define TEMP_5V100C 0.098F
#define TEMP_5V100C_OFFSET 0.0F




class BatteryMonitor {
public:

    BatteryMonitor(int dcInstance, int batCapacity=100,
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
        _dcInstance = dcInstance;
        _dcType = N2kDCt_Battery;
        _batType = batType;
        _batCapacity = batCapacity; // batCapacity
        _batChemistry = batChemistry; // batChemistry
        _stateOfCharge = 80; //stateOfCharge
        _stateOfHealth = 72; //stateOfHealth
        _timeRemaining = 280; // timeRemaining
        _rippleVoltage = 0.01; // rippleVoltage
        _supportsEqual = N2kDCES_Yes; // supportsEqual
        _batNominalVoltage = batNominalVoltage; // batNominalVoltage
        _batTemperatureCoefficient = 53; // batTemperatureCoefficient
        _peukertExponent = 1.135; // PeukertExponent  https://planetcalc.com/2268/
        _chargeEfficiencyFactor = 85; // ChargeEfficiencyFactor
        _voltage = N2kDoubleNA; // voltage
        _current = N2kDoubleNA; // current
        _temperature = N2kDoubleNA; // temperature
        _voltageADCPin = voltageADCPin;
        _convertADCToVolts = convertADCToVolts;
        _zeroVOffset = zeroVOffset;
        _currentADCPin = currentADCPin;
        _convertADCToAmps = convertADCToAmps;
        _zeroAOffset = zeroAOffset;
        _temperatureADCPin = temperatureADCPin;
        _convertADCToC = convertADCToC;
        _zeroCOffset = zeroCOffset;
        useFake = false;

    }
    void begin() {
        // perform any init required.
    }
    void setFake(double voltage, double current, double temp) {
      fakeVoltage = voltage;
      fakeCurrent = current;
      fakeTemperature = temp;
      useFake = true;
    }
    void read() {
#ifdef DEMOMODE
        if ( _voltage == N2kDoubleNA ) {
            _voltage = 12.6;
            _current = 0.0;
            _temperature = 20.0;
        }
        _voltage = _voltage+0.01*(rand()%100);
        _current = _current+0.01*(rand()%100);
        _temperature = _temperature+0.01*(rand()%100);
#else
        if ( _voltageADCPin != 0 ) {
            _voltage = _convertADCToVolts*analogRead(_voltageADCPin)-_zeroVOffset;
        } else {
            _voltage = 12.0F;
        }
        if (_currentADCPin != 0 ) {
            _current = _convertADCToAmps*analogRead(_currentADCPin)-_zeroAOffset;
        } else {
            _current = 0.0F;
        }
        if (_temperatureADCPin != 0 ) {
            _temperature = _convertADCToC*analogRead(_temperatureADCPin)-_zeroCOffset;
        } else {
            _temperature = 20.0F;
        }
#endif
        if (useFake) {
          _voltage = fakeVoltage;
          _current = fakeCurrent;
          _temperature = fakeTemperature;
          useFake = false;
        }
        _batSID = _batSID+1;
    }
    void fillStatusMessage(tN2kMsg &N2kMsg) {
        SetN2kDCBatStatus(N2kMsg,
            _dcInstance,
            _voltage,
            _current,
            CToKelvin(_temperature),
            _batSID);
    }
    void fillChargeStatusMessage(tN2kMsg &N2kMsg) {
        SetN2kDCStatus(N2kMsg,
            _batSID,
            _dcInstance, 
            _dcType,
            _stateOfCharge,
            _stateOfHealth, 
            _timeRemaining, 
            _rippleVoltage);
    }
    void fillBatteryConfigurationMessage(tN2kMsg &N2kMsg) {
        SetN2kBatConf(N2kMsg,
            _dcInstance,
            _batType,
            _supportsEqual,
            _batNominalVoltage, 
            _batChemistry, 
            AhToCoulomb(_batCapacity), 
            _batTemperatureCoefficient,
            _peukertExponent, 
            _chargeEfficiencyFactor);
    }
private:
    unsigned char _dcInstance;
    tN2kDCType _dcType; //Defines type of DC source. See definition of tN2kDCType
    tN2kBatType _batType;
    unsigned char _stateOfCharge; // % of charge
    unsigned char _stateOfHealth; // % of health
    double _timeRemaining; // Time remaining in minutes
    double _rippleVoltage; // ripple in V
    tN2kBatEqSupport _supportsEqual; //         Supports equalization. See definition of tN2kBatEqSupport
    tN2kBatNomVolt _batNominalVoltage;  //  - BatNominalVoltage     Battery nominal voltage. See definition of tN2kBatNomVolt
    tN2kBatChem _batChemistry;  //  - BatChemistry          Battery See definition of tN2kBatChem
    int16_t _batCapacity; //  - BatCapacity           Battery capacity in AH
    int8_t _batTemperatureCoefficient; //  - BatTemperatureCoeff   Battery temperature coefficient in %
    double _peukertExponent; //  - PeukertExponent       Peukert Exponent
    int8_t _chargeEfficiencyFactor; //  - ChargeEfficiencyFactor Charge efficiency factor
    double _voltage;
    double _current;
    double _temperature;
    unsigned char _batSID;
    int _voltageADCPin;
    double _convertADCToVolts; // 3:1 resistor divider, 5V = 15V
    double _zeroVOffset; // 3:1 resistor divider, 5V = 15V
    int _currentADCPin;
    double _convertADCToAmps; // 0V = -100A, 2.5V = 0A, 5V = 100A  
    double _zeroAOffset; // 5V = 100A,  
    int _temperatureADCPin;
    double _convertADCToC; // 0V = 0C, 5V = 100C  
    double _zeroCOffset; // 

    
    double fakeVoltage;
    double fakeCurrent;
    double fakeTemperature;
    bool useFake;
    
};



#endif

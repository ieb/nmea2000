#ifndef __TESTMOCKS_H__
#define __TESTMOCKS_H__

#ifdef TEST

#include <iostream>
#define TRACE(x)
#define DEBUG(x) x
#define INFO(x) x
#define ERROR(x) x
#define F(x) x
#define LOG(x) std::cout << "LOG:" << x
#define LOGC(x) std::cout << x 
#define LOGN(x) std::cout << x << std::endl
#define LOGLN(x) std::cout << "LOG:" << x << std::endl
#define STATUS(x) std::cout << x;

#include <sys/time.h>

unsigned long ticks = 0;
unsigned long millis() {
    ticks = ticks + rand();
    return ticks;
}
#define max(x,y) fmax(x,y)
#define min(x,y) fmin(x,y)
#define analogRead(x)  512
#define N2kDoubleNA -1.0F
#define digitalPinToInterrupt(x) x
#define analogRead(x)  512
#define attachInterrupt(x,y,z) std::cout << "Called Attach Interrupt" << std::endl
#define  SetN2kWindSpeed(N2kMsg, sensorSID, aws, awa, type ) std::cout << "Called SetN2kWindSpeed" << std::endl
#define SetN2kTemperature(a,b,c,d,e) std::cout << "Called SetN2kTemperature" << std::endl
#define SetN2kOutsideEnvironmentalParameters(a,b,c,d,e)  std::cout << "Called SetN2kOutsideEnvironmentalParameters" << std::endl
#define SetN2kFluidLevel(a, b, c, d, e ) std::cout << "Called SetN2kFluidLevel" << std::endl
#define SetN2kBoatSpeed(a, b, c ) std::cout << "Called SetN2kBoatSpeed" << std::endl
#define SetN2kDCBatStatus(a,b,c,d,e,f)  std::cout << "Called SetN2kDCBatStatus" << std::endl
#define SetN2kDCStatus(a,b,c,d,e,f,g,h) std::cout << "Called SetN2kDCStatus" << std::endl
#define SetN2kBatConf(a,b,c,d,e,f,g,h,i,j) std::cout << "Called SetN2kBatConf" << std::endl
    


#define N2kts_SeaTemperature 1
#define N2kft_LiveWell 1

inline double RadToDeg(double v) { return v*180.0/3.1415926535897932384626433832795; }
inline double DegToRad(double v) { return v/180.0*3.1415926535897932384626433832795; }
inline double CToKelvin(double v) { return v+273.15; }
inline double KelvinToC(double v) { return v-273.15; }
inline double FToKelvin(double v) { return (v-32)*5.0/9.0+273.15; }
inline double KelvinToF(double v) { return (v-273.15)*9.0/5.0+32; }
inline double mBarToPascal(double v) { return v*100; }
inline double PascalTomBar(double v) { return v/100; }
inline double hPAToPascal(double v) { return v*100; }
inline double PascalTohPA(double v) { return v/100; }
inline double AhToCoulomb(double v) { return v*3600; }
inline double CoulombToAh(double v) { return v/3600; }
inline double hToSeconds(double v) { return v*3600; }
inline double SecondsToh(double v) { return v/3600; }
inline double msToKnots(double v) { return v*3600/1852.0; }

typedef struct _tN2kMsg {
    double dummy;
} tN2kMsg;

typedef struct _tN2kWindReference {
    double dummy;
} tN2kWindReference;

typedef struct {
    union {
        float v[3];
        struct {
            float x;
            float y;
            float z;
        };
        /* Orientation sensors */
        struct {
            float roll;    /**< Rotation around the longitudinal axis (the plane body, 'X axis'). Roll is positive and increasing when moving downward. -90°<=roll<=90° */
            float pitch;   /**< Rotation around the lateral axis (the wing span, 'Y axis'). Pitch is positive and increasing when moving upwards. -180°<=pitch<=180°) */
            float heading; /**< Angle between the longitudinal axis (the plane body) and magnetic north, measured clockwise when viewing from the top of the device. 0-359° */
        };
    };
    int8_t status;
    uint8_t reserved[3];
} sensors_vec_t;
/** struct sensors_color_s is used to return color data in a common format. */
typedef struct {
    union {
        float c[3];
        /* RGB color space */
        struct {
            float r;       /**< Red component */
            float g;       /**< Green component */
            float b;       /**< Blue component */
        };
    };
    uint32_t rgba;         /**< 24-bit RGBA value */
} sensors_color_t;

typedef struct
{
    int32_t version;                          /**< must be sizeof(struct sensors_event_t) */
    int32_t sensor_id;                        /**< unique sensor identifier */
    int32_t type;                             /**< sensor type */
    int32_t reserved0;                        /**< reserved */
    int32_t timestamp;                        /**< time is in milliseconds */
    union
    {
        float           data[4];
        sensors_vec_t   acceleration;         /**< acceleration values are in meter per second per second (m/s^2) */
        sensors_vec_t   magnetic;             /**< magnetic vector values are in micro-Tesla (uT) */
        sensors_vec_t   orientation;          /**< orientation values are in degrees */
        sensors_vec_t   gyro;                 /**< gyroscope values are in rad/s */
        float           temperature;          /**< temperature is in degrees centigrade (Celsius) */
        float           distance;             /**< distance in centimeters */
        float           light;                /**< light in SI lux units */
        float           pressure;             /**< pressure in hectopascal (hPa) */
        float           relative_humidity;    /**< relative humidity in percent */
        float           current;              /**< current in milliamps (mA) */
        float           voltage;              /**< voltage in volts (V) */
        sensors_color_t color;                /**< color in RGB component values */
    };
} sensors_event_t;

enum tN2kBatNomVolt {
                            N2kDCbnv_6v=0,
                            N2kDCbnv_12v=1,
                            N2kDCbnv_24v=2,
                            N2kDCbnv_32v=3,
                            N2kDCbnv_62v=4,
                            N2kDCbnv_42v=5,
                            N2kDCbnv_48v=6
                          };

                          
enum tN2kBatChem {
                            N2kDCbc_LeadAcid=0,
                            N2kDCbc_LiIon=1,
                            N2kDCbc_NiCad=2,
                            N2kDCbc_NiMh=3
                          };

enum tN2kBatType  {
                            N2kDCbt_Flooded=0,
                            N2kDCbt_Gel=1,
                            N2kDCbt_AGM=2
                          };

enum tN2kDCType {
                            N2kDCt_Battery=0,
                            N2kDCt_Alternator=1,
                            N2kDCt_Converter=2,
                            N2kDCt_SolarCell=3,
                            N2kDCt_WindGenerator=4
                          };

enum tN2kBatEqSupport  {
                            N2kDCES_No=0,  // No, Off, Disabled
                            N2kDCES_Yes=1, // Yes, On, Enabled
                            N2kDCES_Error=2, // Error
                            N2kDCES_Unavailable=3 // Unavailable
                          };
class Adafruit_BMP085_Unified {
public:
    Adafruit_BMP085_Unified(int n) {

    }
    bool begin() {
        return true;
    }
    bool getEvent(sensors_event_t *e) {
       return true; 
    }
    void getTemperature(float *temperature) {
        *temperature =  23.4F;
    }


};

class Stream {
public:
    char read() {
        return -1;
    }
    void print(std::string x) {

    }
    void println(std::string x) {

    }
    void print(double x) {

    }
    void println(double x) {

    }
    int available() {
        return 0;
    }
};


class Adafruit_LSM303D_Accel_Unified {
public:
    Adafruit_LSM303D_Accel_Unified(int n) {

    }
    bool begin() {
        return true;
    }
    bool getEvent(sensors_event_t *e) {
       return true; 
    }
};

class Adafruit_L3GD20_Unified {
public:
    Adafruit_L3GD20_Unified(int n) {

    }

    bool begin() {
        return true;
    }
    bool getEvent(sensors_event_t *e) {
       return true; 
    }
};



Stream LogS;
Stream * LogStream = &LogS;

class DueFlashStorage {
public:
    uint8_t read(uint16_t idx) {
        return 0;
    }
    uint8_t write(uint16_t idx, uint8_t b) {
        return b;
    }
};

// end test mocks ----------------------------------------------

#else 
#define TRACE(x)
#define DEBUG(x)
#define INFO(x)
#define ERROR(x)
#endif
#endif

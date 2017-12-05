#ifndef __MULTISENSORTEST_H__
#define __MULTISENSORTEST_H__

#ifndef ARDUINO
#ifndef TEST
#define TEST 1
#endif
#endif


#include "testmocks.h"
#include <math.h>
#include "demo.h"
#include "configuration.h"
#include "multiSensor.h"
#include "imusensor.h"
#include "pulsesensor.h"
#include "anglesensor.h"


#ifdef TEST

inline double knotsToms(double v) { return v*1852.0/3600.0; }

// unit tests can be run on a desktop, otherwise look in the h class where all the code is to enable inlining.
Adafruit_BNO055 bno = Adafruit_BNO055(55);

bool testMultiSensorMonitor() {

    Stream controlStream = Stream();
    demo_data_t demoData;
    Configuration configuration = Configuration(&controlStream, &demoData);

    Statistics statistics = Statistics();
    IMUSensor imuSensor = IMUSensor(&bno, &demoData);
    AngleSensor windAngleSensor = AngleSensor(5,6, &demoData);
    PulseSensor windSpeedSensor = PulseSensor(knotsToms(1.045F), &readWindMonitor, &windPulseHandler, 12, &demoData.windSpeed);
    PulseSensor waterSpeedSensor = PulseSensor(knotsToms(5.5F), &readWaterMonitor, &waterPulseHandler, 13, &demoData.waterSpeed);

    MultiSensor multiSensor = MultiSensor(&statistics, 
        &windSpeedSensor,
        &waterSpeedSensor,
        &windAngleSensor,
        &imuSensor,
        12.0F, 17.5F);

    configuration.testPatterns();
    multiSensor.updateConfiguration(&configuration);

    tN2kMsg message;
    for ( int i = 0; i < 10; i++ ) {
        multiSensor.read();
    }
    multiSensor.calculate();
    multiSensor.fillBoatSpeed(message);
    multiSensor.fillAparentWind(message);
    multiSensor.fillTrueWind(message);
            
     /*       
    for ( int i = 0; i < 100; i++ ) {
        waterPulseHandler();
        // cycle between 20Hz and 10Hz
        delay(50+cos((i%10)/10)*50);
        if ( i % 3 == 0 ) {
            std::cout << "Read Multisensor " << std::endl;
            multiSensor.read();
        }
        if ( i % 7 == 0 ) {
            std::cout << "Calculate Multisensor " << std::endl;
            multiSensor.calculate();
            multiSensor.fillBoatSpeed(message);
            multiSensor.fillAparentWind(message);
            multiSensor.fillTrueWind(message);
        }
    }  
    */  

    for (double mastHeadWindSpeed = 0.0; mastHeadWindSpeed < 30.0; mastHeadWindSpeed += 2  ) {
        for (double measuredWaterSpeed = 0.0; measuredWaterSpeed < 15.0; measuredWaterSpeed += 2  ) {
            for (double mastHeadWindAngle = -180; mastHeadWindAngle < +180; mastHeadWindAngle += 5  ) {
                double roll = sin(DegToRad(mastHeadWindAngle))*mastHeadWindSpeed/2;
                if ( roll > 15 ) {
                    roll = 15;
                } else if ( roll < -15 ) {
                    roll = -15;
                }
                for (double pitch = -3; pitch <= 3; pitch += 3  ) {
                   for (double gpitch = 5; gpitch <= 5; gpitch += 5  ) {
                       for (double groll = 5; groll <= 5; groll += 5  ) {
                         multiSensor.calculate(DegToRad(pitch), DegToRad(roll), DegToRad(roll), DegToRad(gpitch), DegToRad(groll), knotsToms(mastHeadWindSpeed), DegToRad(mastHeadWindAngle), knotsToms(measuredWaterSpeed));
                       }
                   }
               }
            }
        }
    }

    return true;
}



#endif
#endif



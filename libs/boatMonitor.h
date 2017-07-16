
#ifndef __BOATMONITOR_H__
#define __BOATMONITOR_H__

#include "polar.h"
#include "monitors.h"


#define PI (double)3.1415926535897932384626433832795

#ifndef TEST
#include <N2kMessages.h>
#endif

/*
typedef struct tN2kMsg_s {
    double dummyForTesting;
} tN2kMsg;
#define SetN2kEngineDynamicParam(a, b, c, d, e, f, g, h,  i, j, k, l )std::cout << "Called SetN2kEngineDynamicParam" << std::endl
#define SetN2kEngineParamRapid(a, b, c) std::cout << "Called SetN2kEngineParamRapid" << std::endl
#define N2kDoubleNA 1
#define N2kInt8NA 1
#define DegToRad(v) ((v)/180.0*3.1415926535897932384626433832795)
#define RadToDeg(v) ((v)*180.0/3.1415926535897932384626433832795)
#else

*/
#include "statistic.h"
#define BoatMonitor_MESSAGES  127489L,  127488L, 130306L, 128259L, 130312L, 128267L

inline double knotsToms(double v) { return v*1852.0/3600.0; }


class BoatMonitor {

public:
    BoatMonitor(Polar_Performance *polarPerformance, 
        Statistics *statistics, 
        bool calcTrueWind = true,
        int8_t windowS = 15 // the window to average over in s
        ) {
        this->calcTrueWind = calcTrueWind;
        this->statistics = statistics;
        this->polarPerformance = polarPerformance;
        periods = windowS;
        sid = 0;
        monitorMode = MONITOR_MODE_ENABLED;
    }


    void setMode(tMontorMode mode) {
      monitorMode = mode;
    }

    bool isEnabled() {
        return (monitorMode == MONITOR_MODE_ENABLED) || (monitorMode == MONITOR_MODE_DEMO);
    }


    void read(unsigned long tnow) {
        if (monitorMode == MONITOR_MODE_DEMO || monitorMode == MONITOR_MODE_ENABLED) {
            meanSTW = statistics->stw.means(periods, tnow);
            stdevSTW = statistics->stw.stdevs(periods, tnow);
            meanAWS = statistics->aws.means(periods, tnow);
            stdevAWS = statistics->aws.stdevs(periods, tnow);
            meanAWA = statistics->awa.means(periods, tnow);
            stdevAWA = statistics->awa.stdevs(periods, tnow);
            if (calcTrueWind) {
                while ( meanAWA > PI ) meanAWA = meanAWA - PI;
                while ( meanAWA < -PI) meanAWA = meanAWA + PI;
                meanTWS = sqrt((meanSTW*meanSTW+meanAWS*meanAWS)-(2*meanSTW*meanAWS*cos(meanAWA)));
                meanTWA = 0.0F;
                if ( meanTWS > 1.0E-3F ) {
                    meanTWA = (meanAWS*cos(meanAWA)-meanSTW)/meanTWS;
                    if ( meanTWA > 0.9999F || meanTWA < -0.9999F) {
                        meanTWA = 0.0F;
                    } else {
                        meanTWA = acos(meanTWA);
                    }
                }
                if ( meanAWA < 0) {
                    meanTWA = -meanTWA;
                }
            } else {
                meanTWS = statistics->tws.means(periods, tnow);
                stdevTWS = statistics->tws.stdevs(periods, tnow);
                meanTWA = statistics->twa.means(periods, tnow);
                stdevTWA = statistics->twa.stdevs(periods, tnow);            
            }
            targetSTW = knotsToms(polarPerformance->getBoatSpeed(msToKnots(meanTWS), RadToDeg(meanTWA)));
            performance = knotsToms(polarPerformance->getBoatSpeedPerformance(msToKnots(meanTWS), RadToDeg(meanTWA), msToKnots(meanSTW)));
            sid++;
        }
        LOG(F("Boat Monitor STW:"));
      LOGC(msToKnots(meanSTW));
      LOGC(F(":"));
      LOGC(msToKnots(stdevSTW));
      LOGC(F(" Kn, AWS:"));
      LOGC(msToKnots(meanAWS));
      LOGC(F(":"));
      LOGC(msToKnots(stdevAWS));
      LOGC(F(" Kn, AWA:"));
      LOGC(RadToDeg(meanAWA));
      LOGC(F(":"));
      LOGC(RadToDeg(stdevAWA));
      LOGC(F(" deg, TWS:"));
      LOGC(msToKnots(meanTWS));
      LOGC(F(":"));
      LOGC(msToKnots(stdevTWS));
      LOGC(F(" Kn, TWA:"));
      LOGC(RadToDeg(meanTWA));
      LOGC(F(":"));
      LOGC(RadToDeg(stdevTWA));
      LOGC(F(" deg, TTW:"));
      LOGC(msToKnots(targetSTW));
      LOGC(F(" Kn, PTW:"));
      LOGC(msToKnots(performance));
      LOGN(F(" Kn"));

    }




    void fillPolarPerformance(tN2kMsg &N2kMsg) {      
        // Representing polar performance wth %LiveWell
        SetN2kFluidLevel(N2kMsg, 1, N2kft_LiveWell, performance, 250);
    }

    void fillTargetBoatSpeed(tN2kMsg &N2kMsg) {
        // Representing Target boat speed with True/Theoretical WindChill
        SetN2kTemperature(N2kMsg, sid, 1, N2kts_TheoreticalWindChillTemperature,
                           CToKelvin(targetSTW)); // 
    }
    void fillBoatSpeed(tN2kMsg &N2kMsg) {
        SetN2kBoatSpeed(N2kMsg, sid, meanSTW); //
    }
    void fillAparentWind(tN2kMsg &N2kMsg) {
        SetN2kWindSpeed(N2kMsg, sid, meanAWS, meanAWA, N2kWind_Apprent); //
    }
    void fillTrueWind(tN2kMsg &N2kMsg) {
        SetN2kWindSpeed(N2kMsg, sid, meanTWS, meanTWA, N2kWind_True_boat);
    }
private:
    double performance;
    double targetSTW;
    double meanSTW;
    double stdevSTW;
    double meanTWS;
    double stdevTWS;
    double meanTWA;
    double stdevTWA;
    double meanAWS;
    double stdevAWS;
    double meanAWA;
    double stdevAWA;
    bool calcTrueWind;

    int8_t periods;
    uint8_t sid;
    Polar_Performance *polarPerformance;
    Statistics *statistics;
    tMontorMode monitorMode;


};


#endif


#ifndef __BOATMONITOR_H__
#define __BOATMONITOR_H__

#include "conversions.h"
#include "polar.h"
#include "configuration.h"



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
#define BoatMonitor_MESSAGES  130312L, 127505L





class BoatMonitor {

public:
    BoatMonitor(Polar_Performance *polarPerformance, 
        Statistics *statistics, 
        int8_t windowS = 15 // the window to average over in s
        ) {
        this->statistics = statistics;
        this->polarPerformance = polarPerformance;
        periods = windowS;
        sid = 0;
        performanceEnabled = false;
        demoMode = false;
    }

    bool init() {
      return true;
    }

    void updateConfiguration(Configuration *config) {
      performanceEnabled = config->getPerformanceEnabled();
      demoMode = config->getDemoMode();
    }

    void dumpRunstate() {
      unsigned long tnow = millis();
      float ctws = statistics->tws.means(periods, tnow);
      float ctwa = statistics->twa.means(periods, tnow);
      float cstw = statistics->stw.means(periods, tnow);
      float magneticVariation = statistics->magneticVariation.means(periods, tnow);
      float headingTrue = statistics->hdt.means(periods, tnow);
      float leeway = statistics->leeway.means(periods, tnow);
      performance_data_t pd;
      polarPerformance->fillPerformanceData(ctws, ctwa, cstw, headingTrue, magneticVariation, leeway,  &pd);

      DUMP(F("Boat Monitor STW:"));
      DUMPC(msToKnots(cstw));
      DUMPC(F(" Kn, TWS:"));
      DUMPC(msToKnots(ctws));
      DUMPC(F(" Kn, TWA:"));
      DUMPC(RadToDeg(ctwa));
      DUMPC(F(" deg, TTW:"));
      DUMPC(msToKnots(pd.polarstw));
      DUMPC(F(" Kn, PTW:"));
      DUMPC(pd.polarstwRatio);
      DUMPC(F(" %%, PVMG:"));
      DUMPC(msToKnots(pd.polarvmg));
      DUMPC(F(" Kn, VMG:"));
      DUMPC(msToKnots(pd.vmg));
      DUMPC(F(" Kn, VMGR:"));
      DUMPC(pd.polarvmgRatio);
      DUMPC(F(" %%, TVMG:"));
      DUMPC(msToKnots(pd.targetvmg));
      DUMPC(F(" Kn, TSTW:"));
      DUMPC(msToKnots(pd.targetstw));
      DUMPC(F(" Kn, TTWA:"));
      DUMPC(RadToDeg(pd.targettwa));
      DUMPC(F(" Deg, THAT:"));
      DUMPC(RadToDeg(pd.tackHeadingTrue));
      DUMPC(F(" Deg, TAHM:"));
      DUMPC(RadToDeg(pd.tackHeadingMagnetic));
      DUMPC(F(" Deg, TATT:"));
      DUMPC(RadToDeg(pd.tackTrackTrue));
      DUMPC(F(" Deg, TATM:"));
      DUMPC(RadToDeg(pd.tackTrackMagnetic));
      DUMPN(F(" Deg"));
    }


    bool read(unsigned long tnow) {
      if(!performanceEnabled) {
        return false;
      }
      float ctws = statistics->tws.means(periods, tnow);
      float ctwa = statistics->twa.means(periods, tnow);
      float cstw = statistics->stw.means(periods, tnow);
      float magneticVariation = statistics->magneticVariation.means(periods, tnow);
      float headingTrue = statistics->hdt.means(periods, tnow);
      float leeway = statistics->leeway.means(periods, tnow);
      polarPerformance->fillPerformanceData(ctws, ctwa, cstw, headingTrue, magneticVariation, leeway,  &perfdata);
      sid++;

      LOG(F("Boat Monitor STW:"));
      LOGC(msToKnots(cstw));
      LOGC(F(" Kn, TWS:"));
      LOGC(msToKnots(ctws));
      LOGC(F(" Kn, TWA:"));
      LOGC(RadToDeg(ctwa));
      LOGC(F(" deg, TTW:"));
      LOGC(msToKnots(perfdata.polarstw));
      LOGC(F(" Kn, PTW:"));
      LOGC(perfdata.polarstwRatio);
      LOGC(F(" %%, PVMG:"));
      LOGC(msToKnots(perfdata.polarvmg));
      LOGC(F(" Kn, VMG:"));
      LOGC(msToKnots(perfdata.vmg));
      LOGC(F(" Kn, VMGR:"));
      LOGC(perfdata.polarvmgRatio);
      LOGC(F(" %%, TVMG:"));
      LOGC(msToKnots(perfdata.targetvmg));
      LOGC(F(" Kn, TSTW:"));
      LOGC(msToKnots(perfdata.targetstw));
      LOGC(F(" Kn, TTWA:"));
      LOGC(RadToDeg(perfdata.targettwa));
      LOGC(F(" Deg, THAT:"));
      LOGC(RadToDeg(perfdata.tackHeadingTrue));
      LOGC(F(" Deg, TAHM:"));
      LOGC(RadToDeg(perfdata.tackHeadingMagnetic));
      LOGC(F(" Deg, TATT:"));
      LOGC(RadToDeg(perfdata.tackTrackTrue));
      LOGC(F(" Deg, TATM:"));
      LOGC(RadToDeg(perfdata.tackTrackMagnetic));
      LOGN(F(" Deg"));
      return true;
    }




    void fillPolarPerformance(tN2kMsg &N2kMsg) {      
        // Representing polar performance wth %LiveWell
//        SetN2kFluidLevel(N2kMsg, 1, N2kft_Fuel, performance, 250);
    }

    void fillTargetBoatSpeed(tN2kMsg &N2kMsg) {
        // Representing Target boat speed with True/Theoretical WindChill
//        SetN2kTemperature(N2kMsg, sid, 1, N2kts_TheoreticalWindChillTemperature,
//                           CToKelvin(targetSTW)); // 
    }





/*

Speed PGN 128259
          { path: 'performance.polarSpeed', value: polarPerformance.polarSpeed},   // polar speed at this twa
speed target stw
      { path: 'performance.velocityMadeGood', value: polarPerformance.vmg}, // current vmg at polar speed
speed vmg

      { path: 'performance.targetVelocityMadeGood', value: targets.vmg}, // target vmg -ve == downwind
speed target vmg
      { path: 'performance.targetSpeed', value: targets.stw}, // target speed on at best vmg and angle
speed target for best vmg



Heading PGN 127250
      { path: 'performance.tackMagnetic', value: track.trackMagnetic}, // other track through water magnetic taking into account leeway 
heading - other tack magnetic - with leeway

      { path: 'performance.tackTrue', value: track.trackTrue}, // other track through water true taking into account leeway
heading - other tack true - with leeway

      { path: 'performance.headingMagnetic', value: track.headingMagnetic}, // other track heading on boat compass
heading - otehr tack magnetic no leeway

      { path: 'performance.headingTrue', value: track.headingTrue}, // other track heading true
heading - other tack true no leeway 

Wind PGN 130306
      { path: 'performance.targetAngle', value: targets.twa}, // target twa on this track for best vmg
wind angle, target for best vmg


Levels 127505

      { path: 'performance.polarVelocityMadeGoodRatio', value: polarPerformance.polarVmgRatio} // current vmg vs current polar vmg.
ratio target vmg to vmg.
      { path: 'performance.polarSpeedRatio', value: polarPerformance.polarSpeedRatio}, // polar speed ratio
ratio speed to stw
*/


private:
    performance_data_t perfdata;
    bool performanceEnabled;
    bool demoMode;

    int8_t periods;
    uint8_t sid;
    Polar_Performance *polarPerformance;
    Statistics *statistics;



};


#endif

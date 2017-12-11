/**
 * This code simulates the standard St60+ Wind Sensor, to enable simulatio and debugging of a Raymarine i60/i60 or ITC5 setup without real
 * instruments or real enviroment.
 * The wind direction is simulated by driving 2 I2C DACs between 0 and the Arduino supply voltage. This is not exactly the 2-6V 
 * that the Raymarine wind transducer produces so if sending the voltages to a real ITC5 or Pods it will need conditioning.
 * 
 * All the parameter as controllable over the serial line. Default settings have AWS at 1.045Hz/kn and STW at 5.5Hz/Kn.
 * serial line runs at 115.2Kbaud, type help to see the command format.
 */


#include <Wire.h>
// #include "DueTimer.h"


#define AWS_PIN 4
#define STW_PIN 5
#define DAC_REGISTER 0x40
#define DACV 3F/4096.0F
#define WINDANGLE_UPDATE_PERIOD 500
#define SPEED_UPDATE_PERIOD 500

float awsHzPerKn = 1.045F;
float stwHzPerKn = 5.5F;
float targetAWA = 0.0F;
float targetAWS = 0.0F;
float targetSTW = 0.0F;
float targetFAWS = 0.0F;
float targetFSTW = 0.0F;
unsigned long noiseLevel = 0L;
unsigned long awsPeriod = 1000000;
unsigned long stwPeriod = 1000000;
unsigned long awsError = 0L;
unsigned long stwError = 0L;
float awa = 0.0F;



/*
 * Wiring.
 * Needs 2x MCP Digital pots set to generate wiper voltages between 2V and 6V. Cos Pin Select is pin 7, Sin pin select is pin 8
 * On Pin 4 there will be a 25/75 PWM output for AWS
 * On Pin 5 there will be a 25/75 PWM output for STW
 */

void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200);
   Serial.println(F("Wind and speed Transducer simulator"));

  Wire.begin(); 

  pinMode(AWS_PIN, OUTPUT);
  pinMode(STW_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Timer1.attachInterrupt(pulseAWS).setPeriod(500000).start();
  // Timer2.attachInterrupt(pulseSTW).setPeriod(500000).start();

}
/*
uint8_t awaState = 0;
void pulseAWS(void) {
  awaState = (awaState+1)&0x03;
  digitalWrite(AWS_PIN, awaState!=0);
}

uint8_t stwState = 0;
void pulseSTW(void) {
  stwState = (stwState+1)&0x03;
  digitalWrite(STW_PIN, stwState!=0);
}
*/
/*
 * Return the period in us.
 */
unsigned long getPeriod(float frequency) {
  if (frequency == 0 ) {
    return 500000;
  }  
  // no faster than 1Khz
  // no slower than 0.5Hz
  float period = 1000000/frequency;   
  return max(min(2000000,(unsigned long)period),1000);
}








void setVoltage(byte address, float voltageF) {
  byte op[3];
  op[0] = DAC_REGISTER;
  if ( voltageF > 4095.0F) {
    op[1] = (4095>>4) & 0xff;
    op[2] = (4095<<4) & 0xff;
  } else if ( voltageF < 0.0F ) {
    op[1] = 0x00;
    op[2] = 0x00;
  } else {
    uint16_t vi = (uint16_t) voltageF;
    op[1] = (vi>>4) & 0xff;
    op[2] = (vi<<4) & 0xff;
  }
  Wire.beginTransmission(address);
  Wire.write(op,3);
  Wire.endTransmission();
}

void setWindAngle() {
  static float lastAWA = 0.0F;
  if ( awa == lastAWA) {
    return;
  }
  lastAWA = awa;

  float cosADC = 2000.0F+1500.0F*cos(awa*M_PI/180.0F);
  float sinADC = 2000.0F+1500.0F*sin(awa*M_PI/180.0F);

  // sin MCP4725 on I2C at 0x62
  Serial.print("V0 ");
  Serial.print(cosADC);
  Serial.print(" ");
  Serial.print((float)cosADC*3.42F/4096.0F);
  Serial.print(" V1 ");
  Serial.print(sinADC);
  Serial.print(" ");
  Serial.println((float)sinADC*3.42F/4096.0F);
  setVoltage(0x62,cosADC);
  // cos MCP4725 on I2C at 0x62
  setVoltage(0x63,sinADC);    
}

int ncycles = 0, cawa = 0;
// these are the factory settings. There appears to be some significant non linearity
// between the angle generated and hte andle recored


void getIncommingChars() {
  static String input = "";
  
  char inChar = Serial.read();
  if(inChar == 59 || inChar == 10 || inChar == 13){
    if ( input.length() > 0 ) {
      Serial.println(" ");
      input.toLowerCase();
      if ( input.startsWith("awa,") ) {
        cawa = 0;
        targetAWA = input.substring(4).toFloat();
      } else if (input.startsWith("rnd,") ) {
        noiseLevel = input.substring(4).toInt();
      } else if (input.startsWith("cwa,") ) {
        cawa = input.substring(4).toInt();
      } else if (input.startsWith("aws,") ) {
        targetAWS = input.substring(4).toFloat();
        targetFAWS = targetAWS*awsHzPerKn;
      } else if (input.startsWith("ahz") ) {
        awsHzPerKn = input.substring(4).toFloat();
        targetFAWS = targetAWS*awsHzPerKn;
      } else if (input.startsWith("stw") ) {
        targetSTW = input.substring(4).toFloat();
        targetFSTW = targetSTW*stwHzPerKn;
      } else if (input.startsWith("bhz") ) {
        stwHzPerKn = input.substring(4).toFloat();
        targetFSTW = targetSTW*stwHzPerKn;
      } else if (input.startsWith("help")) {
        Serial.println(F("aws,<Aparent Wind speed in kn>"));
        Serial.println(F("awa,<Aparent Wind Angle in degrees>"));
        Serial.println(F("stw,<Boat Speed in Kn>"));
        Serial.println(F("ahz,<Hz per kn of AWS>"));
        Serial.println(F("bhz,<Hz per kn of STW>"));
      }
      Serial.print(F("AWA,"));
      Serial.print(targetAWA);
      Serial.print(F(",opAWA,"));
      Serial.print(awa);
      Serial.print(F(",AWS,"));
      Serial.print(targetAWS);
      Serial.print(F(",AWSHz,"));
      Serial.print(targetFAWS);
      Serial.print(F(",opAWSp,"));
      Serial.print(awsPeriod);
      Serial.print(F(",STW,"));
      Serial.print(targetSTW);
      Serial.print(F(",STWHz,"));
      Serial.print(targetFSTW);
      Serial.print(F(",opSTWp,"));
      Serial.print(stwPeriod);
      Serial.print(F(",AWSHz,"));
      Serial.print(awsHzPerKn);
      Serial.print(F(",STWHz,"));
      Serial.print(stwHzPerKn);
      Serial.print(F(",AWSuserr,"));
      Serial.print(awsError);
      Serial.print(F(",STWuserr,"));
      Serial.println(stwError);
      
      input = "";    
      Serial.print("->");        
    }
  } else {
    if (inChar > 32 && inChar < 127 && input.length() < 255 ) {
      input += (char)inChar;
    }
  }
}
void flashLED() {
  static unsigned long nextLED = 0L;
  static bool ledOn = true;
  if ( millis() > nextLED ) {
    nextLED = millis() + 5000;
    digitalWrite(LED_BUILTIN, ledOn);
    ledOn = !ledOn;
  }
}


void moveAWA() {
  static unsigned long nextAWAMove = 0L;
  if (cawa != 0 && millis() > nextAWAMove ) {
    targetAWA = targetAWA+1;
    if ( targetAWA > 180.0F ) {
      targetAWA = targetAWA-360.0F;
    }
    nextAWAMove = millis() + cawa;
  }
}

void updateAWA() {
  static unsigned long nextAWAUpdate = 0L;
  if (millis() > nextAWAUpdate ) {
    if ( noiseLevel > 0) {
      awa = targetAWA + random(-noiseLevel,+noiseLevel);
    } else {
      awa = targetAWA;
    }
    setWindAngle();
    nextAWAUpdate = millis() + WINDANGLE_UPDATE_PERIOD;
  }
}

void updateSpeeds() {
  static unsigned long nextSpeedUpdate = 0L;
  if (millis() > nextSpeedUpdate ) {
    if ( noiseLevel > 0) {
      stwPeriod = getPeriod((targetSTW + (float)(random(-noiseLevel*100,+noiseLevel*100)/100.0F)) * stwHzPerKn);
      awsPeriod = getPeriod((targetAWS + (float)(random(-noiseLevel*100,+noiseLevel*100)/100.0F)) *awsHzPerKn);
    } else {
      stwPeriod = getPeriod(targetSTW * stwHzPerKn);
      awsPeriod = getPeriod(targetAWS * awsHzPerKn);
    }
    nextSpeedUpdate = millis() + SPEED_UPDATE_PERIOD;
  }
}

void pulseSTW() {
  static unsigned long nextSTWUpdate = 0L;
  static unsigned long previousSTWUpdate = 0L;
  static bool stwPulse = true;
  // micros wraps every 70 min.
  unsigned long microsNow = micros();
  if (microsNow < previousSTWUpdate || microsNow > nextSTWUpdate ) {
    stwError = stwError-(stwError/10)+(microsNow-nextSTWUpdate); // do an IIR on the error
    previousSTWUpdate = microsNow;
    stwPulse = !stwPulse;
    if ( stwPulse ) {
      digitalWrite(STW_PIN, 0);
      nextSTWUpdate = micros() + stwPeriod/4;
    } else {
      digitalWrite(STW_PIN, 1);
      nextSTWUpdate = micros() + 3*stwPeriod/4;
    }
  }
}
void pulseAWS() {
  static unsigned long nextAWSUpdate = 0L;
  static unsigned long previousAWSUpdate = 0L;
  static bool awsPulse = true;
  // micros wraps every 70 min.
  unsigned long microsNow = micros();
  if (microsNow < previousAWSUpdate || microsNow > nextAWSUpdate ) {
    awsError = awsError-(awsError/10)+(microsNow-nextAWSUpdate); // do an IIR on the error
    previousAWSUpdate = microsNow;
    awsPulse = !awsPulse;
    if ( awsPulse ) {
      digitalWrite(AWS_PIN, 0);
      nextAWSUpdate = micros() + awsPeriod/4;
    } else {
      digitalWrite(AWS_PIN, 1);
      nextAWSUpdate = micros() + 3*awsPeriod/4;
    }
  }
}




void loop() {
  flashLED();
  updateSpeeds();
  pulseAWS();
  pulseSTW();
  moveAWA();
  updateAWA();
  if (Serial.available()) {
    getIncommingChars();
  }
}





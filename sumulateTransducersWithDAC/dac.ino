/**
 * This code simulates the standard St60+ Wind Sensor, to enable simulatio and debugging of a Raymarine i60/i60 or ITC5 setup without real
 * instruments or real enviroment.
 * The wind direction is simulated by driving 2 I2C DACs between 0 and the Arduino supply voltage. This is not exactly the 2-6V 
 * that the Raymarine wind transducer produces so if sending the voltages to a real ITC5 or Pods it will need conditioning.
 * 
 * All the parameter as controllable over the serial line. Default settings have AWS at 1.045Hz/kn and BSP at 5.5Hz/Kn.
 * serial line runs at 115.2Kbaud, type help to see the command format.
 */


#include <Wire.h>
#include <TimerOne.h>   # https://github.com/PaulStoffregen/TimerOne
#include <TimerThree.h>  # https://github.com/PaulStoffregen/TimerThree


#define AWS_PIN 4
#define BSP_PIN 5

/*
 * Wiring.
 * Needs 2x MCP Digital pots set to generate wiper voltages between 2V and 6V. Cos Pin Select is pin 7, Sin pin select is pin 8
 * On Pin 4 there will be a 25/75 PWM output for AWS
 * On Pin 5 there will be a 25/75 PWM output for BSP
 */

void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200);
   Serial.println("Wind and speed Transducer simulator");

  Wire.begin(); 

  pinMode(AWS_PIN, OUTPUT);
  pinMode(BSP_PIN, OUTPUT);
  
  Timer1.initialize(500000);         // initialize timer1, and set a 1/2 second period
  Timer1.attachInterrupt(pulseAWS);
  Timer1.start();
  Timer3.initialize(500000);         // initialize timer1, and set a 1/2 second period
  Timer3.attachInterrupt(pulseBSP);
  Timer3.start();

}
uint8_t awaState = 0;
void pulseAWS(void) {
  awaState = (awaState+1)&0x03;
  digitalWrite(AWS_PIN, awaState==0);
}

uint8_t bspState = 0;
void pulseBSP(void) {
  bspState = (bspState+1)&0x03;
  digitalWrite(BSP_PIN, bspState==0);
}

unsigned long getPeriod(float value, float hz_per) {
  if (value == 0 ) {
    return 500000;
  }
  // need 4 pulses per cycle to get 25 % duty cycle.
  return max(min(500000,250000/(value*hz_per)),1000);
}



int cawa = 0;
// these are the factory settings. There appears to be some significant non linearity
// between the angle generated and hte andle recored

float awa = 10, aws = 25, awsHzPerKn = 1.045F, bsp = 12, bspHzPerKn = 5.5F;

#define DAC_REGISTER 0x40
#define DACV 5.5F/4096.0F

void setVoltage(byte address, uint_16 voltage) {
  if ( voltage > 4095) {
    voltage = 4095;
  } else if ( voltage < 0 ) {
    voltage = 0;
  }

  byte[] op = { DAC_REGISTER, (voltage >> 4) & 0xff, (voltage << 4 ) & 0xff };
  Write.beginTransmission(address);
  Write.write(3,op);
  Write.endTransmission();
}

void setWindAngle() {

  float cosADC = 2048.0F+2048.0F*cos(awa*M_PI/180.0F);
  float sinADC = 2048.0F+2048.0F*sin(awa*M_PI/180.0F);

  unit_16 cosADCI = (int) cosADC;
  unit_16 sinADCI = (int) sinADC;

  // sin MCP4725 on I2C at 0x62
  setVoltage(0x62,(int)cosADC);
  // cos MCP4725 on I2C at 0x62
  setVoltage(0x63,(int)sinADC);


    
  Serial.print("AWA ");
  Serial.print(awa);
  Serial.print(" Cos V  ");
  Serial.print(cosADC*DACV);
  Serial.print(" SinT V ");
  Serial.println(sinADC*DACV);
}


void getIncommingChars() {
  static String input = "";
  
  char inChar = Serial.read();
  if(inChar == 59 || inChar == 10 || inChar == 13){
    if ( input.length() > 0 ) {
      Serial.println(" ");
      input.toLowerCase();
      if ( input.startsWith("awa,") ) {
        cawa = 0;
        awa = input.substring(4).toFloat();
        setWindAngle();
      } else if (input.startsWith("cwa,") ) {
        cawa = input.substring(4).toInt();
        awa = 0.0F;
        setWindAngle();
      } else if (input.startsWith("aws,") ) {
        aws = input.substring(4).toFloat();
        Timer1.setPeriod(getPeriod(aws,awsHzPerKn));
      } else if (input.startsWith("ahz") ) {
        awsHzPerKn = input.substring(4).toFloat();
        Timer1.setPeriod(getPeriod(aws,awsHzPerKn));
      } else if (input.startsWith("bsp") ) {
        bsp = input.substring(4).toFloat();
        Timer3.setPeriod(getPeriod(bsp,bspHzPerKn));
      } else if (input.startsWith("bhz") ) {
        bspHzPerKn = input.substring(4).toFloat();
        Timer3.setPeriod(getPeriod(bsp,bspHzPerKn));
      } else if (input.startsWith("help")) {
        Serial.println("aws,<Aparent Wind speed in kn>");
        Serial.println("awa,<Aparent Wind Angle in degrees>");
        Serial.println("bsp,<Boat Speed in Kn>");
        Serial.println("ahz,<Hz per kn of AWS>");
        Serial.println("bhz,<Hz per kn of BSP>");
      }
      Serial.println("Current: AWA,AWS,BSP,AWS Hz/Kn,BSP Hz/Kn,aws ms, bsp ms");
      Serial.print(awa);
      Serial.print(",");
      Serial.print(aws);
      Serial.print(",");
      Serial.print(bsp);
      Serial.print(",");
      Serial.print(awsHzPerKn);
      Serial.print(",");
      Serial.print(bspHzPerKn);
      Serial.print(",");
      Serial.print(getPeriod(aws,awsHzPerKn));
      Serial.print(",");
      Serial.println(getPeriod(bsp,bspHzPerKn));
      
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
    digitalWrite(13, ledOn);
    ledOn = !ledOn;
  }
}

void loop() {
  flashLED();
   if (Serial.available()) {
      getIncommingChars();
   }
   if ( cawa != 0 ) {
    awa = awa+1;
    if ( awa > 180 ) {
      awa = awa-360;
    }
    setWindAngle();
    delay(cawa);
   }
}





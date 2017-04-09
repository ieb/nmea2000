/**
 * This code simulates the standard St60+ Wind Sensor, to enable simulatio and debugging of a Raymarine i60/i60 or ITC5 setup without real
 * instruments or real enviroment.
 * The wind direction is simulated by driving 2 digital pots to generate the correct sin and cosine voltages between 6 and 2 volts.
 * the MCP4161-103 10K pots Pots are controlled over SPI with the cos select pin on pin 7 and sin select pin on pin 8
 * The MCP4161-103 are wired as voltage dividers wth 1K to the i60 supply voltage (8V) 0.5K to 0V. 
 *
 *     MCP4161-103  5  --- 1K ---- 8V from i60 Red        
 *     MCP4161-103  6  --- to i60 green or blue
 *     MCP4161-103  5  ----0.5K --- 0V from i60 grey
 *
 *     Arduino pin 4 --- AWS i60 Yellow
 *     Ardiono pin 5 --- BSP i50 Green
 *     10K pot for temp i50 White.
 *
 * The wind speed is simulated with a pulse using timers, output on pin 9 
 * Boat speed is simulated with a pulse on pin 5
 * ADC 1 and ADC 0 read the output voltage and adjust the pot to match since a raymarine ST60 will draw sufficient current to make the 
 * resistance to voltage non linear. Sense is taken via 2x 10K resistors to divide the voltage into 2.
 * Any errors in cos/sin voltages from the ideal result in amplification of the error on  the instrument.
 * All the parameter as controllable over the serial line. Default settings have AWS at 1.045Hz/kn and BSP at 5.5Hz/Kn.
 * serial line runs at 115.2Kbaud, type help to see the command format.
 */


#include <SPI.h>
#include <McpDigitalPot.h> # https://github.com/teabot/McpDigitalPot.git
#include <TimerOne.h>   # https://github.com/PaulStoffregen/TimerOne
#include <TimerThree.h>  # https://github.com/PaulStoffregen/TimerThree


#define AWS_PIN 4
#define BSP_PIN 5
#define AWA_COS_SLAVE_SELECT_PIN 7
#define AWA_SIN_SLAVE_SELECT_PIN 8
#define COS_SENSE_PIN 0  // ADC Pin 0
#define SIN_SENSE_PIN 1  // ADC Pin 1
McpDigitalPot awaCos = McpDigitalPot( AWA_COS_SLAVE_SELECT_PIN, 10900.00 );
McpDigitalPot awaSin = McpDigitalPot( AWA_SIN_SLAVE_SELECT_PIN, 10900.00 );

/*
 * Wiring.
 * Needs 2x MCP Digital pots set to generate wiper voltages between 2V and 6V. Cos Pin Select is pin 7, Sin pin select is pin 8
 * On Pin 4 there will be a 25/75 PWM output for AWS
 * On Pin 5 there will be a 25/75 PWM output for BSP
 */

void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200);
   Serial.println("Wind and speed Trasnducer simulator");

  SPI.begin(); 
  awaSin.scale = 100.0;
  awaCos.scale = 100.0;
  awaSin.setResistance(0, 50.0F); 
  awaCos.setResistance(0, 0.0F);

  pinMode(AWS_PIN, OUTPUT);
  pinMode(BSP_PIN, OUTPUT);
  pinMode(13, OUTPUT);
  
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

void setWindAngle() {
  // A i60 draws enough current from the sensor to result in offsetting the pot, so we have to
  // adjust and then set a target voltage to achieve by iteration.
    float cosr = (cos(awa*M_PI/180.0F)+1.0F)*100.0; // starting wiper position
    float sinr = (sin(awa*M_PI/180.0F)+1.0F)*100.0; // starting wiper position
    float cost = 4.0F+1.6F*(cos(awa*M_PI/180.0F)); // target voltage
    float sint = 4.0F+1.6F*(sin(awa*M_PI/180.0F)); // target voltage
    // set the voltages aproximately.
    bool hasError = true;
    float cossense = 0.0, sinsense = 0.0;
    awaCos.setResistance(0,cosr);
    awaSin.setResistance(0,sinr);
    // adjust
    int n = 0;
    while(hasError && n < 200) {
      n++;
      delay(5);
      hasError = false;
      cossense = 2.0F*0.0049*analogRead(COS_SENSE_PIN);
      if ( cossense > (cost+0.03) ) {
        cosr = cosr-0.5;
        if (cosr < 0.3) {
          cosr = 0.3;
        }
        awaCos.setResistance(0,cosr);
        hasError = true;
      } else if (cossense < (cost-0.05)) {
        cosr = cosr+0.5;
        awaCos.setResistance(0,cosr);
        hasError = true;
      }
      sinsense = 2.0F*0.0049*analogRead(SIN_SENSE_PIN);
      if ( sinsense > (sint+0.03) ) {
        sinr = sinr-0.5;
        if (sinr < 0.3) {
          sinr = 0.3;
        }
        awaSin.setResistance(0,sinr);
        hasError = true;
        
      } else if (sinsense < (sint-0.05)) {
        sinr = sinr+0.5;
        awaSin.setResistance(0,sinr);
        hasError = true;
      }
      Serial.print(" Sin Error:");
      Serial.print(sinsense);
      Serial.print(":");
      Serial.print(sint);
      Serial.print(":");
      Serial.print(sinsense-sint);
      Serial.print(" Cos Error:");
      Serial.print(cossense);
      Serial.print(":");
      Serial.print(cost);
      Serial.print(":");
      Serial.println(cossense-cost);
    }
    
    Serial.print("AWA ");
    Serial.print(awa);
    Serial.print(" CosT V  ");
    Serial.print(cost);
    Serial.print(" SinT V ");
    Serial.print(sint);
    Serial.print(" CosS V  ");
    Serial.print(cossense);
    Serial.print(" SinS V  ");
    Serial.println(sinsense);
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
      } else if (input.startsWith("rc,") ) {
        awaCos.setResistance(0,input.substring(3).toFloat()*2.0F);
      } else if (input.startsWith("rs,") ) {
        awaSin.setResistance(0,input.substring(3).toFloat()*2.0F);
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





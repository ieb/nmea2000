#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include <SPI.h>
#include <SD.h>


/* 
 *  Based on an Adafruit example. Performs calibration and saves calibration to a SD card file.
 */

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (100)

Adafruit_BNO055 bno = Adafruit_BNO055(55);

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
    */
/**************************************************************************/
void displaySensorDetails(void)
{
    sensor_t sensor;
    bno.getSensor(&sensor);
    Adafruit_BNO055::adafruit_bno055_rev_info_t rev_info;
    bno.getRevInfo(&rev_info);
    Serial.println("------------------------------------");
    Serial.print("Sensor:       "); Serial.println(sensor.name);
    Serial.print("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
    Serial.print("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
    Serial.print("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
    Serial.print("accel_rev:   "); Serial.print(rev_info.accel_rev); Serial.println(" xxx");
    Serial.print("mag_rev:   "); Serial.print(rev_info.mag_rev); Serial.println(" xxx");
    Serial.print("gyro_rev:   "); Serial.print(rev_info.gyro_rev); Serial.println(" xxx");
    Serial.print("bl_rev:   "); Serial.print(rev_info.bl_rev); Serial.println(" xxx");
    Serial.print("sw_rev:   "); Serial.print(rev_info.sw_rev); Serial.println(" xxx");
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}

/**************************************************************************/
/*
    Display some basic info about the sensor status
    */
/**************************************************************************/
void displaySensorStatus(void)
{
    /* Get the system status values (mostly for debugging purposes) */
    uint8_t system_status, self_test_results, system_error;
    system_status = self_test_results = system_error = 0;
    bno.getSystemStatus(&system_status, &self_test_results, &system_error);

    /* Display the results in the Serial Monitor */
    Serial.println("");
    Serial.print("System Status: 0x");
    Serial.println(system_status, HEX);
    Serial.print("Self Test:     0x");
    Serial.println(self_test_results, HEX);
    Serial.print("System Error:  0x");
    Serial.println(system_error, HEX);
    Serial.println("");
    delay(500);
}

/**************************************************************************/
/*
    Display sensor calibration status
    */
/**************************************************************************/
void displayCalStatus(void)
{
    /* Get the four calibration values (0..3) */
    /* Any sensor data reporting 0 should be ignored, */
    /* 3 means 'fully calibrated" */
    uint8_t system, gyro, accel, mag;
    system = gyro = accel = mag = 0;
    bno.getCalibration(&system, &gyro, &accel, &mag);

    /* The data should be ignored until the system calibration is > 0 */
    Serial.print("\t");
    if (!system)
    {
        Serial.print("! ");
    }

    /* Display the individual values */
    Serial.print("Sys:");
    Serial.print(system, DEC);
    Serial.print(" G:");
    Serial.print(gyro, DEC);
    Serial.print(" A:");
    Serial.print(accel, DEC);
    Serial.print(" M:");
    Serial.print(mag, DEC);
}

/**************************************************************************/
/*
    Display the raw calibration offset and radius data
    */
/**************************************************************************/
void displaySensorOffsets(const adafruit_bno055_offsets_t &calibData)
{
    Serial.print("Accelerometer: ");
    Serial.print(calibData.accel_offset_x); Serial.print(" ");
    Serial.print(calibData.accel_offset_y); Serial.print(" ");
    Serial.print(calibData.accel_offset_z); Serial.print(" ");

    Serial.print("\nGyro: ");
    Serial.print(calibData.gyro_offset_x); Serial.print(" ");
    Serial.print(calibData.gyro_offset_y); Serial.print(" ");
    Serial.print(calibData.gyro_offset_z); Serial.print(" ");

    Serial.print("\nMag: ");
    Serial.print(calibData.mag_offset_x); Serial.print(" ");
    Serial.print(calibData.mag_offset_y); Serial.print(" ");
    Serial.print(calibData.mag_offset_z); Serial.print(" ");

    Serial.print("\nAccel Radius: ");
    Serial.print(calibData.accel_radius);

    Serial.print("\nMag Radius: ");
    Serial.print(calibData.mag_radius);
}


bool saveSensorCalibration(long id, adafruit_bno055_offsets_t *calibrationData, char*configFileName) {
    if (!SD.begin(4)) {
      Serial.println(F("Initialization from SD Card failed, reverting to factory settings."));
      return false;
    }
    File configStream = SD.open(configFileName, FILE_WRITE);
    if (! configStream) {
      return false;
    } else {
      unsigned long endOF = configStream.size();
      configStream.seek(endOF);
      configStream.print("id:");
      configStream.println(id);
      configStream.print("accel_offset_x:");
      configStream.println(calibrationData->accel_offset_x);
      configStream.print("accel_offset_y:");
      configStream.println(calibrationData->accel_offset_y);
      configStream.print("accel_offset_z:");
      configStream.println(calibrationData->accel_offset_z);
      configStream.print("gyro_offset_x:");
      configStream.println(calibrationData->gyro_offset_x);
      configStream.print("gyro_offset_y:");
      configStream.println(calibrationData->gyro_offset_y);
      configStream.print("gyro_offset_z:");
      configStream.println(calibrationData->gyro_offset_z);
      configStream.print("mag_offset_x:");
      configStream.println(calibrationData->mag_offset_x);
      configStream.print("mag_offset_y:");
      configStream.println(calibrationData->mag_offset_y);
      configStream.print("mag_offset_z:");
      configStream.println(calibrationData->mag_offset_z);
      configStream.print("accel_radius:");
      configStream.println(calibrationData->accel_radius);
      configStream.print("mag_radius:");
      configStream.println(calibrationData->mag_radius);
      configStream.close();
      return true;
    }
}
 

#define MAX_LINE_LENGTH 80
#define Escape 0x10
#define CarrageReturn 0x0A
#define NewLine 0x0D
#define Space 0x20
#define Del 0x7F

bool loadSensorCalibraton(long id, adafruit_bno055_offsets_t *calibrationData, char*configFileName) {
    if (!SD.begin(4)) {
      Serial.println(F("Initialization from SD Card failed, reverting to factory settings."));
      return false;
    }
    // re-open the file for reading:
    File configStream = SD.open(configFileName);
    if ( !configStream ) {
      // if the file didn't open, print an error:
      Serial.print(F("error opening "));
      Serial.println(configFileName);
      return false;
    }
    int lineno = 0, fbp=0;
    long currentId = -1, foundId = -1;
    char readBuffer[MAX_LINE_LENGTH+1];
    char lastChar;
    // read from the file until there's nothing else in it:
    // configuration sets are preceded by id:<sensor ID>,
    // there can be many configuration sets in the file.
    // only the last matching the id is used.
    // this allows for re-confguration without loosing previous attempts but does append the file.
    while (configStream.available() && (lastChar = configStream.read() != -1)) {
      switch(lastChar) {
        case NewLine:
          readBuffer[fbp++] = 0;
          lineno++;
          if ( strncmp(readBuffer,"id:",3) == 0 ) {
            currentId = atol(&readBuffer[3]);
            if ( currentId == id ) {
              Serial.println("Loading Config for ");
              Serial.println(currentId);
            } else {
              Serial.println("Ignoring Config for ");
              Serial.println(currentId);
            }
          } else if ( currentId == id ) {
            if ( strncmp(readBuffer,"accel_offset_x:",15) == 0) {
              foundId = currentId;
              calibrationData->accel_offset_x = atol(&readBuffer[15]);
            } else if ( strncmp(readBuffer,"accel_offset_y:",15) == 0){
              foundId = currentId;
              calibrationData->accel_offset_y = atol(&readBuffer[15]);
            } else if ( strncmp(readBuffer,"accel_offset_z:",15) == 0) {
              foundId = currentId;
              calibrationData->accel_offset_z = atol(&readBuffer[15]);
            } else if ( strncmp(readBuffer,"gyro_offset_x:",14) == 0) {
              foundId = currentId;
              calibrationData->gyro_offset_x = atol(&readBuffer[14]);
            } else if ( strncmp(readBuffer,"gyro_offset_y:",14) == 0) {
              foundId = currentId;
              calibrationData->gyro_offset_y = atol(&readBuffer[14]);
            } else if ( strncmp(readBuffer,"gyro_offset_z:",14) == 0) {
              foundId = currentId;
              calibrationData->gyro_offset_z = atol(&readBuffer[14]);
            } else if ( strncmp(readBuffer,"mag_offset_x:",13) == 0) {
              foundId = currentId;
              calibrationData->mag_offset_x = atol(&readBuffer[13]);
            } else if ( strncmp(readBuffer,"mag_offset_y:",13) == 0) {
              foundId = currentId;
              calibrationData->mag_offset_y = atol(&readBuffer[13]);
            } else if ( strncmp(readBuffer,"mag_offset_z:",13) == 0) {
              foundId = currentId;
              calibrationData->mag_offset_z = atol(&readBuffer[13]);
            } else if ( strncmp(readBuffer,"accel_radius:",13) == 0) {
              foundId = currentId;
              calibrationData->accel_radius = atol(&readBuffer[13]);
            } else if ( strncmp(readBuffer,"mag_radius:",12) == 0) {
              foundId = currentId;
              calibrationData->mag_radius = atol(&readBuffer[12]);
            }
          }
          fbp = 0;
          break;
        default:
          if (lastChar >= Space && lastChar < Del && fbp < MAX_LINE_LENGTH) {
            readBuffer[fbp++] = lastChar;
          }
      }
    }
    configStream.close();
    return (foundId == id);
}

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
    */
/**************************************************************************/
void setup(void)
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Orientation Sensor Test"); Serial.println("");

    /* Initialise the sensor */
    if (!bno.begin())
    {
        /* There was a problem detecting the BNO055 ... check your connections */
        Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
        while (1);
    }

    int eeAddress = 0;
    long bnoID;
    bool foundCalib = false;


    adafruit_bno055_offsets_t calibrationData;
    sensor_t sensor;
    bno.getSensor(&sensor);
    if ( !loadSensorCalibraton(sensor.sensor_id, &calibrationData, "imucfg.txt")) {
        Serial.println("\nNo Calibration Data for this sensor exists on the SD card.");
        delay(500);
    } else {
        displaySensorOffsets(calibrationData);

        Serial.println("\n\nRestoring Calibration data to the BNO055...");
        bno.setSensorOffsets(calibrationData);

        Serial.println("\n\nCalibration data loaded into BNO055");
        foundCalib = true;
    }

    delay(1000);

    /* Display some basic information on this sensor */
    displaySensorDetails();

    /* Optional: Display current status */
    displaySensorStatus();

    bno.setExtCrystalUse(true);

    sensors_event_t event;
    bno.getEvent(&event);
    if (foundCalib){
        Serial.println("Move sensor slightly to calibrate magnetometers");
        while (!bno.isFullyCalibrated())
        {
            bno.getEvent(&event);
            delay(BNO055_SAMPLERATE_DELAY_MS);
        }
    }
    else
    {
        Serial.println("Please Calibrate Sensor: ");
        while (!bno.isFullyCalibrated())
        {
            bno.getEvent(&event);

            Serial.print("X: ");
            Serial.print(event.orientation.x, 4);
            Serial.print("\tY: ");
            Serial.print(event.orientation.y, 4);
            Serial.print("\tZ: ");
            Serial.print(event.orientation.z, 4);

            /* Optional: Display calibration status */
            displayCalStatus();

            /* New line for the next sample */
            Serial.println("");

            /* Wait the specified delay before requesting new data */
            delay(BNO055_SAMPLERATE_DELAY_MS);
        }
    }

    Serial.println("\nFully calibrated!");
    Serial.println("--------------------------------");
    Serial.println("Calibration Results: ");
    adafruit_bno055_offsets_t newCalib;
    bno.getSensorOffsets(newCalib);
    displaySensorOffsets(newCalib);

    Serial.println("\n\nStoring calibration data to EEPROM...");

    eeAddress = 0;
    bno.getSensor(&sensor);
    bnoID = sensor.sensor_id;

    saveSensorCalibration(bnoID, &newCalib, "imucfg.txt");

    Serial.println("\n--------------------------------\n");
    delay(500);
}

void loop() {
    /* Get a new sensor event */
    sensors_event_t event;
    bno.getEvent(&event);

    /* Display the floating point data */
    Serial.print("X: ");
    Serial.print(event.orientation.x, 4);
    Serial.print("\tY: ");
    Serial.print(event.orientation.y, 4);
    Serial.print("\tZ: ");
    Serial.print(event.orientation.z, 4);

    /* Optional: Display calibration status */
    displayCalStatus();

    /* Optional: Display sensor status (debug only) */
    //displaySensorStatus();

    /* New line for the next sample */
    Serial.println("");

    /* Wait the specified delay before requesting new data */
    delay(BNO055_SAMPLERATE_DELAY_MS);
}




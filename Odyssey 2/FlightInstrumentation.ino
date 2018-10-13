#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_BMP085_U.h>
#include <utility/imumaths.h>
#include <SPI.h>
#include <SD.h>

adafruit_bno055_offsets_t calibValues;

#define cardSelect 4
File logfile;
Adafruit_BNO055 bno = Adafruit_BNO055();
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
int CCPT;
int initTime = 0;
const int numDataPoints = 23;
String stringOfValues[numDataPoints];
String strPrint = "";

void printDataToSD();
void printDataToSerial1();

void printDataToSD()
{
  
  for (int i = 0; i < numDataPoints; i++)
  {
    strPrint += stringOfValues[i];
    if (i < numDataPoints - 1)
    {
      strPrint += ",";
    }
  }
  logfile.println(strPrint);
  logfile.flush();
  strPrint = "";
}

void printDataToSerial1()
{
  strPrint += stringOfValues[22];
  strPrint += "\n";
  Serial1.print(strPrint);
  strPrint = "";
}

void setup(void)
{
  Serial1.begin(9600);
  Serial.begin(9600);

////////////////////////////////////////////////////////////////////////////////
  //LOGGING SETUP:
  if (!SD.begin(cardSelect)) 
  {
    //Serial.println("Card init. failed!");
  }

  char filename[20];
  strcpy(filename, "FLIGHT00.TXT");
  for (uint8_t i = 0; i < 100; i++) 
  {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  logfile = SD.open(filename, O_CREAT | O_WRITE);
  if( ! logfile ) 
  {
    //Serial.print("Couldnt create "); 
    //Serial.println(filename);
  }

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
  //BMP (altimeter) SETUP:
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    //Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    //while(1);
  }

/////////////////////////////////////////////////////////////////////////////////////
  //BNO (IMU) SETUP:
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    //Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    //while(1);
  }

  int8_t temp = bno.getTemp();

  bno.setExtCrystalUse(true);

  calibValues.accel_offset_x = -24;
  calibValues.accel_offset_y = -30;
  calibValues.accel_offset_z = -14;
  calibValues.mag_offset_x = 113;
  calibValues.mag_offset_y = 131;
  calibValues.mag_offset_z = -174;
  calibValues.gyro_offset_x = -1;
  calibValues.gyro_offset_y = 3;
  calibValues.gyro_offset_z = 0 ;
  calibValues.accel_radius = 1000;
  calibValues.mag_radius = 509;

  bno.setSensorOffsets(calibValues);


}

void loop(void)
{ 
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  stringOfValues[0] = String(millis());

  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //For CC PT:

  CCPT = analogRead(A0);
  stringOfValues[1] = String(CCPT);
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //For BNO:
  // Possible vector values can be:
  // - VECTOR_ACCELEROMETER - m/s^2
  // - VECTOR_MAGNETOMETER  - uT
  // - VECTOR_GYROSCOPE     - rad/s
  // - VECTOR_EULER         - degrees
  // - VECTOR_LINEARACCEL   - m/s^2
  // - VECTOR_GRAVITY       - m/s^2
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  imu::Vector<3> magnet = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
  imu::Vector<3> gyros = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  imu::Vector<3> grav = bno.getVector(Adafruit_BNO055::VECTOR_GRAVITY);
  imu::Vector<3> accelo = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  imu::Vector<3> linaccelo = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  imu::Quaternion quat = bno.getQuat();
  
  /* Display the floating point data */
  stringOfValues[2] = String(quat.w());
  stringOfValues[3] = String(quat.x());
  stringOfValues[4] = String(quat.y());
  stringOfValues[5] = String(quat.z());

  stringOfValues[6] = String(accelo.x());
  stringOfValues[7] = String(accelo.y());
  stringOfValues[8] = String(accelo.z());

  stringOfValues[9] = String(linaccelo.x());
  stringOfValues[10] = String(linaccelo.y());
  stringOfValues[11] = String(linaccelo.z());

  stringOfValues[12] = String(grav.x());
  stringOfValues[13] = String(grav.y());
  stringOfValues[14] = String(grav.z());

  stringOfValues[15] = String(gyros.x());
  stringOfValues[16] = String(gyros.y());
  stringOfValues[17] = String(gyros.z());

  stringOfValues[18] = String(euler.x());
  stringOfValues[19] = String(euler.y());
  stringOfValues[20] = String(euler.z());
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//IMU Calibration

  uint8_t system, gyro, accel, mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);
  /* Display calibration status for each sensor. */
  /*Serial1.print("CALIBRATION: Sys=");
  Serial1.print(system, DEC);
  Serial1.print(" Gyro=");
  Serial1.print(gyro, DEC);
  Serial1.print(" Accel=");
  Serial1.print(accel, DEC);
  Serial1.print(" Mag=");
  Serial1.println(mag, DEC);*/
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //For BMP Altimeter:
  sensors_event_t event;
  bmp.getEvent(&event);
  
  event.pressure;
  float temperature;
  bmp.getTemperature(&temperature);

  float pressureInhPA = event.pressure;// * 0.0145037738;

  stringOfValues[21] = String(pressureInhPA);
  
  float seaLevelPressure = 1012.00;
  stringOfValues[22] = String(bmp.pressureToAltitude(seaLevelPressure, event.pressure) * 3.28084);

  //Serial.println(millis() - millisStart);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Printing to SD and to Serial1:
  printDataToSD();
  printDataToSerial1(); 
  
  /*Serial.print("Quatw: ");
  Serial.print(stringOfValues[2]);
  Serial.print(" Quatx: ");
  Serial.print(stringOfValues[3]);
  Serial.print(" Quaty: ");
  Serial.print(stringOfValues[4]);
  Serial.print(" Quatz: ");
  Serial.print(stringOfValues[5]);

  Serial.print(" Eulerx: ");
  Serial.print(stringOfValues[18]);
  Serial.print(" Eulery: ");
  Serial.print(stringOfValues[19]);
  Serial.print(" Eulerz: ");
  Serial.print(stringOfValues[20]);*/
  
}

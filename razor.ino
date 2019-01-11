#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library
//#include <Wire.h> // Depending on your Arduino version, you may need to include Wire.h

#define LOG_PORT SERIAL_PORT_USBVIRTUAL
#define SERIAL_BAUD_RATE 115200 // Serial port baud
#define INTERRUPT_PIN 4

MPU9250_DMP razor; // Create an instance of the MPU9250_DMP class

int timestamp = 0;

void setup()
{
  LOG_PORT.begin(SERIAL_BAUD_RATE);
  if (razor.begin() != INV_SUCCESS)
  {
  }

  // Use setSensors to turn on or off MPU-9250 sensors.
  // Any of the following defines can be combined:
  // INV_XYZ_GYRO, INV_XYZ_ACCEL, INV_XYZ_COMPASS,
  // INV_X_GYRO, INV_Y_GYRO, or INV_Z_GYRO
  razor.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS); // Enable all sensors

  // Use setGyroFSR() and setAccelFSR() to configure the
  // gyroscope and accelerometer full scale ranges.
  // Gyro options are +/- 250, 500, 1000, or 2000 dps
  razor.setGyroFSR(2000); // Set gyro to 2000 dps
  // Accel options are +/- 2, 4, 8, or 16 g
  razor.setAccelFSR(16); // Set accel to +/-2g

  // setLPF() can be used to set the digital low-pass filter
  // of the accelerometer and gyroscope.
  // Can be any of the following: 188, 98, 42, 20, 10, 5
  // (values are in Hz).
  razor.setLPF(5); // Set LPF corner frequency to 5Hz

  // The sample rate of the accel/gyro can be set using
  // setSampleRate. Acceptable values range from 4Hz to 1kHz
  razor.setSampleRate(1000); // Set sample rate to 10Hz

  // Likewise, the compass (magnetometer) sample rate can be
  // set using the setCompassSampleRate() function.
  // This value can range between: 1-100Hz
  razor.setCompassSampleRate(10); // Set mag rate to 10Hz
  
  pinMode(INTERRUPT_PIN, INPUT_PULLUP); // Set interrupt as an input w/ pull-up resistor
  // Use enableInterrupt() to configure the MPU-9250's 
  // interrupt output as a "data ready" indicator.
  razor.enableInterrupt();

  // The interrupt level can either be active-high or low. Configure as active-low.
  // Options are INT_ACTIVE_LOW or INT_ACTIVE_HIGH
  razor.setIntLevel(INT_ACTIVE_LOW);

  // The interrupt can be set to latch until data is read, or as a 50us pulse.
  // Options are INT_LATCHED or INT_50US_PULSE
  razor.setIntLatched(INT_LATCHED);
}

void loop() {
  if ( digitalRead(INTERRUPT_PIN) == LOW ) // If MPU-9250 interrupt fires (active-low)
  {
    razor.update(); // Update all sensor's
    // ... do stuff with imu.ax, imu.ay, etc.
    float accelX = razor.calcAccel(razor.ax); // accelX is x-axis acceleration in g's
    float accelY = razor.calcAccel(razor.ay); // accelY is y-axis acceleration in g's
    float accelZ = razor.calcAccel(razor.az); // accelZ is z-axis acceleration in g's

    float gyroX = razor.calcGyro(razor.gx); // gyroX is x-axis rotation in dps
    float gyroY = razor.calcGyro(razor.gy); // gyroY is y-axis rotation in dps
    float gyroZ = razor.calcGyro(razor.gz); // gyroZ is z-axis rotation in dps

    float magX = razor.calcMag(razor.mx); // magX is x-axis magnetic field in uT
    float magY = razor.calcMag(razor.my); // magY is y-axis magnetic field in uT
    float magZ = razor.calcMag(razor.mz); // magZ is z-axis magnetic field in uT
    LOG_PORT.println(millis() - timestamp);
    timestamp = millis();
  }
}

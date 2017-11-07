#include <Adafruit_MAX31856.h>

int TCArray[3];
int TCCount = 3; //CHANGE TCCount TO EQUAL NUMBER OF THERMOCOUPLES (3 for FSM, 7 for GSM)
int csPinCount[] = {7, 8, 9};
int SDI = 11;
int SDO = 12;
int SCLK = 13;

//FUcked: 0 , 1, 11, 12, 6,
// This array has the pins whose voltages will be measured
int pins[3] = {A0, A1, A2};
// This array will be the one we send out that has FSM/GCM,
// and the voltage values
int PTVol[4];
// This int is used to loop through all of the pins taking
// analog inputs and store the data in the correct spot in
// PTVol
int currentPin = 0;
// isFSM is used if the Arduino is a flight sensor module, 
// otherwise use 0 for PTVol[0]
//const int isFSM = 1;
//const int isGSM = 0;

// Use software SPI: (CS, DI, DO, CLK)
// use hardware SPI, just pass in the CS pin
//Adafruit_MAX31856 max = Adafruit_MAX31856(10);

void setup() {
  
  //PTVol[0] = isFSM;
  Serial.begin(9600);
  
//Sets up thermocouples connected to pins 7,8,9

  for (int i = 0; i < TCCount; i++){
    Adafruit_MAX31856 max = Adafruit_MAX31856(csPinCount[i], SDI, SDO, SCLK);
  Serial.begin(9600);

  max.begin();

//Set thermocouple type to type J
  max.setThermocoupleType(MAX31856_TCTYPE_J);

  /*Serial.print("Thermocouple type: ");
  switch ( max.getThermocoupleType() ) {
    case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
    case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
    case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
    case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
    case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
    case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
    case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
    case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
    case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
    case MAX31856_VMODE_G32: Serial.println("Voltage x8 Gain mode"); break;
    default: Serial.println("Unknown"); break;
  }*/
  max.Config();
  }


}

void loop() {

  for (currentPin = 0; currentPin <= 2; currentPin++){
    PTVol[currentPin+2] = analogRead(pins[currentPin]);
  }

  for (int i = 0; i < TCCount; i++){
    Adafruit_MAX31856 max = Adafruit_MAX31856(csPinCount[i], SDI, SDO, SCLK);

//define variable TCT as an int (length of 2 bytes) and sets equal to temperature reading from Thermocouple
    int TCT = max.readThermocoupleTemperature();

    ::TCArray[i] = TCT;

/*/Gets each of 2 bytes from TCT and assigns them each to a position in a byte array
    byte byteArray[2];
    byteArray[0] = (TCT >> 8) & 255;
    byteArray[1] = (TCT >> 0) & 255;

//For the thermocoupler connected to current pin, sets the byteArray it produces to position i of TCArray
    for (int j = 0; j < 2; j++){
      ::TCArray[i][j] = byteArray[j];
      }*/

//Check and print any faults
    uint8_t fault = max.readFault();
    if (fault) {
      if (fault & MAX31856_FAULT_CJRANGE) Serial.print("Cold Junction Range Fault");
      if (fault & MAX31856_FAULT_TCRANGE) Serial.print("Thermocouple Range Fault");
      if (fault & MAX31856_FAULT_CJHIGH)  Serial.print("Cold Junction High Fault");
      if (fault & MAX31856_FAULT_CJLOW)   Serial.print("Cold Junction Low Fault");
      if (fault & MAX31856_FAULT_TCHIGH)  Serial.print("Thermocouple High Fault");
      if (fault & MAX31856_FAULT_TCLOW)   Serial.print("Thermocouple Low Fault");
      if (fault & MAX31856_FAULT_OVUV)    Serial.print("Over/Under Voltage Fault");
      if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
      }
  }

  

//Prints each of the parts of TCArray, which includes the byteArray from each of the thermocouples in the order they are evaluated (10, 9, 8, etc.)
  Serial.print(millis());
  Serial.print(",");
  Serial.print(PTVol[2]);
  Serial.print(",");
  Serial.print(PTVol[3]);
  Serial.print(",");
  Serial.print(PTVol[4]);

  for (int i = 0; i < TCCount; i++){
    Serial.print(",");
    Serial.print(::TCArray[i]);
  }
    Serial.println(" ");
}


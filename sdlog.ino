#include <SdFat.h>
SdFatSdioEX sd;
File file;
char filename[11] = "log000.txt";
int led = 13;
int starttime = 0;
bool first = true;
bool counter = false;
int halfsecond = 0;

void setup() {

  // wait for teensy
  Serial.begin(9600);

  // activate led
  pinMode(led, OUTPUT); 
  digitalWrite(led, HIGH); 
  
  // Initialize SdFat or print a detailed error message and halt
  if (!sd.begin())
  {
    sd.initErrorHalt();
  }

  int filecount = 0;

  while (sd.exists(filename))
  {
    filecount++;
    int hundredth = filecount / 100 % 10;
    int tenth = filecount / 10 % 10;
    int one = filecount % 10;
    filename[3] = hundredth + '0';
    filename[4] = tenth + '0';
    filename[5] = one + '0';
  }
  
  if (!file.open(filename, O_WRITE | O_CREAT)) {
    sd.errorHalt("opening test.txt for write failed");
  }
}

void loop() {
  if (first)
  {
    starttime = millis();
    first = false;
  }
  if ((millis() - starttime) < 10000)
  {
  if (!counter)
  {
    halfsecond = millis();
    counter = true;
  }
  file.println(filename);
  
  if ((millis() - halfsecond) >= 50)
  {
    file.flush();
    counter = false;
  }
  }
  else
  {
    file.flush();
    digitalWrite(led, LOW);
    Serial.println(filename);
    delay(1000);
  }
}

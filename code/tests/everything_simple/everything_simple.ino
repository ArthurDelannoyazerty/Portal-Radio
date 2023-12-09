// DS3231
#include <RTClib.h>
#include <Wire.h>
RTC_DS3231 rtc;
char t[32];

// TFT
#include <TFT.h>  
#include <SPI.h>
#define cs   10
#define dc   9
#define rst  8
TFT TFTscreen = TFT(cs, dc, rst);

// MP3-TF-16P V3.0
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/6, /*tx =*/7);
#define FPSerial softSerial
DFRobotDFPlayerMini myDFPlayer;

void setup() {
  Serial.begin(9600);

  // DS3231
  Wire.begin();
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));     // Uncomment to set current time

  // TFT screen
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);
  TFTscreen.setTextSize(2);

  // MP3 player
  FPSerial.begin(9600);
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin: Please recheck the connection or insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Good");
}

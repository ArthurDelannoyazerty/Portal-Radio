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

// PROJECT
#define position_hour 5,5
#define position_minute 5,55
#define position_second 5,105

#define relay_pin 2

#define sw_pin  5
#define dt_pin  4
#define clk_pin 3

// ---------------------------------------
char charHour[2]; 
char charMinute[2]; 
char charSecond[2]; 

int previous_hour;
int previous_minute;
int previous_second;

int compteur_rotary = 0;
int etatPrecedentLigneSW;
int etatPrecedentLigneCLK;


void setup() {
  Serial.begin(9600);

  // DS3231
  Wire.begin();
  rtc.begin();
  // rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));     // Uncomment to set current time


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

  
  //////////////////////////// INIT TIME ON SCREEN ////////////////////////////////////////
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  TFTscreen.stroke(255, 255, 255);
  TFTscreen.setCursor(5, 5);
  TFTscreen.println(hour);
  TFTscreen.setCursor(5, 55);
  TFTscreen.println(minute);
  TFTscreen.setCursor(5, 105);
  TFTscreen.println(second);

  previous_hour = hour;
  previous_minute = minute;
  previous_second = second;


  //////////////////////////// MUSIC ////////////////////////////////////////
  
  pinMode(relay_pin, OUTPUT);    // Relay activation
  power_to_speaker(true);

  //////////////////////////// POTENTIOMETRE//////////////////////////////////
    pinMode(sw_pin, INPUT_PULLUP);
    pinMode(dt_pin, INPUT);
    pinMode(clk_pin, INPUT);

    etatPrecedentLigneSW  = digitalRead(sw_pin);
    etatPrecedentLigneCLK = digitalRead(clk_pin);

    Serial.println(myDFPlayer.readFolderCounts());
    Serial.println(myDFPlayer.readFileCounts());
    
    myDFPlayer.volume(10);  //Set volume value. From 0 to 30
    myDFPlayer.play();
    power_to_speaker(true);
}

void loop() {
  // delay(1000);
  display_time_tft();
  refresh_compteur_rotary();
}

void refresh_compteur_rotary(){
    int etatActuelDeLaLigneCLK = digitalRead(clk_pin);
    int etatActuelDeLaLigneSW  = digitalRead(sw_pin);
    int etatActuelDeLaLigneDT  = digitalRead(dt_pin);

    if(etatActuelDeLaLigneSW != etatPrecedentLigneSW) {
        if(etatActuelDeLaLigneSW == LOW)  Serial.println(F("Bouton SW appuyé"));
        else                              Serial.println(F("Bouton SW relâché"));
        delay(10);
    }
    if(etatActuelDeLaLigneCLK != etatPrecedentLigneCLK) {
      etatPrecedentLigneCLK = etatActuelDeLaLigneCLK;
      if(etatActuelDeLaLigneCLK == LOW) {
        if(etatActuelDeLaLigneCLK != etatActuelDeLaLigneDT) {
            compteur_rotary--;
            Serial.println(compteur_rotary);
        }
        else {
            compteur_rotary++;
            Serial.println(compteur_rotary);
        }
        delay(1);
      }
    }
}

void power_to_speaker(bool power){
  power ? digitalWrite(relay_pin, HIGH) : digitalWrite(relay_pin, LOW);
}


void display_time_tft(){
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  if (hour != previous_hour){
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.setCursor(position_hour);
    TFTscreen.println(previous_hour);
    TFTscreen.stroke(255, 255, 255);
    TFTscreen.setCursor(position_hour);
    TFTscreen.println(hour);
    previous_hour = hour;
  }

  if (minute != previous_minute){
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.setCursor(position_minute);
    TFTscreen.println(previous_minute);
    TFTscreen.stroke(255, 255, 255);
    TFTscreen.setCursor(position_minute);
    TFTscreen.println(minute);
    previous_minute = minute;
  }

  if (second != previous_second){
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.setCursor(position_second);
    TFTscreen.println(previous_second);
    TFTscreen.stroke(255, 255, 255);
    TFTscreen.setCursor(position_second);
    TFTscreen.println(second);
    previous_second = second;
  }
}


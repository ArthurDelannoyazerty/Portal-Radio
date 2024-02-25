// DS3231
#include <RTClib.h>
#include <Wire.h>
RTC_DS3231 rtc;
char t[32];

// TFT
#include <TFT.h>
#include <SPI.h>
#define cs 10
#define dc 9
#define rst 8
TFT TFTscreen = TFT(cs, dc, rst);

// MP3-TF-16P V3.0
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/6, /*tx =*/7);
#define FPSerial softSerial
DFRobotDFPlayerMini myDFPlayer;

// PROJECT
#define position_hour_tens 5, 50
#define position_hour_unit 35, 50
#define position_double_points 65, 50
#define position_minute_tens 95, 50
#define position_minute_unit 125, 50

#define relay_pin 2

#define sw_pin 5
#define dt_pin 4
#define clk_pin 3

#define button_pin 12

#define tft_background_color 0, 0, 0
#define tft_text_color 222, 222, 73
#define tft_text_color_darker 88, 88, 16

// ---------------------------------------
char charHour[2];
char charMinute[2];
char charSecond[2];

int previous_hour_tens = 0;
int previous_hour_unit = 0;
int previous_minute_tens = 0;
int previous_minute_unit = 0;
int previous_seconds = 0;
bool double_dots_darker = true;

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
  TFTscreen.background(tft_background_color);
  TFTscreen.setTextSize(5);

  // MP3 player
  FPSerial.begin(9600);
  if (!myDFPlayer.begin(FPSerial, /*isACK = */ true, /*doReset = */ true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin: Please recheck the connection or insert the SD card!"));
    while (true) {
      delay(0);  // Code to compatible with ESP8266 watch dog.
    }
  }

  //////////////////////////// MUSIC ////////////////////////////////////////

  pinMode(relay_pin, OUTPUT);  // Relay activation
  power_to_speaker(false);

  //////////////////////////// POTENTIOMETRE//////////////////////////////////
  pinMode(sw_pin, INPUT_PULLUP);
  pinMode(dt_pin, INPUT);
  pinMode(clk_pin, INPUT);

  etatPrecedentLigneSW = digitalRead(sw_pin);
  etatPrecedentLigneCLK = digitalRead(clk_pin);

  Serial.println(myDFPlayer.readFolderCounts());
  Serial.println(myDFPlayer.readFileCounts());

  //////////////////////////////////////////////////////////////////////////
  pinMode(button_pin, INPUT);
  ////////////////////////////////////////////////////////////////////////////
  // myDFPlayer.volume(10);  //Set volume value. From 0 to 30
  // myDFPlayer.play();

  //////////////////////////////////////////////////////////////////////////

  int clock_logo[51][2] = {
    { 0, 4 },
    { 0, 5 },
    { 0, 6 },
    { 0, 7 },
    { 1, 2 },
    { 1, 3 },
    { 1, 4 },
    { 1, 7 },
    { 1, 8 },
    { 1, 9 },
    { 2, 1 },
    { 2, 2 },
    { 2, 9 },
    { 2, 10 },
    { 3, 1 },
    { 3, 5 },
    { 3, 10 },
    { 4, 0 },
    { 4, 1 },
    { 4, 5 },
    { 4, 10 },
    { 4, 11 },
    { 5, 0 },
    { 5, 5 },
    { 5, 11 },
    { 6, 0 },
    { 6, 5 },
    { 6, 6 },
    { 6, 7 },
    { 6, 8 },
    { 6, 11 },
    { 7, 0 },
    { 7, 1 },
    { 7, 10 },
    { 7, 11 },
    { 8, 1 },
    { 8, 10 },
    { 9, 1 },
    { 9, 2 },
    { 9, 9 },
    { 9, 10 },
    { 10, 2 },
    { 10, 3 },
    { 10, 4 },
    { 10, 6 },
    { 10, 7 },
    { 10, 8 },
    { 11, 4 },
    { 11, 5 },
    { 11, 6 },
    { 11, 7 }
  };

  TFTscreen.stroke(tft_text_color);
  for (int i=0; i<52; i++){
    TFTscreen.point(10+clock_logo[i][1], 10+clock_logo[i][0]);
  }

}

void loop() {
  // delay(1000);
  display_time_tft();
  refresh_compteur_rotary();
  Serial.println(digitalRead(button_pin));
}

void refresh_compteur_rotary() {
  int etatActuelDeLaLigneCLK = digitalRead(clk_pin);
  int etatActuelDeLaLigneSW = digitalRead(sw_pin);
  int etatActuelDeLaLigneDT = digitalRead(dt_pin);

  if (etatActuelDeLaLigneSW != etatPrecedentLigneSW) {
    if (etatActuelDeLaLigneSW == LOW) Serial.println(F("Bouton SW appuyé"));
    else Serial.println(F("Bouton SW relâché"));
    delay(10);
  }
  if (etatActuelDeLaLigneCLK != etatPrecedentLigneCLK) {
    etatPrecedentLigneCLK = etatActuelDeLaLigneCLK;
    if (etatActuelDeLaLigneCLK == LOW) {
      if (etatActuelDeLaLigneCLK != etatActuelDeLaLigneDT) {
        compteur_rotary--;
        Serial.println(compteur_rotary);
      } else {
        compteur_rotary++;
        Serial.println(compteur_rotary);
      }
      delay(1);
    }
  }
}


void power_to_speaker(bool power) {
  power ? digitalWrite(relay_pin, HIGH) : digitalWrite(relay_pin, LOW);
}


void display_time_tft() {
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  int hour_tens = hour / 10;
  int hour_unit = hour % 10;
  int minute_tens = minute / 10;
  int minute_unit = minute % 10;

  if (hour_tens != previous_hour_tens) {
    TFTscreen.stroke(tft_background_color);
    TFTscreen.setCursor(position_hour_tens);
    TFTscreen.println(previous_hour_tens);
    TFTscreen.stroke(tft_text_color);
    TFTscreen.setCursor(position_hour_tens);
    TFTscreen.println(hour_tens);
    previous_hour_tens = hour_tens;
  }
  if (hour_unit != previous_hour_unit) {
    TFTscreen.stroke(tft_background_color);
    TFTscreen.setCursor(position_hour_unit);
    TFTscreen.println(previous_hour_unit);
    TFTscreen.stroke(tft_text_color);
    TFTscreen.setCursor(position_hour_unit);
    TFTscreen.println(hour_unit);
    previous_hour_unit = hour_unit;
  }

  if (minute_tens != previous_minute_tens) {
    TFTscreen.stroke(tft_background_color);
    TFTscreen.setCursor(position_minute_tens);
    TFTscreen.println(previous_minute_tens);
    TFTscreen.stroke(tft_text_color);
    TFTscreen.setCursor(position_minute_tens);
    TFTscreen.println(minute_tens);
    previous_minute_tens = minute_tens;
  }
  if (minute_unit != previous_minute_unit) {
    TFTscreen.stroke(tft_background_color);
    TFTscreen.setCursor(position_minute_unit);
    TFTscreen.println(previous_minute_unit);
    TFTscreen.stroke(tft_text_color);
    TFTscreen.setCursor(position_minute_unit);
    TFTscreen.println(minute_unit);
    previous_minute_unit = minute_unit;
  }

  if (second != previous_seconds) {
    second % 2 == 0 ? TFTscreen.stroke(tft_text_color) : TFTscreen.stroke(tft_text_color_darker);
    TFTscreen.setCursor(position_double_points);
    TFTscreen.println(":");
    previous_seconds = second;
  }
}

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

#define max_volume_music 25
#define min_volume_music 1

// ---------------------------------------
char charHour[2];
char charMinute[2];
char charSecond[2];

int hour_force_hour_screen = 19;
int minute_force_hour_screen = 13;

int previous_hour_tens   = -1;
int previous_hour_unit   = -1;
int previous_minute_tens = -1;
int previous_minute_unit = -1;
int previous_seconds     = -1;
bool double_dots_darker = true;

int alarm_hour = 7;
int alarm_minute = 30;
bool alarm_active = false;
bool alarm_logo_visible = false;
bool alarm_temporary_disabled = false;
bool alarm_alarming = false;

int volume_music = 11;

int compteur_rotary = 0;
int etatPrecedentLigneSW;
int etatPrecedentLigneCLK;

bool previous_simple_button_state = false;
bool previous_rotary_button_state = false;

bool simple_button_rising = false;
bool rotary_button_rising = false;

bool temp_disable_alarm_override = false;

  int clock_logo[52][2] = {
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
    { 10, 7 },
    { 10, 8 },
    { 10, 9 },
    { 11, 4 },
    { 11, 5 },
    { 11, 6 },
    { 11, 7 }
  };

enum { HOUR_SCREEN,
       INIT_ALARM_SCREEN,
       ALARM_SCREEN,
       INIT_MUSIC_SCREEN,
       MUSIC_SCREEN };
int main_state = HOUR_SCREEN;


bool configuring_alarm = false;

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
  myDFPlayer.volume(volume_music);

  //////////////////////////// MUSIC ////////////////////////////////////////
  pinMode(relay_pin, OUTPUT);  // Relay activation
  power_to_speaker(false);

  //////////////////////////// POTENTIOMETRE//////////////////////////////////
  pinMode(sw_pin, INPUT_PULLUP);
  pinMode(dt_pin, INPUT);
  pinMode(clk_pin, INPUT);

  etatPrecedentLigneSW = digitalRead(sw_pin);
  etatPrecedentLigneCLK = digitalRead(clk_pin);

  //////////////////////////////////////////////////////////////////////////
  pinMode(button_pin, INPUT);
}

void loop() {
  scan_buttons();
  if (!temp_disable_alarm_override) check_alarm();
  switch (main_state) {
    case HOUR_SCREEN:
      if (rotary_button_rising) alarm_active = !alarm_active;
      if (alarm_active  && !alarm_logo_visible) display_clock_logo(true);
      if (!alarm_active &&  alarm_logo_visible) display_clock_logo(false);
      display_current_time_tft();
      if (simple_button_rising){
        if (alarm_alarming) myDFPlayer.next();
        else{
          reset_tft();
          main_state=INIT_ALARM_SCREEN;
        }
      }
      if (alarm_alarming){
        int rotation_volume = get_button_rotation();
        if (rotation_volume!=0){
          volume_music += rotation_volume;
          if (volume_music>max_volume_music) volume_music=max_volume_music;
          if (volume_music<min_volume_music) volume_music=min_volume_music;
          myDFPlayer.volume(volume_music);
        }
      }
      break;
    case INIT_ALARM_SCREEN:
      TFTscreen.setTextSize(2);
      TFTscreen.setCursor(50,20);
      TFTscreen.stroke(tft_text_color);
      TFTscreen.println("ALARM");
      display_alarm_time(-1, false);
      main_state = ALARM_SCREEN;
      break;
    case ALARM_SCREEN:
      if (rotary_button_rising) configuring_alarm = true;
      if (configuring_alarm){
        int index_number_blink = 0;
        while (true){
          delay(1);
          scan_buttons();
          display_alarm_time(index_number_blink, false);
          int rotation = get_button_rotation();
          if (index_number_blink==0){
            alarm_hour += rotation*10;
            if      (alarm_hour<0      && rotation==-1) alarm_hour += 30;
            else if (alarm_hour/10==3  && rotation==1)  alarm_hour -= 30;
          }
          if (index_number_blink==1){
            if (alarm_hour>23) alarm_hour = 23;
            alarm_hour += rotation;
            if      (alarm_hour<0    && rotation==-1) alarm_hour += 24;
            else if (alarm_hour==24  && rotation==1)  alarm_hour -= 24;
          }
          if (index_number_blink==2){
            alarm_minute += rotation*10;
            if      (alarm_minute<0     && rotation==-1) alarm_minute += 60;
            else if (alarm_minute/10==6 && rotation==1)  alarm_minute -= 60;
          }
          if (index_number_blink==3){
            if (alarm_minute>59) alarm_minute = 59;
            alarm_minute += rotation;
            if      (alarm_minute<0    && rotation==-1) alarm_minute += 60;
            else if (alarm_minute==60  && rotation==1)  alarm_minute -= 60;
          }
          if (rotary_button_rising){
            index_number_blink++;
            display_alarm_time(-1, true);
          }
          if (index_number_blink==4){ 
            configuring_alarm = false;
            break;
          }
        }
      }
      if (simple_button_rising){
        reset_tft();
        main_state = INIT_MUSIC_SCREEN; 
      }
      break;
    case INIT_MUSIC_SCREEN:
      TFTscreen.setTextSize(2);
      TFTscreen.stroke(tft_text_color);
      TFTscreen.setCursor(50,20);
      TFTscreen.println("MUSIC");
      TFTscreen.setCursor(25,55);
      TFTscreen.println("VOLUME");
      TFTscreen.setCursor(105,55);
      TFTscreen.println(volume_music);
      temp_disable_alarm_override = true;
      main_state = MUSIC_SCREEN;
      break;
    case MUSIC_SCREEN:
      if (rotary_button_rising){
        if (alarm_alarming){
          power_to_speaker(false);
          myDFPlayer.pause();
          alarm_alarming = false;
        }
        else if (!alarm_alarming){
          power_to_speaker(true);
          myDFPlayer.randomAll();
          alarm_alarming = true;
        }
      }
      int rotation_music = get_button_rotation();
      if (rotation_music==1){
        volume_music += 1;
        if (volume_music>max_volume_music) volume_music=max_volume_music;
        myDFPlayer.volume(volume_music);
        TFTscreen.setTextSize(2);
        TFTscreen.stroke(tft_background_color);
        TFTscreen.setCursor(105,55);
        TFTscreen.println(volume_music-1);
        TFTscreen.stroke(tft_text_color);
        TFTscreen.setCursor(105,55);
        TFTscreen.println(volume_music);
      }
      else if (rotation_music==-1){
        volume_music -= 1;
        if (volume_music<min_volume_music) volume_music=min_volume_music;
        myDFPlayer.volume(volume_music);
        TFTscreen.setTextSize(2);
        TFTscreen.stroke(tft_background_color);
        TFTscreen.setCursor(105,55);
        TFTscreen.println(volume_music+1);
        TFTscreen.stroke(tft_text_color);
        TFTscreen.setCursor(105,55);
        TFTscreen.println(volume_music);
      }
      if (simple_button_rising){
        if (!alarm_alarming){
          reset_tft();
          main_state = HOUR_SCREEN;
          temp_disable_alarm_override = false;
        }
        else{
          myDFPlayer.next();
        }
      }
      break;
  }
}


void force_hour_screen(){
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();
  int seconds = now.second();

  if (main_state!=HOUR_SCREEN && hour==hour_force_hour_screen && minute==minute_force_hour_screen && seconds==58 && !alarm_alarming){
    reset_tft();
    main_state = HOUR_SCREEN;
  }
}

void reset_tft(){
  TFTscreen.background(tft_background_color);
  previous_hour_tens   = -1;
  previous_hour_unit   = -1;
  previous_minute_tens = -1;
  previous_minute_unit = -1;
  previous_seconds     = -1;
  alarm_logo_visible = false;
}


void check_alarm(){
  if (alarm_active){
    DateTime now = rtc.now();
    int hour = now.hour();
    int minute = now.minute();
    int seconds = now.second();

    if (alarm_hour==hour && alarm_minute==minute && seconds==58){
      power_to_speaker(true);
      myDFPlayer.randomAll();
      alarm_alarming = true;
    }
  }
  if (alarm_alarming && rotary_button_rising){
    power_to_speaker(false);
    myDFPlayer.pause();
    alarm_alarming = false;
    alarm_active = false;
  }
}


void scan_buttons(){
  bool simple_button_pressed = is_simple_button_pressed();
  bool rotary_button_pressed = is_rotary_button_pressed();

  simple_button_rising = simple_button_pressed && !previous_simple_button_state;
  rotary_button_rising = rotary_button_pressed && !previous_rotary_button_state;

  previous_simple_button_state = simple_button_pressed;
  previous_rotary_button_state = rotary_button_pressed;
}




void display_clock_logo(bool enable){
  alarm_logo_visible = enable;
  enable ? TFTscreen.stroke(tft_text_color) : TFTscreen.stroke(tft_background_color);
  for (int i = 0; i < 51; i++) {
    TFTscreen.point(10 + clock_logo[i][1], 10 + clock_logo[i][0]);
  }
}


bool is_rotary_button_pressed() {
  int etatActuelDeLaLigneSW = digitalRead(sw_pin);
  bool button_pressed = false;

  if (etatActuelDeLaLigneSW != etatPrecedentLigneSW) {
    if (etatActuelDeLaLigneSW == LOW) button_pressed = true;
    delay(10);
  }
  return button_pressed;
}

bool is_simple_button_pressed(){
  return digitalRead(button_pin);
}

int get_button_rotation() {
  int etatActuelDeLaLigneCLK = digitalRead(clk_pin);
  int etatActuelDeLaLigneDT = digitalRead(dt_pin);
  int rotation = 0;
  if (etatActuelDeLaLigneCLK != etatPrecedentLigneCLK) {
    etatPrecedentLigneCLK = etatActuelDeLaLigneCLK;
    if (etatActuelDeLaLigneCLK == LOW) {
      etatActuelDeLaLigneCLK != etatActuelDeLaLigneDT ? rotation = -1 : rotation = 1;
      delay(1);
    }
  }
  return rotation;
}


void power_to_speaker(bool power) {
  power ? digitalWrite(relay_pin, HIGH) : digitalWrite(relay_pin, LOW);
}


void display_alarm_time(int index_blink, bool force_display){
  DateTime now = rtc.now();
  int second = now.second();

  display_time_tft(alarm_hour/10, alarm_hour%10, alarm_minute/10, alarm_minute%10, second, index_blink, force_display);
}


void display_current_time_tft() {
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  display_time_tft(hour/10, hour%10, minute/10, minute%10, second, -1, false);
}


void display_time_tft(int hour_tens, int hour_unit, int minute_tens, int minute_unit, int second, int index_number_blink, bool force_display){
  TFTscreen.setTextSize(5);
  if (hour_tens != previous_hour_tens || force_display) {    //TODO alarm also
    TFTscreen.stroke(tft_background_color);
    TFTscreen.setCursor(position_hour_tens);
    TFTscreen.println(previous_hour_tens);
    TFTscreen.stroke(tft_text_color);
    TFTscreen.setCursor(position_hour_tens);
    TFTscreen.println(hour_tens);
    previous_hour_tens = hour_tens;
  }
  if (hour_unit != previous_hour_unit || force_display) {
    TFTscreen.stroke(tft_background_color);
    TFTscreen.setCursor(position_hour_unit);
    TFTscreen.println(previous_hour_unit);
    TFTscreen.stroke(tft_text_color);
    TFTscreen.setCursor(position_hour_unit);
    TFTscreen.println(hour_unit);
    previous_hour_unit = hour_unit;
  }

  if (minute_tens != previous_minute_tens || force_display) {
    TFTscreen.stroke(tft_background_color);
    TFTscreen.setCursor(position_minute_tens);
    TFTscreen.println(previous_minute_tens);
    TFTscreen.stroke(tft_text_color);
    TFTscreen.setCursor(position_minute_tens);
    TFTscreen.println(minute_tens);
    previous_minute_tens = minute_tens;
  }
  if (minute_unit != previous_minute_unit || force_display) {
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

    if (configuring_alarm){
      if (index_number_blink==0){
        TFTscreen.setCursor(position_hour_tens);
        TFTscreen.println(hour_tens);
      }
      if (index_number_blink==1){
        TFTscreen.setCursor(position_hour_unit);
        TFTscreen.println(hour_unit);
      }
      if (index_number_blink==2){
        TFTscreen.setCursor(position_minute_tens);
        TFTscreen.println(minute_tens);
      }
      if (index_number_blink==3){
        TFTscreen.setCursor(position_minute_unit);
        TFTscreen.println(minute_unit);
      }
    }
  }
}

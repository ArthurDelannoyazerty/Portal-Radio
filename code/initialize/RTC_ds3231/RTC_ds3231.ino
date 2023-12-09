#include <RTClib.h>
#include <Wire.h>
 
RTC_DS3231 rtc;
 
char t[32];
 
void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  // rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));     // Uncomment to set current time
  Serial.print(F("Date/Time: "));
  //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}
 
void loop(){
  DateTime now = rtc.now();
  sprintf(t, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());  
  
  Serial.println(t);
 
  delay(1000);
}

#include <M5StickC.h>
#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;


#define SSIDNAME  "your-wifi-SSID"
#define WIFIPASS  "your-wifi-password"

#define NTPSERVER "ntp.nict.jp","time.google.com","ntp.jst.mfeed.ad.jp"
#define TIMEZONE  1 // GMT+1 = 1 -> winter time, GMT+2 = 2 -> summer time -> Brussels timezone

#define WIDTH 152
#define HEIGHT 72

float v;
float t;
int batpercent = 100;
char mycolor[] = "TFT_WHITE";


//unsigned long awaketime;
//#define BUTTON_PIN_BITMASK 0x2000000000  pin 37, 2^37 in hex

#define BUTTON_PIN_BITMASK 0x2000000000 // 2^33 in hex

void showTime(){
  M5.Rtc.GetTime(&RTC_TimeStruct);
  
  if (t!=RTC_TimeStruct.Seconds){
    M5.Lcd.fillRect(0, 25, 160, 45, TFT_BLACK);
    M5.Lcd.setCursor(7,28);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.printf("%02d:%02d:%02d\n",RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
    t = RTC_TimeStruct.Seconds;
  }
  if(digitalRead(M5_BUTTON_RST) == LOW){getNTP();}
}


void batStatus(){
  if (v!=M5.Axp.GetBatVoltage()){
    M5.Lcd.setCursor(135, 5);
    M5.Lcd.fillRect(135, 0, 50, 15, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    batpercent = (M5.Axp.GetBatVoltage()-3.6)*166.666;
    if (batpercent<=15){M5.Lcd.setTextColor(TFT_RED);}
    else {M5.Lcd.setTextColor(TFT_WHITE);}
    M5.Lcd.print(batpercent);
    M5.Lcd.print("%");
    //M5.Lcd.printf(ASCIIpercent);
    Serial.print(batpercent);
    Serial.println("%");
    v = M5.Axp.GetBatVoltage();
    }
}

void lowBatOff(){
  if (batpercent<=2){
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setCursor(20,28);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_BLACK);
    M5.Lcd.printf("LOW BATTERY");
    M5.Lcd.setCursor(20,45);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Turning OFF");
    delay(7000);
    M5.Axp.PowerOff();
  }
}

// TIME ADJUST
void getNTP(){
  const char* ssid = SSIDNAME;
  int count = 0;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,4);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.println(" Adjust by NTP");
  M5.Lcd.println(" Wifi Conecting...");

#ifdef SMARTCONFIG
  WiFi.begin();
#else
  WiFi.begin(ssid, WIFIPASS);
#endif

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if(++count > 60){
        M5.Lcd.println("No Wifi");
        delay(1000);
        M5.Lcd.fillScreen(BLACK);
        showTime();
        return;    
      }
  }
  M5.Lcd.println(" CONNECTED!");
  
  //init and get the time
  configTime(TIMEZONE * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");  //TIMEZONE * 3600L
  struct tm timeInfo;
  if (getLocalTime(&timeInfo)) { 
    // Set RTC time
    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours   = timeInfo.tm_hour;
    TimeStruct.Minutes = timeInfo.tm_min;
    TimeStruct.Seconds = timeInfo.tm_sec;
    M5.Rtc.SetTime(&TimeStruct);
 
    RTC_DateTypeDef DateStruct;
    DateStruct.WeekDay = timeInfo.tm_wday;
    DateStruct.Month = timeInfo.tm_mon + 1;
    DateStruct.Date = timeInfo.tm_mday;
    DateStruct.Year = timeInfo.tm_year + 1900;
    M5.Rtc.SetData(&DateStruct);
  }

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(2000);
  M5.Lcd.fillScreen(BLACK);
  showTime();
}

//setup
void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(10, OUTPUT); 
  digitalWrite(10, HIGH);
  M5.begin();
  M5.Axp.ScreenBreath(8);
  M5.IMU.Init();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.MPU6886.Init();
  v = M5.Axp.GetBatVoltage();
  t = RTC_TimeStruct.Seconds;

  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_37,0); //1 = High, 0 = Low
  showTime();
}


void gotosleep(){
  if (millis()>=15000){
    Serial.println("Going to sleep");
    M5.Axp.SetSleep(); // conveniently turn off screen, etc.
    WiFi.mode(WIFI_OFF);
    digitalWrite(10, HIGH);
    esp_deep_sleep_start(); //esp_deep_sleep_start();
  } 
}


void loop() {
  gotosleep();
  batStatus();
  showTime();
  lowBatOff();
  if( M5.Axp.GetBtnPress())
  {
    M5.Axp.PowerOff();
  }
  delay(10);
}

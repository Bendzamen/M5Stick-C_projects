#include "M5StickC.h" // include the M5StickC library
#include "WiFi.h" // include the WiFi library for NTP
#include <time.h> // include time.h for struct tm

const char* ssid = "SSID your-wifi-SSID"; // replace with your WiFi
const char* password = "your-wifi-password"; // replace with your WiFi password

#define TIMEZONE  1 // GMT+1, set your own time zone

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

int prevDay = 0;
float t;
time_t christmasTime;

void setup() {
  M5.begin(); // initialize the M5StickC
  M5.Axp.ScreenBreath(8);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,4);
  // connect to WiFi
  WiFi.begin(ssid, password);
  M5.Lcd.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  }
  M5.Lcd.println("WiFi connected");

  // set the time using NTP
  configTime(TIMEZONE * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp"); // set the NTP servers
  time_t now;
  struct tm timeInfo;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,4);
  M5.Lcd.println("Retrieving time from NTP server..");
  while (!getLocalTime(&timeInfo)) { // wait until the time is retrieved
    delay(500);
  }
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
    
  M5.Rtc.GetTime(&RTC_TimeStruct);
  t = RTC_TimeStruct.Seconds;

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(20,4);
  M5.Lcd.printf("Days until Christmas\n");
}

void loop() {
  // get the current date and time from the RTC
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.Rtc.GetData(&RTC_DateStruct);

  tm christmas = {};
  if (RTC_DateStruct.Month-1==11 && RTC_DateStruct.Date>25){
    christmas.tm_year = RTC_DateStruct.Year+1;
  }
  else{
    christmas.tm_year = RTC_DateStruct.Year;
  }
  christmas.tm_mon = 11; // Month is 0-based
  christmas.tm_mday = 25;
  time_t christmasTime = mktime(&christmas);

  tm current= {};
  current.tm_year = RTC_DateStruct.Year;
  current.tm_mon = RTC_DateStruct.Month-1; // Month is 0-based
  current.tm_mday = RTC_DateStruct.Date;

  time_t currentTime = mktime(&current);
  double secondsUntilChristmas = difftime(christmasTime, currentTime);

  // Calculate the number of days until Christmas
  int daysUntilChristmas = secondsUntilChristmas / 86400;

  // display the number of days until Christmas on the M5StickC screen

  if (prevDay!=daysUntilChristmas){ //update everything when the daysUntilChristmas has changed
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(20,4);
    M5.Lcd.printf("Days until Christmas\n");
    M5.Lcd.setCursor(50,25);
    M5.Lcd.setTextSize(4);
    M5.Lcd.printf("%d\n", daysUntilChristmas);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(4, 70);
    M5.Lcd.printf("%02d:%02d:%02d\n",RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
    M5.Lcd.setCursor(100, 70);
    M5.Lcd.printf("%02d.%02d.%04d\n",RTC_DateStruct.Date, RTC_DateStruct.Month, RTC_DateStruct.Year);
    prevDay = daysUntilChristmas;
  }
  if (t!=RTC_TimeStruct.Seconds){ //update time when seconds have changed, prevents flickering
    M5.Lcd.fillRect(0, 70, 90, 10, BLACK);
    M5.Lcd.setCursor(4,70);
    M5.Lcd.printf("%02d:%02d:%02d\n",RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
    t = RTC_TimeStruct.Seconds;
  }
}
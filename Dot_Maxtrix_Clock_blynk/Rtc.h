#include <Rtc_Pcf8563.h>
Rtc_Pcf8563 rtc;//初始化实时时钟

// Offset hours from gps time (UTC)
//#define TIMEZONE  8     // Central European Time

void GetRtc() {
  Serial.println("GetRtc");
  rtc.getDate();
  rtc.getTime();
  setTime(rtc.getHour(), rtc.getMinute(), rtc.getSecond(),  rtc.getDay(), rtc.getMonth(), rtc.getYear());
#ifdef TIMEZONE
  adjustTime(TIMEZONE * SECS_PER_HOUR);
#endif
}

void SetRtc() {
  Serial.println("SetRtc");
  rtc.initClock();  //set a time to start with.
  rtc.setDate(day(), weekday() , month(), 0, year() - 2000); //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setTime(hour(), minute() , second()); //hr, min, sec
}

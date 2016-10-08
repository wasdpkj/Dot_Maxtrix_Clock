#include <avr/wdt.h>
#include <Wire.h>

#include <Rtc_Pcf8563.h>
#include <TimeLib.h>
//#define TIMEZONE  8     // Central European Time

#include "Microduino_Matrix.h"

uint8_t Addr[MatrixPix_X][MatrixPix_Y] = {  //1x4
  { 64}
};

Matrix display = Matrix(Addr);

#include "time.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); //RX,TX

char buffer[128];
boolean buffer_sta_t = false;
boolean buffer_sta_d = false;

boolean mode = false;
unsigned long mode_num = 0;
unsigned long buffer_num = 0;
unsigned long timer[4] = {0, 0, 0, 0};

void setup() {
  wdt_enable(WDTO_8S); //开启看门狗，并设置溢出时间为4秒

  Serial.begin(9600);
  mySerial.begin(9600);

  Wire.begin();
  GetRtc();

  //getDeviceAddr
  for (int a = 0; a < display.getMatrixNum(); a++) {
    Serial.print(display.getDeviceAddr(a));
    Serial.print(" ");
  }
  Serial.println("");

  //setLedColor
  for (int y = 0; y < display.getHeight() * 8; y++) {
    for (int x = 0; x < display.getWidth() * 8; x++) {
      randomSeed(analogRead(A0));
      display.setLedColor(x, y, random(0, 255), random(0, 255), random(0, 255));   //x, y, r, g, b
      delay(5);
    }
  }
  wdt_reset();
  delay(1000);
  display.clearDisplay();
}

void loop() {
  bleUpdata();

  if (millis() < timer[0]) timer[0] = millis();
  if (millis() - timer[0] > 1000) { //1 second
    if ((timeStatus() != timeNotSet)) {
      display.clearDisplay();
      display.setColor(0, 0, 255);
      for (int a = 0; a < 16; a++) {
        display.setLed(a < 8 ? a : a - 8, a < 8 ? 0 : 1, bitRead(year(), 15 - a));
      }

      display.setColor(0, 255, 0);
      for (int a = 0; a < 8; a++) {
        display.setLed(a , 2, bitRead(month(), 7 - a));
      }

      display.setColor(0, 255, 255);
      for (int a = 0; a < 8; a++) {
        display.setLed(a , 3, bitRead(day(), 7 - a));
      }

      display.setColor(255, 0, 0);
      for (int a = 0; a < 8; a++) {
        display.setLed(a , 4, bitRead(weekday(), 7 - a));
      }

      display.setColor(255, 0, 255);
      for (int a = 0; a < 8; a++) {
        display.setLed(a , 5, bitRead(hour(), 7 - a));
      }

      display.setColor(255, 255, 0);
      for (int a = 0; a < 8; a++) {
        display.setLed(a , 6, bitRead(minute(), 7 - a));
      }

      display.setColor(255, 255, 255);
      for (int a = 0; a < 8; a++) {
        display.setLed(a , 7, bitRead(second(), 7 - a));
      }
    }
    timer[0] = millis();
  }

  if (millis() < timer[1]) timer[1] = millis();
  if (millis() - timer[1] > 3600 * 1000) { //3600 second
    GetRtc();

    timer[1] = millis();
  }

  if (millis() < timer[3]) timer[3] = millis();
  if (millis() - timer[3] > 1000) { //1 second
    mySerial.print(rtc.formatDate());
    mySerial.print(" ");
    mySerial.print(rtc.formatTime());

    timer[3] = millis();
  }

  wdt_reset();
}

void bleUpdata() {

  while (mySerial.available()) {
    char c = mySerial.read();
    delay(1);
    if (c == 't' && buffer_num == 0 && !buffer_sta_d)
      buffer_sta_t = true;
    if (c == 'm' && buffer_num == 0 && !buffer_sta_t)
      buffer_sta_d = true;

    if (buffer_sta_t || buffer_sta_d) {
      if (buffer_sta_d) {
        if (buffer_num == 0 && c == 'm')
          buffer_num = 0;
        else
          buffer[buffer_num++] = c;
      }
      else
        buffer[buffer_num++] = c;
    }
    Serial.print(c);
  }
  wdt_reset();

  if (buffer_sta_t) {
    buffer_sta_t = false;

    sscanf((char *)strstr((char *)buffer, "t"), "t%d,%d,%d,%d,%d,%d", &sta[0], &sta[1], &sta[2], &sta[3], &sta[4], &sta[5]);
    setTime(sta[3], sta[4], sta[5],  sta[2], sta[1], sta[0]);
    SetRtc();
    GetRtc();

    for (int x = 0; x < display.getWidth() * 8 * 2; x++) {
      for (int y = 0; y < display.getHeight() * 8; y++) {
        randomSeed(analogRead(A0));
        if (x < display.getWidth() * 8)
          display.setLedColor(x, y, random(0, 255), random(0, 255), random(0, 255));   //x, y, r, g, b
        else
          display.setLedColor(x - display.getWidth() * 8, y, 0, 0, 0); //x, y, r, g, b
      }
      delay(20);
    }
    display.clearDisplay();

    for (int a = 0; a < buffer_num; a++)
      buffer[a] = NULL;
    buffer_num = 0;
  }
  wdt_reset();

  if (buffer_sta_d) {
    buffer_sta_d = false;

    display.clearColor();
    display.writeString(buffer, 20, 0);
    display.clearDisplay();

    for (int a = 0; a < buffer_num; a++)
      buffer[a] = NULL;
    buffer_num = 0;
  }
}

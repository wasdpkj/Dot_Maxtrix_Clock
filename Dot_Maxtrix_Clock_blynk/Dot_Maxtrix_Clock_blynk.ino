// 本作品采用知识共享 署名-非商业性使用-相同方式共享 3.0 未本地化版本 许可协议进行许可
// 访问 http://creativecommons.org/licenses/by-nc-sa/3.0/ 查看该许可协议
// ==============

// 版权所有：
// @老潘orz  wasdpkj@hotmail.com
// ==============

// Microduino-IDE
// ==============
// Microduino Getting start:
// http://www.microduino.cc/download/

// Microduino IDE Support：
// https://github.com/wasdpkj/Microduino-IDE-Support/

// ==============
// Microduino wiki:
// http://wiki.microduino.cc

// ==============
// E-mail:
// Kejia Pan
// pankejia@microduino.cc

// ==============
// Weibo:
// @老潘orz

#include <avr/wdt.h>  //for wdt
#include <Time.h> //for rtc
#include <TimerOne.h> //for initialize
#include <EEPROM.h> //for eeprom
#include <Wire.h> //for i2c

//----------------------------------------
#define WIFI_SSID "Makermodule"
#define WIFI_PASSWORD "microduino"
char auth[] = "2d6635f12d0143339b5304bafb62c3cfpkj";// You should get Auth Token in the Blynk App.

boolean staMessage = false, staProximity = false;
int modeNum = 0;
uint8_t color[2][3] = {
  {0, 255, 255},
  {255, 0, 255}
};
unsigned long timer[4];
String _buffer_data;
char buffer_data[128];

//----------------------------------------
#include "Microduino_Matrix.h"  //for matrix
uint8_t Addr[MatrixPix_X][MatrixPix_Y] = {  //3x2
  { 64, 63, 62, 61}
};
Matrix display = Matrix(Addr);

//----------------------------------------
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266_HardSer.h>
#include <BlynkSimpleShieldEsp8266_HardSer.h>
#define EspSerial Serial1     // Set ESP8266 Serial object
ESP8266 wifi(EspSerial);

WidgetLED ledB(V0);
WidgetLED ledA(V1);
WidgetTerminal terminal(V2);// Attach virtual serial terminal to Virtual Pin V1

BLYNK_WRITE(V2) { //Message and SyncTime
  for (int a = 0; a < 256; a++) buffer_data[a] = NULL;
  _buffer_data =  param.asStr();
  if (_buffer_data.startsWith("T:", 0)) {
    _buffer_data.setCharAt(0, ' ');
    _buffer_data.setCharAt(1, ' ');
    _buffer_data.trim();
    unsigned long _time = _buffer_data.toInt();
    if (_time >= 1453998871) {
      setTime(_time);
      adjustTime(8 * SECS_PER_HOUR);
      SetRtc();
      Serial.print("Time is Sync:");
      terminal.println("Time is Sync");
    }
    else {
      Serial.print("Time no Sync:");
      terminal.println("Time no Sync");
    }
  }
  else {
    if (_buffer_data.length() < 128) {
      for (int a = 0; a < _buffer_data.length(); a++) buffer_data[a] = _buffer_data[a];
      terminal.print("You said:");
      terminal.write(param.getBuffer(), param.getLength());
      staMessage = true;
    }
    else {
      terminal.print("Error:toolength");
    }
    terminal.println();
  }
  terminal.flush();
}

BLYNK_WRITE(V3) { //Sync TimeColor
  for (int a = 0; a < 3; a++) {
    color[0][a] = param[a].asInt();
    EEPROM.write(a, color[0][a] );
  }
  terminal.println("TimeColor is Sync");
  terminal.println();
  terminal.flush();
}

BLYNK_WRITE(V4) { //Sync DateColor
  for (int a = 0; a < 3; a++) {
    color[1][a] = param[a].asInt();
    EEPROM.write(3 + a, color[1][a] );
  }
  terminal.println("DateColor is Sync");
  terminal.println();
  terminal.flush();
}

void UnReadMessages() {
  colorClear(2);
  display.setColor(255, 0, 0);
  display.setCursor(0, 1);   //x, y
  display.print("a Mes.");
  ledA.on();
  terminal.println("Un Read Messages!");
  // Ensure everything is sent
  terminal.flush();
}

void ReadMessages() {
  staProximity = false;
  staMessage = false;

  colorClear(10);
  display.setColor(255, 255, 255);
  display.clearDisplay();
  display.writeString(buffer_data, MODE_H, 50, 1);
  display.clearDisplay();
  ledA.off();
  terminal.println("Message has been read!");
  terminal.print("At:"); terminal.print(hour()); terminal.print(":"); terminal.print(minute());
  terminal.println();
  terminal.println("I'm Ready!");
  terminal.flush();
}


void showClock() {
  modeNum++;
  if (modeNum > 20) modeNum = 0;
  if (modeNum == 15 || !modeNum) colorClear(2);
  //display.clearDisplay();
  display.setFontMode(MODE_H);
  if (modeNum < 15) {
    display.setColor(color[0][0], color[0][1], color[0][2]);
    display.setCursor(4, 1);   //x, y
    if (hour() < 10) display.print('0'); display.print(hour());
    display.print((millis() / 1000) % 2 ? " " : ":");
    if (minute() < 10) display.print('0'); display.print(minute());
  }
  else {
    display.setColor(color[1][0], color[1][1], color[1][2]);
    display.setCursor(4, 1);   //x, y
    if (month() < 10) display.print('0'); display.print(month());
    display.print("-");
    if (day() < 10) display.print('0'); display.print(day());
  }
  display.print("   ");
  (millis() / 1000) % 2 ? ledB.on() : ledB.off();
}

void colorClear(uint8_t _time) {
  for (int x = 0; x < display.getWidth() * 8 * 2; x++) {
    for (int y = 0; y < display.getHeight() * 8; y++) {
      randomSeed(analogRead(A0));
      if (x < display.getWidth() * 8) display.setLedColor(x, y, random(0, 255), random(0, 255), random(0, 255));   //x, y, r, g, b
      else display.setLedColor(x - display.getWidth() * 8, y, 0, 0, 0); //x, y, r, g, b
    }
    delay(_time);
  }
  display.clearDisplay();
}

void updataProximity() {
  pinMode(A6, INPUT_PULLUP);
  if (!digitalRead(A6) && staMessage) {
    staProximity = true;
    Serial.println("Key!");
  }
}

void setup() {
  Serial.begin(115200); // See the connection status in Serial Monitor
  EspSerial.begin(115200);
  Wire.begin();

  for (int a = 0; a < 3; a++) color[0][a] = EEPROM.read(a);
  for (int a = 0; a < 3; a++) color[1][a] = EEPROM.read(3 + a);
  //getDeviceAddr
  for (int a = 0; a < display.getMatrixNum(); a++) {
    Serial.print(display.getDeviceAddr(a));
    Serial.print(" ");
  }
  Serial.println("");

  colorClear(10);
  //clearColor
  display.clearColor();
  //writeString H
  display.writeString("Microduino", MODE_H, 5, 1); //string, MODE, time ,y
  display.clearDisplay();
  display.setColor(255, 255, 0);
  display.setCursor(0, 1);   //x, y
  display.print("conWifi"); //string, MODE, time ,y
  Blynk.begin(auth, wifi, WIFI_SSID, WIFI_PASSWORD);
  display.clearDisplay();
  display.setCursor(0, 1);   //x, y
  while (Blynk.connect() == false) {
    Serial.println("Con ERROR");
    display.setColor(255, 0, 0);
    display.print("conER."); //string, MODE, time ,y
    wdt_enable(WDTO_1S); //reset mcu
  }
  Serial.println("con OK");
  display.setColor(0, 255, 0);
  display.print("conOK."); //string, MODE, time ,y
  delay(1000);
  display.clearDisplay();

  wdt_enable(WDTO_8S); //开启看门狗，并设置溢出时间为8秒

  terminal.println("I'm Ready!");
  // Ensure everything is sent
  terminal.flush();

  Timer1.initialize(50000); // set a timer of microseconds
  Timer1.attachInterrupt(updataProximity); // attach the service routine here

  GetRtc(); //get time from rtc
}

void loop() {
  Blynk.run(); // Initiates Blynk

  if (staMessage) {
    if (!staProximity) {
      if (timer[0] > millis()) timer[0] = millis();
      if (millis() - timer[0] > 5000) {
        UnReadMessages();
        timer[0] = millis();
      }
    }
    else {
      ReadMessages();
      timer[1] = millis() - 1000;
    }
  }
  else {
    if (timer[1] > millis()) timer[1] = millis();
    if (millis() - timer[1] > 500) {
      showClock();
      timer[1] = millis();
    }
    if (timer[2] > millis()) timer[2] = millis();
    if (millis() - timer[2] > 3600000) {
      GetRtc();
      timer[2] = millis();
    }
  }

  wdt_reset();
}

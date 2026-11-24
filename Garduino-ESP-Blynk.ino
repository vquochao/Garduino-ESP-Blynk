/**************************************************************

        Garduino

        An Arduino Controlled Irrigation System using
        Wifi connected moisture sensors and integrated
        with the Blynk Mobile App for input and control.

        Github: https://github.com/jaminNZx/Garduino-ESP-Blynk

        Written by: Ben Selkirk (JaminNZx)

        Official Blynk Support Forum: 
        http://community.blynk.cc/t/garduino-the-solar-wifi-sensor-driven-irrigation-system/11150

 **************************************************************/

//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

#include "settings_base.h"
#include "globals.h"
#include "functions.h"
#include "blynk_writes.h"

/*
   Status line + monthly reset (every second)
*/
void statusTask() {
  long rssi = WiFi.RSSI();
  Blynk.setProperty(vPIN_TERMINAL, "label",
                    String("WIFI: ") + constrain(map(rssi, -105, -40, 0, 100), 0, 100) +
                    "% (" + rssi + "dB) | " + curDateTime());
  if (timeStatus() != timeNotSet && year() > 2000 && month() != month_old) {
    waterCost_month = 0;
    totalMilliLitres_month = 0;
    month_old = month();
    Blynk.virtualWrite(vPIN_LAST_MONTH, month_old);
    Blynk.virtualWrite(vPIN_WATER_TOTAL_MONTH, 0);
    Blynk.virtualWrite(vPIN_WATER_COST_MONTH, 0);
    addTableRow("RESET MONTHLY DATA", " ");
  }
}
/*
   SETUP
*/
void setup() {
  // COMMUNICATIONS
  Serial.begin(115200);
  // PINMODES
  digitalWrite(TAP, TAP_INACTIVE);   // set level before enabling output -> no relay glitch at boot
  pinMode(TAP, OUTPUT);
  // FLOW METER
  pinMode(FLOW_SENSOR, INPUT_PULLUP);
  lastFlowCheck = millis();
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR), pulseCounter, FALLING);
  // CONNECT TO BLYNK (Blynk.begin blocks until connected; sync is done in BLYNK_CONNECTED)
  WiFi.mode(WIFI_STA);
#ifdef LOCAL_SERVER
  Blynk.begin(AUTH, WIFI_SSID, WIFI_PASS, LOCAL_SERVER);
#else
  Blynk.begin(AUTH, WIFI_SSID, WIFI_PASS);
#endif
  // OVER THE AIR UPDATES
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.begin();
  // RTC: sync fast until the first valid time, then every 5 minutes
  setSyncInterval(5);
  // TIMERS
  timer.setInterval(1000L, flowSensor);
  timer.setInterval(1000L, statusTask);
  rtcSyncTimer = timer.setInterval(1000L, []() {
    if (timeStatus() != timeNotSet && year() > 2000) {
      setSyncInterval(300);
      printTask("RTC SYNC", "5 MIN");
      cancelTimer(rtcSyncTimer);
    }
  });
  // DONE
  Serial.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.flush();
}
/*
   LOOP
*/
void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
}

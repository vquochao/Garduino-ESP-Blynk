/**************************************************************
 *
 *                  Settings - Base
 *
 **************************************************************/
/*
   Auth Codes & Wifi info go in the following file.
   Create a new folder in your library dir called 'wifi_credentials'
   and create a new file called 'wifi_credentials.h' and copy the
   example in the README. You can also define AUTH there to keep the
   Blynk token out of this repository.
*/
#include <wifi_credentials.h>
/*
     Blynk Auth Codes (fallback if not defined in wifi_credentials.h)
*/
#ifndef AUTH
#define AUTH                      "0913ee2009cd4fa9b5307e572a0c5028"
#endif
/*
   Over The Air Hostname
*/
#define OTA_HOSTNAME              "GARDUINO-BASE"
/*
   Local Server Settings (comment out to use the Blynk cloud)
*/
#define LOCAL_SERVER              IPAddress(192, 168, 1, 2)
/*
   Console Output (Debug/Log)
*/
//#define OUTPUT_SERIAL
#define OUTPUT_TERMINAL
/*
   Hardware Pins
*/
#define TAP                       14 // GPIO14 (D5 on NodeMCU/WeMos, GPIO14 on ESP32)
#define TAP_ACTIVE                LOW   // relay level that opens the valve (LOW = active-low relay)
#define TAP_INACTIVE              (!TAP_ACTIVE)
/*
   Flow Meter Pin and Calibration factor
   (YF-S201: pulse frequency [Hz] = 4.5 * flow [L/min])
*/
#define FLOW_SENSOR               4  // GPIO4  (D2 on NodeMCU/WeMos, GPIO4 on ESP32)
#define FLOW_CALIBRATION          4.5
/*
   Water Cost. Comment out to disable.
*/
#define WATER_PRICE               0.1444 // c/l -- Find it on your water bill. $1.444 /Kl = $0.001444 /l === 0.1444 c/l !
/*
   Virtual Pins
*/
#define vPIN_TABLE                V1

#define vPIN_TAP_LED              V7
#define vPIN_TAP_LIMIT            V14
#define vPIN_TAP_MANUAL           V18
#define vPIN_TAP_ACTIVATE         V19

#define vPIN_WATER_TOTAL          V25
#define vPIN_WATER_TOTAL_MONTH    V21
#define vPIN_WATER_FLOW           V26
#define vPIN_WATER_COST           V28
#define vPIN_WATER_COST_MONTH     V11

#define vPIN_WATER_TAP            V27

#define vPIN_TERMINAL             V31

#define vPIN_LIMITMENU            V9

#define vPIN_LAST_MONTH           V10

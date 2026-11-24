# Garduino-ESP-Blynk
A wireless (solar poweed) sensor controlled garden irrigation system.

This is still a work in progress. I'll write up a readme later on but feel free to pick it up and try it out. 

Hardware: 

* 3.3V or 5V Relay Module (HIGH or LOW active)
* ESP8266-Dev (WeMos/NodeMCU style)
* Liquid Flow Meter (I used model YF-S201) 
* 12VDC Solenoid water valves

Software:

* Download this project as a zip and extract to your project directory in its own folder.
* Match the pinouts from the code (settings_base.h). 
* Download the Blynk mobile app and create a project. Open the email send to you with the auth code and put it in settings_base.h (or, better, `#define AUTH "..."` in wifi_credentials.h so the token is not committed).
* Create a new folder in your library dir called 'wifi_credentials' and create a new file called 'wifi_credentials.h' and copy the example below. Otherwise just enter the credentials in settings. 

```cpp
/**************************************************************
 *           Settings - Wifi Credentials
 **************************************************************/

/*
   Wifi Credentials
*/
#define WIFI_SSID               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define WIFI_PASS               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
```

Backup:

* The original code before the bug-fix/optimization pass is kept in `backup/2026-11-24_original/` and under the git tag `backup-before-optimize-2026-11-24`.

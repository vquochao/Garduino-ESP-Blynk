/**************************************************************

                     BLYNK_WRITE

 **************************************************************/
/*
  On (re)connect
    - RTC must be (re)started after each connection (WidgetRTC requirement)
    - stored values are only pulled from the server on the first connection,
      otherwise data counted while offline would be overwritten by older values
*/
BLYNK_CONNECTED() {
  if (firstConnect) {
    Blynk.syncVirtual(vPIN_LIMITMENU, vPIN_WATER_COST_MONTH, vPIN_WATER_TOTAL_MONTH, vPIN_LAST_MONTH);
    firstConnect = false;
  } else {
    Blynk.syncVirtual(vPIN_LIMITMENU);
  }
  // request time AFTER the sync so vPIN_LAST_MONTH arrives before the RTC is valid
  rtc.begin();
  // valve state survives a reconnect, reflect it in the app
  Blynk.virtualWrite(vPIN_TAP_LED, isTapOn() ? 255 : 0);
  Blynk.virtualWrite(vPIN_TAP_ACTIVATE, isTapOn() ? 1 : 0);
}
/*
  Usage limit menu selector
*/
BLYNK_WRITE(vPIN_LIMITMENU) {
  switch (param.asInt()) {
    case 1:
      limitType = 1;
      printTask("LIMITED BY", "TIME");
      Blynk.setProperty(vPIN_TAP_LIMIT, "label", "TIME: MINUTES");
      break;
    case 2:
      limitType = 2;
      printTask("LIMITED BY", "WATER");
      Blynk.setProperty(vPIN_TAP_LIMIT, "label", "WATER: LITRES");
      break;
  }
  Blynk.syncVirtual(vPIN_TAP_LIMIT);
}
/*
  Usage limit slider
*/
BLYNK_WRITE(vPIN_TAP_LIMIT) {
  switch (limitType) {
    case 1:
      tap_limit_value = param.asLong() * 60000L;
      printTask("TAP LIMIT", String(tap_limit_value / 1000) + " SEC");
      break;
    case 2:
      tap_limit_value = param.asLong() * 10L;
      printTask("TAP LIMIT", String(tap_limit_value) + " LTRS");
      break;
  }
}
/*
  Old manual button (toggles the tap on press)
*/
BLYNK_WRITE(vPIN_TAP_MANUAL) {
  if (param.asInt()) TAP_Toggle();   // ignore the release event of push buttons
}
/*
  Activate button
*/
BLYNK_WRITE(vPIN_TAP_ACTIVATE) {
  param.asInt() ? TAP_On() : TAP_Off();
}
/*
  Total water cost month
*/
BLYNK_WRITE(vPIN_WATER_COST_MONTH) {
  waterCost_month = param.asDouble();
  printTask("SYNC WATER COST", String(waterCost_month, 6));
}
/*
  Total water used this month
*/
BLYNK_WRITE(vPIN_WATER_TOTAL_MONTH) {
  totalMilliLitres_month = param.asDouble() * 1000.0;
  printTask("SYNC WATER USE", String(totalMilliLitres_month / 1000.0));
}
/*
  Last Month Widget
*/
BLYNK_WRITE(vPIN_LAST_MONTH) {
  month_old = param.asInt();
  printTask("SYNC MONTH", String(month_old));
}

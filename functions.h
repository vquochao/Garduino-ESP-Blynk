/**************************************************************

                       General Functions

 **************************************************************/
#ifndef IRAM_ATTR
#define IRAM_ATTR ICACHE_RAM_ATTR
#endif
#if defined(ESP32)
// ESP32 is dual core: noInterrupts() is not enough, use a spinlock
portMUX_TYPE pulseMux = portMUX_INITIALIZER_UNLOCKED;
#define PULSE_LOCK()        portENTER_CRITICAL(&pulseMux)
#define PULSE_UNLOCK()      portEXIT_CRITICAL(&pulseMux)
#define PULSE_LOCK_ISR()    portENTER_CRITICAL_ISR(&pulseMux)
#define PULSE_UNLOCK_ISR()  portEXIT_CRITICAL_ISR(&pulseMux)
#else
#define PULSE_LOCK()        noInterrupts()
#define PULSE_UNLOCK()      interrupts()
#define PULSE_LOCK_ISR()
#define PULSE_UNLOCK_ISR()
#endif
/*
  getCurrentTime() -
      - return the time as 'HH:MM:SS'
*/
String getCurrentTime() {
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hour(), minute(), second());
  return String(buf);
}
/*
  getCurrentDate() -
      - return the date as 'D-Mon-YY'
*/
String getCurrentDate() {
  char buf[12];
  snprintf(buf, sizeof(buf), "%d-%s-%02d", day(), monthShortStr(month()), year() % 100);
  return String(buf);
}
/*
  curDateTime() -
      - return the date & time as String 'D-Mon-YY HH:MM:SS'
*/
String curDateTime() {
  return getCurrentDate() + ' ' + getCurrentTime();
}
/*
   printTask() - usage 'printTask("ABC", "123");'
        - outputs to Serial or Terminal or both depending what is defined in settings
*/
void printTask(const String &a, const String &b) {
#if defined(OUTPUT_SERIAL)
  Serial.print(a);
  Serial.print('=');
  Serial.println(b);
#endif
#if defined(OUTPUT_TERMINAL)
  terminal.print(a);
  terminal.print('=');
  terminal.println(b);
  terminal.flush();
#endif
}
/*
   addTableRow() - add a row to the table widget with a unique index
*/
void addTableRow(const String &name, const String &value) {
  Blynk.virtualWrite(vPIN_TABLE, "add", rowIndex++, name, value);
}
/*
   isTapOn() - true when the valve relay is active
*/
inline bool isTapOn() {
  return digitalRead(TAP) == TAP_ACTIVE;
}
/*
   cancelTimer() - delete a BlynkTimer (frees the slot, unlike disable())
*/
void cancelTimer(int &id) {
  if (id >= 0) {
    timer.deleteTimer(id);
    id = -1;
  }
}
/**************************************************************

                     Flow Sensor

 **************************************************************/
/*
   pulseCounter() -
      - attached to an interrupt counting the pulses from the sensor
      - must live in IRAM on ESP8266/ESP32, otherwise it crashes ("ISR not in IRAM!")
*/
void IRAM_ATTR pulseCounter() {
  PULSE_LOCK_ISR();
  pulseCount++;
  PULSE_UNLOCK_ISR();
}
/*
   flowSensor() -
      - reads the pulses counted since the last call (atomically)
      - calculates the flowRate in L/min using the real elapsed time
      - accumulates the totals and cost
      - updates Blynk only when something changed (less traffic)
*/
void flowSensor() {
  PULSE_LOCK();
  uint32_t pulses = pulseCount;
  pulseCount = 0;
  PULSE_UNLOCK();

  unsigned long now = millis();
  unsigned long elapsed = now - lastFlowCheck;
  lastFlowCheck = now;
  if (elapsed == 0) return;

  static bool wasFlowing = true;   // force one update after boot
  if (pulses == 0) {
    flowRate = 0;
    flowMilliLitres = 0;
    if (wasFlowing) {               // send a single "0" when flow stops
      Blynk.virtualWrite(vPIN_WATER_FLOW, 0);
      wasFlowing = false;
    }
    return;
  }
  wasFlowing = true;

  // pulses/sec / calibration = L/min
  flowRate = (pulses * 1000.0f / elapsed) / FLOW_CALIBRATION;
  // mL during this period
  flowMilliLitres = (flowRate / 60.0f) * elapsed;
  totalMilliLitres       += flowMilliLitres;
  totalMilliLitres_month += flowMilliLitres;

  Blynk.virtualWrite(vPIN_WATER_TOTAL,       totalMilliLitres / 1000.0);
  Blynk.virtualWrite(vPIN_WATER_FLOW,        String(flowMilliLitres, 0));
  Blynk.virtualWrite(vPIN_WATER_TOTAL_MONTH, totalMilliLitres_month / 1000.0);
#ifdef WATER_PRICE
  double cost = (flowMilliLitres / 1000.0) * WATER_PRICE / 100.0;  // c/L -> $
  waterCost       += cost;
  waterCost_month += cost;
  Blynk.virtualWrite(vPIN_WATER_COST,       String(waterCost, 6));
  Blynk.virtualWrite(vPIN_WATER_COST_MONTH, String(waterCost_month, 6));
#endif
}
/**************************************************************

                     Tap Control

 **************************************************************/
/*
  TAP_Off() -
      - only trigger if relay is active
      - cancel any pending limit timer
      - turn off tap relay, dim the status led widget
      - after 6 sec (water drained from sensor) report usage & cost
*/
void TAP_Off() {
  if (!isTapOn()) return;
  cancelTimer(limitTimer);
  digitalWrite(TAP, TAP_INACTIVE);
  Blynk.virtualWrite(vPIN_TAP_LED, 128);
  Blynk.virtualWrite(vPIN_TAP_ACTIVATE, 0);   // keep the app button in sync
  printTask("TAP", "SHUTTING DOWN..");
  cancelTimer(offTimer);
  offTimer = timer.setTimeout(6000, []() {
    offTimer = -1;
    String litres = String(totalMilliLitres / 1000.0, 3);
    printTask("TAP", "OFF");
    printTask("USED", litres + "L  COST=$" + String(waterCost, 3));
    Blynk.virtualWrite(vPIN_TAP_LED, 0);
    addTableRow(curDateTime(), litres + " L");
    addTableRow(" ", "$" + String(waterCost, 6));
  });
}
/*
   TAP_On() -
      - reset 'current' usage and cost
      - turn on tap relay & led status widget
      - depending on limit type start a timer to stop the tap
*/
void TAP_On() {
  if (isTapOn()) return;
  cancelTimer(offTimer);      // previous run's report is superseded
  cancelTimer(limitTimer);
  totalMilliLitres = 0;
  waterCost = 0;
  digitalWrite(TAP, TAP_ACTIVE);
  Blynk.virtualWrite(vPIN_TAP_LED, 255);
  Blynk.virtualWrite(vPIN_TAP_ACTIVATE, 1);
  switch (limitType) {
    case 1:
      printTask("TAP=ON  LIMIT", String(tap_limit_value / 1000) + " SEC");
      if (tap_limit_value > 0)
        limitTimer = timer.setTimeout(tap_limit_value, []() {
          limitTimer = -1;
          TAP_Off();
        });
      break;
    case 2:
      printTask("TAP=ON  LIMIT", String(tap_limit_value) + " LTRS");
      if (tap_limit_value > 0)
        limitTimer = timer.setInterval(250, []() {
          if (totalMilliLitres / 1000.0 >= tap_limit_value) TAP_Off();
        });
      break;
    default:
      printTask("TAP", "ON (NO LIMIT)");
      break;
  }
}
/*
  TAP_Toggle() -
      - Toggles the tap state.
*/
void TAP_Toggle() {
  isTapOn() ? TAP_Off() : TAP_On();
}

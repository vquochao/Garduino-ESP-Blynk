/**************************************************************

                            Globals

 **************************************************************/
BlynkTimer timer;

WidgetTerminal terminal(vPIN_TERMINAL);
WidgetRTC rtc;

// limitType: 0 = no limit, 1 = time, 2 = water volume
int  limitType = 0;
long tap_limit_value = 0;        // ms (time limit) or litres (water limit)
int  limitTimer   = -1;          // id of the timer that enforces the active limit
int  offTimer     = -1;          // id of the delayed "tap off" report timer
int  rtcSyncTimer = -1;
int  rowIndex     = 0;
int  month_old    = 0;
bool firstConnect = true;

volatile uint32_t pulseCount = 0;
unsigned long lastFlowCheck  = 0;

float  flowRate = 0.0f;          // L/min
float  flowMilliLitres = 0.0f;   // mL during the last measurement period
// double: float only has ~7 significant digits, monthly totals would lose precision
double totalMilliLitres = 0, totalMilliLitres_month = 0;
double waterCost = 0, waterCost_month = 0;

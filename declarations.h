#define RELAY_SIGNAL 9
#define UP_BTN 10
#define DOWN_BTN 11
#define MODE_BTN 8

// SDA: A4
// SCL: A5
// Possible addresses: 0x27, 0x38, 0x7C
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x38, 16, 2);

/* =========================== */
/* ======== CONSTANTS ======== */
/* =========================== */

const int BTN_COUNT = 3;
const int MODE_COUNT = 4;
const int LCD_ROW_LEN = 16;

const int ALL_BUTTONS[BTN_COUNT] = {UP_BTN, DOWN_BTN, MODE_BTN};
const char MODE_NAMES[MODE_COUNT][LCD_ROW_LEN] = {
  "ON TIME",
  "OFF TIME",
  "SHALL WAKE",
  "INCREMENT"
};

/* =========================== */
/* ========== ENUMS ========== */
/* =========================== */

enum Button { up, down };

/* =========================== */
/* ======== VARIABLES ======== */
/* =========================== */

bool shallWake = false;
bool isWaking = false;
bool isShooting = false;

int selectedMode = 0;
int buttonStates[BTN_COUNT] = {};
unsigned long btnsLastPressedTime[BTN_COUNT] = {millis(), millis(), millis()};

unsigned long debounceTime = 150;
unsigned long minShutterHoldDuration = 500, minTimeBetweenShots = 500;
unsigned long shutterHoldDuration = 2300;
unsigned long shutterHoldDurationWake = 100;
unsigned long timeBetweenShots = 2000;
unsigned long durationIncrement = 5000;
unsigned long cameraWakeBeforeShot = 3000;

unsigned long lastWakeTime = millis();
unsigned long lastShotTime = millis();
unsigned long lastShotStopTime = millis();
unsigned long currentTime = millis();
#include <string.h>
#include <LiquidCrystal_I2C.h>

#define RELAY_SIGNAL 9
#define UP_BTN 10
#define DOWN_BTN 11
#define MODE_BTN 8

// SDA: A4
// SCL: A5
// Possible addresses: 0x27, 0x38, 0x7C
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x38, 16, 2);

const int BTN_COUNT = 3;
const int MODE_COUNT = 4;

const int ALL_BUTTONS[BTN_COUNT] = {UP_BTN, DOWN_BTN, MODE_BTN};
int buttonStates[BTN_COUNT] = {};

enum Button { up, down };

unsigned long debounceTime = 150;
unsigned long btnsLastPressedTime[BTN_COUNT] = {millis(), millis(), millis()};

bool shallWake = false;
bool isWaking = false;
bool isShooting = false;

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

int selectedMode = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_SIGNAL, OUTPUT);
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
}

void handleShutter(unsigned long currentTime) {
  unsigned long timeSinceLastStop = currentTime - lastShotStopTime;

  if (
    shallWake
    && cameraWakeBeforeShot < timeBetweenShots
    && timeSinceLastStop >= timeBetweenShots - cameraWakeBeforeShot
    && timeSinceLastStop < timeBetweenShots
    && currentTime - lastWakeTime > cameraWakeBeforeShot
  ) {
    isWaking = true;
    lastWakeTime = currentTime;
  }
  else if (!isShooting && timeSinceLastStop >= timeBetweenShots) {
    isShooting = true;
    lastShotTime = currentTime;
  }
  else if (isShooting && currentTime - lastShotTime >= shutterHoldDuration) {
    isShooting = false;
    lastShotStopTime = currentTime;
  }

  if (shallWake && isWaking) {
    digitalWrite(RELAY_SIGNAL, HIGH);
    if (currentTime - lastWakeTime >= shutterHoldDurationWake) {
      digitalWrite(RELAY_SIGNAL, LOW);
      isWaking = false;
    }
  } else {
    digitalWrite(RELAY_SIGNAL, isShooting ? HIGH : LOW);
  }
}

// ================================
// ================ MODE HANDLING ================

typedef void (*ModeHandler)(Button button);

void onDurationModifier(Button button) {
  switch (button) {
  case up:
    shutterHoldDuration += durationIncrement;
    break;
  case down:
    shutterHoldDuration = durationIncrement > shutterHoldDuration ? minShutterHoldDuration : shutterHoldDuration - durationIncrement;
    break;
  }
}

void offDurationModifier(Button button) {
  switch (button) {
  case up:
    timeBetweenShots += durationIncrement;
    break;
  case down:
    timeBetweenShots = durationIncrement > timeBetweenShots ? minTimeBetweenShots : timeBetweenShots - durationIncrement;
    break;
  }
}

void toggleWaker(Button button) {
  shallWake = !shallWake;
}

void incrementorModifier(Button button) {
  switch (button) {
  case up:
    durationIncrement += 500;
    break;
  case down:
    durationIncrement = 500 > durationIncrement ? minTimeBetweenShots : durationIncrement - 500;
    break;
  }
}

ModeHandler MODE_HANDLERS[MODE_COUNT] = {
  onDurationModifier,
  offDurationModifier,
  toggleWaker,
  incrementorModifier
};

const int modeNameLen = 16;
char modeNames[MODE_COUNT][modeNameLen] = {
  "ON TIME",
  "OFF TIME",
  "SHALL WAKE",
  "INCREMENT"
};

// ================================
// ================ BUTTONS ================

typedef void (*ButtonHandler)();

void handleUp() {
  MODE_HANDLERS[selectedMode](up);
}

void handleDown() {
  MODE_HANDLERS[selectedMode](down);
}

void handleMode() {
  selectedMode = (selectedMode + 1) % MODE_COUNT;
}

ButtonHandler BUTTON_HANDLERS[BTN_COUNT] = {
  handleUp,
  handleDown,
  handleMode
};

void handleButtons(unsigned long currentTime) {
  for(int i = 0; i < BTN_COUNT; i++) {
    int buttonReading = !digitalRead(ALL_BUTTONS[i]);
    if (currentTime - btnsLastPressedTime[i] > debounceTime && buttonReading && !buttonStates[i]) {
      btnsLastPressedTime[i] = currentTime;
      buttonStates[i] = HIGH;
      BUTTON_HANDLERS[i]();
    } else if (!buttonReading && buttonStates[i]) {
      buttonStates[i] = LOW;
    }
  }
}

// ================================
// ================ DISPLAY ================

void handleDisplay() {
  char upperText[modeNameLen] = "";
  char lowerText[16] = "";

  strcpy(upperText, modeNames[selectedMode]);
  int upperTextLen = strlen(upperText);
  memset(upperText + upperTextLen, ' ', modeNameLen - upperTextLen);

  switch (selectedMode) {
    case 0:
      sprintf(lowerText, "%ss", String(shutterHoldDuration / 1000.0f).c_str());
      break;
    case 1:
      sprintf(lowerText, "%ss", String(timeBetweenShots / 1000.0f).c_str());
      break;
    case 2:
      sprintf(lowerText, "%s", shallWake ? "YES" : "NO");
      break;
    case 3:
      sprintf(lowerText, "%ss", String(durationIncrement / 1000.0f).c_str());
      break;
    default:
      break;
  }

  int lowerTextLen = strlen(lowerText);
  memset(lowerText + lowerTextLen, ' ', 16 - lowerTextLen);

  lcd.setCursor(0, 0);
  lcd.print(upperText);
  lcd.setCursor(0, 1);
  lcd.print(lowerText);
}

// ================================

void loop() {
  currentTime = millis();

  handleShutter(currentTime);
  handleButtons(currentTime);
  handleDisplay();
}

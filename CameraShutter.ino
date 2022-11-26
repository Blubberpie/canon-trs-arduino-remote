#include <string.h>
#include <LiquidCrystal_I2C.h>
#include "declarations.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_SIGNAL, OUTPUT);
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
}

/* =========================== */
/* ====== MODE HANDLING ====== */
/* =========================== */

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

/* =========================== */
/* ===== BUTTON HANDLING ===== */
/* =========================== */

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

/* =========================== */
/* ========= DISPLAY ========= */
/* =========================== */

void handleDisplay() {
  char upperText[LCD_ROW_LEN] = "";
  char lowerText[LCD_ROW_LEN] = "";

  strcpy(upperText, MODE_NAMES[selectedMode]);
  int upperTextLen = strlen(upperText);
  memset(upperText + upperTextLen, ' ', LCD_ROW_LEN - upperTextLen);

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
  memset(lowerText + lowerTextLen, ' ', LCD_ROW_LEN - lowerTextLen);

  lcd.setCursor(0, 0);
  lcd.print(upperText);
  lcd.setCursor(0, 1);
  lcd.print(lowerText);
}

/* =========================== */
/* ========= SHUTTER ========= */
/* =========================== */

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

void loop() {
  currentTime = millis();

  handleShutter(currentTime);
  handleButtons(currentTime);
  handleDisplay();
}

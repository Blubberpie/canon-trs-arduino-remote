#define RELAY_SIGNAL 9
#define INCR_BTN 10
#define DECR_BTN 11

long int min_shutter_duration = 5000;
long int shutter_duration = 60000;
long int shutter_start = millis();
long int curr_time = millis();

bool is_held = false;
long int shutter_hold_duration = 400;
long int shutter_hold_start = millis();
long int shutter_hold_end = millis();

long int shutter_duration_incr = 15000;

bool should_blink = false;
int blink_state = LOW;
long int led_blink_time = 100;
long int led_off_time = 1000;
int blink_count = 0;
long int last_blink_state_time = millis();
long int last_blink_time = millis();

int last_incr_state = LOW;
int last_decr_state = LOW;

void setup() {
//  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_SIGNAL, OUTPUT);
  pinMode(INCR_BTN, INPUT_PULLUP);
  pinMode(DECR_BTN, INPUT_PULLUP);
}

void handle_blink(long int curr_time) {
  int mins = shutter_duration / 60000;

  //  Serial.println(blink_count);
  if (blink_count > 0) {
    digitalWrite(LED_BUILTIN, blink_state);
  }

  if (!should_blink && curr_time - last_blink_time >= led_off_time) {
    should_blink = true;
  }

  if (should_blink) {
    int last_blink_state = blink_state;
    if (curr_time - last_blink_state_time >= led_blink_time) {
      blink_state = !blink_state;
      last_blink_state_time = millis();
    }
    if (blink_state && blink_state != last_blink_state) {
      blink_count += 1;
    }
    if (blink_count > mins) {
      blink_count = 0;
      should_blink = false;
      last_blink_time = millis();
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void handle_shutter(long int shutter_end) {
//    Serial.println(shutter_duration);

  if (shutter_end - shutter_start >= shutter_duration && shutter_duration >= min_shutter_duration) {
//    Serial.println("Shooting");
    is_held = true;
    shutter_start = millis();
    shutter_hold_start = millis();
  }

  if (is_held) {
    shutter_hold_end = millis();
    long int shutter_hold_elapsed = shutter_hold_end - shutter_hold_start;
    digitalWrite(RELAY_SIGNAL, HIGH);
    if (shutter_hold_elapsed >= shutter_hold_duration) {
      is_held = false;
//      Serial.println("Release");
    }
  } else {
    digitalWrite(RELAY_SIGNAL, LOW);
  }
}

void handle_buttons() {
  int incr_reading = !digitalRead(INCR_BTN);
  int decr_reading = !digitalRead(DECR_BTN);

  if (incr_reading && !last_incr_state) {
    shutter_start = millis();
    shutter_duration += shutter_duration_incr;
    last_incr_state = HIGH;
  } else if (!incr_reading && last_incr_state) {
    last_incr_state = LOW;
  }

  if (decr_reading && !last_decr_state) {
    shutter_start = millis();
    shutter_duration = max(0, shutter_duration - shutter_duration_incr);
    last_decr_state = HIGH;
  } else if (!decr_reading && last_decr_state) {
    last_decr_state = LOW;
  }
}

void loop() {
  curr_time = millis();

  handle_blink(curr_time);
  handle_shutter(curr_time);
  handle_buttons();
}

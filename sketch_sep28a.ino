#include <Wire.h>
#include "uRTCLib.h"
#include <TM1637Display.h>

#define MODE_BTN 23
#define INC_BTN  25
#define DEC_BTN  26
#define SET_BTN  27

int setHour = 0, setMinute = 0, setSecond = 0;
int setDay = 0, setMonth = 0, setYear = 0;
int timeSubState = 0; // 0=HH tens,1=HH units,2=MM tens,3=MM units,4=SS tens,5=SS units
int dateSubState = 0; // 0=DD tens,1=DD units,2=MM tens,3=MM units,4=YYYY hundreds,5=YYYY tens,6=YYYY units
bool timeEditing = false;
bool dateEditing = false;

int mode = 0;
int lastModeBtn = HIGH;
int subState = 0;   // for sequential entry
unsigned long lastDebounce = 0;

int alarmA_hour = 6, alarmA_min = 30;   // default 06:30
int alarmB_hour = 7, alarmB_min = 00;   // default 07:00

const int NUM_MODES = 9;

bool editing = false;
int timerDigits[6] = {0,0,0,0,0,0}; // HHMMSS for entry
int timerSubState = 0; // digit position
bool swRunning = false;
unsigned long swStartTime = 0;
unsigned long swElapsed = 0;
bool timerRunning = false;
unsigned long timerStartTime = 0;
unsigned long timerDuration = 0;
bool timerSet = false;

// RTC
uRTCLib rtc(0x68);

TM1637Display display1(16, 17);
TM1637Display display2(18, 19);

void setup() {
  Serial.begin(115200);
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(INC_BTN, INPUT_PULLUP);
  pinMode(DEC_BTN, INPUT_PULLUP);
  pinMode(SET_BTN, INPUT_PULLUP);
  // In setup()
display1.setBrightness(0x0f); // Sets maximum brightness (0x00 to 0x0f)
// In setup()
display2.setBrightness(0x0f); // Sets maximum brightness (0x00 to 0x0f)

rtc.set(0, 0, 12, 5, 4, 10, 25);
}

void loop() {
  checkModeButton();

  switch (mode) {
    case 0: showMode1();break;
    case 1: showMode2(); break;
    case 2: showMode3(); break;
    case 3: setAlarmA(); break;  // A
    case 4: setAlarmB(); break;  // B
    case 5: runStopwatch(); break;
    case 6: runTimer(); break;
    case 7: setTime(); break;
    case 8: setDate(); break;
  }
}

// Button to cycle modes
void checkModeButton() {
  int reading = digitalRead(MODE_BTN);
  if (reading == LOW && lastModeBtn == HIGH && millis() - lastDebounce > 200) {
    // Abort any editing
    subState = 0;
    editing = false;
    timeEditing = false;
    dateEditing = false;
    timerSet = false;  // if using a flag for timer entry

    // Change mode
    mode++;
    if (mode >= NUM_MODES) mode = 0;
    lastDebounce = millis();
  }
  lastModeBtn = reading;
}

void showMode1() {
  rtc.refresh();
  int hour = rtc.hour();
  int minute = rtc.minute();
  int second = rtc.second();

  // Convert to 12h format with AM/PM
  bool pm = false;
  int hour12 = hour;
  if (hour == 0) hour12 = 12;
  else if (hour >= 12) {
    pm = true;
    if (hour > 12) hour12 -= 12;
  }

  // HH:MM on display1
  int timeVal = hour12 * 100 + minute;
  display1.showNumberDecEx(timeVal, 0b01000000, true);
  Serial.print(timeVal);
  hour,
  minute
  display2.clear();
  display2.showNumberDec(second, true, 2, 0);
  // SS on display2 + AM/PM indicator
  if (pm) {
    // Add crude PM indicator on last digit (underscore position)
    uint8_t segP[] = { SEG_A | SEG_B | SEG_E | SEG_F | SEG_G }; // "P"
    display2.setSegments(segP, 1, 2);
  } else {
    // "A"
    uint8_t segA[] = { SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G };
    display2.setSegments(segA, 1, 2);
    Serial.print(second);
  }
}
void showMode2() {
  rtc.refresh();
  int hour = rtc.hour();
  int minute = rtc.minute();
  int day = rtc.day();
  int month = rtc.month();

  // HH:MM on display1
  int timeVal = hour * 100 + minute;
  display1.showNumberDecEx(timeVal, 0b01000000, true);

  // DDMM on display2
  int dateVal = day * 100 + month;
  display2.showNumberDec(dateVal, true);
}
void showMode3() {
  rtc.refresh();
  int day = rtc.day();
  int month = rtc.month();
  int year = rtc.year();

  // DDMM on display1
  int dateVal = day * 100 + month;
  display1.showNumberDec(dateVal, true);

  // YYYY on display2
  display2.showNumberDec(year, true);
}
void setAlarmA() {
  static int tempHour = alarmA_hour;
  static int tempMin  = alarmA_min;

  // --- Button handling ---
  if (digitalRead(INC_BTN) == LOW) {
    if (subState == 0) { tempHour++; if (tempHour > 23) tempHour = 0; }
    if (subState == 1) { tempMin++;  if (tempMin > 59)  tempMin  = 0; }
    delay(200);
  }
  if (digitalRead(DEC_BTN) == LOW) {
    if (subState == 0) { tempHour--; if (tempHour < 0)  tempHour = 23; }
    if (subState == 1) { tempMin--;  if (tempMin < 0)   tempMin  = 59; }
    delay(200);
  }
  if (digitalRead(SET_BTN) == LOW) {
    subState++;
    delay(200);
    if (subState > 1) {
      // Confirm -> save alarm
      alarmA_hour = tempHour;
      alarmA_min  = tempMin;
      subState = 0;
      editing = false;
      mode = 0; // back to normal time display
    }
  }

  // --- Display current entry ---
  int alarmVal = tempHour * 100 + tempMin;
  display1.showNumberDecEx(alarmVal, 0b01000000, true);
  display2.clear();
}
void setAlarmB() {
  static int tempHour = alarmB_hour;
  static int tempMin  = alarmB_min;

  if (digitalRead(INC_BTN) == LOW) {
    if (subState == 0) { tempHour++; if (tempHour > 23) tempHour = 0; }
    if (subState == 1) { tempMin++;  if (tempMin > 59)  tempMin  = 0; }
    delay(200);
  }
  if (digitalRead(DEC_BTN) == LOW) {
    if (subState == 0) { tempHour--; if (tempHour < 0)  tempHour = 23; }
    if (subState == 1) { tempMin--;  if (tempMin < 0)   tempMin  = 59; }
    delay(200);
  }
  if (digitalRead(SET_BTN) == LOW) {
    subState++;
    delay(200);
    if (subState > 1) {
      alarmB_hour = tempHour;
      alarmB_min  = tempMin;
      subState = 0;
      editing = false;
      mode = 0; // back to normal time display
    }
  }

  int alarmVal = tempHour * 100 + tempMin;
  display1.showNumberDecEx(alarmVal, 0b01000000, true);
  display2.clear();
}
void runStopwatch() {
  static int lastInc = HIGH;
  static int lastDec = HIGH;
  static int lastSet = HIGH;

  int inc = digitalRead(INC_BTN);
  int dec = digitalRead(DEC_BTN);
  int set = digitalRead(SET_BTN);

  unsigned long now = millis();

  // Start
  if (inc == LOW && lastInc == HIGH) {
    if (!swRunning) {
      swStartTime = now - swElapsed; // resume from elapsed
      swRunning = true;
    }
  }

  // Stop
  if (dec == LOW && lastDec == HIGH) {
    if (swRunning) {
      swElapsed = now - swStartTime;
      swRunning = false;
    }
  }

  // Reset
  if (set == LOW && lastSet == HIGH) {
    swRunning = false;
    swElapsed = 0;
  }

  lastInc = inc;
  lastDec = dec;
  lastSet = set;

  // Update display
  unsigned long displayTime = swRunning ? (now - swStartTime) : swElapsed;
  int totalSeconds = displayTime / 1000;
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  int hundredths = (displayTime % 1000) / 10;

  // MMSS on display1
  display1.showNumberDecEx(minutes * 100 + seconds, 0b01000000, true);

  // Hundredths on display2
  display2.showNumberDec(hundredths, true, 2, 0);
}

void runTimer() {
  static int tempHour = 0, tempMin = 0, tempSec = 0;
  static bool timerSet = false;
  unsigned long now = millis();

  // --- Setting timer ---
  if (!timerRunning && !timerSet) {
    if (digitalRead(INC_BTN) == LOW) {
      if (timerSubState < 6) {
        timerDigits[timerSubState]++;
        if ((timerSubState == 0 && timerDigits[0] > 2) || (timerSubState == 1 && timerDigits[1] > 3)) timerDigits[timerSubState] = 0;
        if ((timerSubState == 2 && timerDigits[2] > 5)) timerDigits[timerSubState] = 0;
        if ((timerSubState == 4 && timerDigits[4] > 5)) timerDigits[timerSubState] = 0;
        delay(200);
      }
    }
    if (digitalRead(DEC_BTN) == LOW) {
      if (timerSubState < 6) {
        timerDigits[timerSubState]--;
        if (timerDigits[timerSubState] < 0) timerDigits[timerSubState] = 9;
        delay(200);
      }
    }
    if (digitalRead(SET_BTN) == LOW) {
      timerSubState++;
      delay(200);
      if (timerSubState >= 6) {
        tempHour = timerDigits[0]*10 + timerDigits[1];
        tempMin  = timerDigits[2]*10 + timerDigits[3];
        tempSec  = timerDigits[4]*10 + timerDigits[5];
        timerDuration = (tempHour*3600 + tempMin*60 + tempSec)*1000UL;
        timerStartTime = now;
        timerRunning = true;
        timerSet = true;
      }
    }

    // Display entry
    display1.showNumberDecEx(timerDigits[0]*1000 + timerDigits[1]*100 + timerDigits[2]*10 + timerDigits[3], 0b01000000, true);
    display2.showNumberDec(timerDigits[4]*10 + timerDigits[5], true, 2, 0);
  }

  // --- Running timer ---
  if (timerRunning) {
    long remaining = timerDuration - (now - timerStartTime);
    if (remaining <= 0) {
      remaining = 0;
      timerRunning = false; // stop timer
      // optional: trigger buzzer/alarm here
    }

    int remSec = remaining / 1000;
    int h = remSec / 3600;
    int m = (remSec % 3600) / 60;
    int s = remSec % 60;

    display1.showNumberDecEx(h*100 + m, 0b01000000, true);
    display2.showNumberDec(s, true, 2, 0);
  }
}
void setTime() {
  rtc.refresh();
  unsigned long now = millis();

  if (!timeEditing) {
    // initialize with current RTC time
    setHour   = rtc.hour();
    setMinute = rtc.minute();
    setSecond = rtc.second();
    timeSubState = 0;
    timeEditing = true;
  }

  // Button handling
  if (digitalRead(INC_BTN) == LOW) {
    switch(timeSubState) {
      case 0: setHour = (setHour + 10) % 24; break;
      case 1: setHour = (setHour + 1) % 24; break;
      case 2: setMinute = (setMinute + 10) % 60; break;
      case 3: setMinute = (setMinute + 1) % 60; break;
      case 4: setSecond = (setSecond + 10) % 60; break;
      case 5: setSecond = (setSecond + 1) % 60; break;
    }
    delay(200);
  }
  if (digitalRead(DEC_BTN) == LOW) {
    switch(timeSubState) {
      case 0: setHour = (setHour + 14) % 24; break; // -10 mod 24
      case 1: setHour = (setHour + 23) % 24; break; // -1 mod 24
      case 2: setMinute = (setMinute + 50) % 60; break; // -10
      case 3: setMinute = (setMinute + 59) % 60; break; // -1
      case 4: setSecond = (setSecond + 50) % 60; break;
      case 5: setSecond = (setSecond + 59) % 60; break;
    }
    delay(200);
  }
  if (digitalRead(SET_BTN) == LOW) {
    timeSubState++;
    delay(200);
    if (timeSubState > 5) {
      rtc.set(setSecond, setMinute, setHour, rtc.dayOfWeek(), rtc.day(), rtc.month(), rtc.year());
      timeEditing = false;
      timeSubState = 0;
      mode = 0; // back to normal display
    }
  }

  // Display HHMM on display1, SS on display2
  display1.showNumberDecEx(setHour*100 + setMinute, 0b01000000, true);
  display2.showNumberDec(setSecond, true, 2, 0);
}
void setDate() {
  rtc.refresh();

  if (!dateEditing) {
    // initialize with current RTC date
    setDay   = rtc.day();
    setMonth = rtc.month();
    setYear  = rtc.year();
    dateSubState = 0;
    dateEditing = true;
  }

  // Button handling
  if (digitalRead(INC_BTN) == LOW) {
    switch(dateSubState) {
      case 0: setDay = (setDay + 10) % 32; break;
      case 1: setDay = (setDay + 1) % 32; break;
      case 2: setMonth = (setMonth + 10) % 13; break;
      case 3: setMonth = (setMonth + 1) % 13; break;
      case 4: setYear += 1000; break;
      case 5: setYear += 100; break;
      case 6: setYear += 10; break;
      case 7: setYear += 1; break;
    }
    delay(200);
  }

  if (digitalRead(DEC_BTN) == LOW) {
    switch(dateSubState) {
      case 0: setDay = (setDay + 22) % 32; break;
      case 1: setDay = (setDay + 31) % 32; break;
      case 2: setMonth = (setMonth + 3) % 13; break;
      case 3: setMonth = (setMonth + 12) % 13; break;
      case 4: setYear -= 1000; break;
      case 5: setYear -= 100; break;
      case 6: setYear -= 10; break;
      case 7: setYear -= 1; break;
    }
    delay(200);
  }

  if (digitalRead(SET_BTN) == LOW) {
    dateSubState++;
    delay(200);
    if (dateSubState > 7) {
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), rtc.dayOfWeek(), setDay, setMonth, setYear);
      dateEditing = false;
      dateSubState = 0;
      mode = 0; // back to normal display
    }
  }

  // Display DDMM on display1, YYYY on display2
  display1.showNumberDec(setDay*100 + setMonth, true);
  display2.showNumberDec(setYear, true);
}


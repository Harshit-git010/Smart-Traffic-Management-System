/*
  ============================================================
  SMART TRAFFIC MANAGEMENT SYSTEM - Arduino Mega
  ============================================================
  Lanes      : Alpha, Beta, Gamma, Delta
  Sequence   : Alpha -> Gamma -> Delta -> Beta -> (repeat)

  Each lane has:
    - An RGB signal head (separate Red / Green / Blue pins)
    - An emergency slide switch wired to GND (INPUT_PULLUP)

  VEHICLE COUNT
    Currently simulated with random numbers (3-15) via
    getVehicleCount(). Replace this function later with real
    IR / Ultrasonic sensor reading logic - no other code needs
    to change.

  GREEN TIME FORMULA
    greenTime (sec) = (vehicleCount * 7) + 8

  EMERGENCY MODE
    Sliding ANY lane's emergency switch (pin goes LOW):
      - That lane's GREEN turns ON (RED & BLUE off)
      - All other lanes turn BLUE (RED & GREEN off)
      - LCD shows "** EMERGENCY **" and the active lane name
    Sliding the switch back (pin goes HIGH again):
      - Normal cycle resumes exactly where it left off
        (the interrupted countdown continues)

  LCD
    16x2 I2C display. Default address used here is 0x27.
    If your display doesn't show anything, try 0x3F instead
    (run an I2C scanner sketch to confirm the address).

  REQUIRED LIBRARY
    "LiquidCrystal I2C" by Frank de Brabander
    (Install via Arduino IDE -> Library Manager)

  ============================================================
  PIN MAP
  ============================================================
  Lane   | Red | Green | Blue | Emergency Switch
  -------|-----|-------|------|------------------
  Alpha  |  5  |   7   |  6   |       14
  Beta   |  8  |  10   |  9   |       15
  Gamma  |  2  |   4   |  3   |       16
  Delta  | 11  |  13   | 12   |       17

  LCD (I2C) -> SDA = pin 20, SCL = pin 21 (Mega hardware I2C)
  ============================================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---- LCD setup (change 0x27 to 0x3F if your module needs it) ----
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---- Lane configuration ----
struct Lane {
  const char* name;
  uint8_t redPin;
  uint8_t greenPin;
  uint8_t bluePin;
  uint8_t emergencyPin;
};

// Index:        0       1      2       3
Lane lanes[4] = {
  {"Alpha",  5,  7,  6, 14},   // 0 - Alpha
  {"Beta",   8, 10,  9, 15},   // 1 - Beta
  {"Gamma",  2,  4,  3, 16},   // 2 - Gamma
  {"Delta", 11, 13, 12, 17}    // 3 - Delta
};

// Operating sequence: Alpha -> Gamma -> Delta -> Beta
uint8_t sequence[4] = {0, 2, 3, 1};

const int VEHICLE_CROSS_TIME = 7;   // seconds each vehicle takes to cross
const int BUFFER_TIME        = 8;   // extra buffer seconds

// ============================================================
void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(lanes[i].redPin, OUTPUT);
    pinMode(lanes[i].greenPin, OUTPUT);
    pinMode(lanes[i].bluePin, OUTPUT);
    pinMode(lanes[i].emergencyPin, INPUT_PULLUP);
  }

  lcd.init();
  lcd.backlight();

  randomSeed(analogRead(A0));   // seed RNG from floating analog pin

  allRed();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Traffic");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(1500);
}

// ============================================================
void loop() {
  for (int i = 0; i < 4; i++) {
    int laneIdx       = sequence[i];
    int vehicleCount  = getVehicleCount(laneIdx);
    int greenTime     = (vehicleCount * VEHICLE_CROSS_TIME) + BUFFER_TIME;

    runSignal(laneIdx, vehicleCount, greenTime);
  }
}

// ============================================================
// Returns a simulated vehicle count for a lane (3-15).
// Replace the body with real IR/Ultrasonic sensor code later.
// ============================================================
int getVehicleCount(int laneIdx) {
  return random(3, 16);   // random(min, maxExclusive) -> 3..15
}

// ============================================================
// All lanes RED (idle / startup state)
// ============================================================
void allRed() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(lanes[i].redPin,   HIGH);
    digitalWrite(lanes[i].greenPin, LOW);
    digitalWrite(lanes[i].bluePin,  LOW);
  }
}

// ============================================================
// Set one lane GREEN, all other lanes RED
// ============================================================
void setActiveLane(int activeLane) {
  for (int i = 0; i < 4; i++) {
    if (i == activeLane) {
      digitalWrite(lanes[i].redPin,   LOW);
      digitalWrite(lanes[i].greenPin, HIGH);
      digitalWrite(lanes[i].bluePin,  LOW);
    } else {
      digitalWrite(lanes[i].redPin,   HIGH);
      digitalWrite(lanes[i].greenPin, LOW);
      digitalWrite(lanes[i].bluePin,  LOW);
    }
  }
}

// ============================================================
// Run one lane's green phase with a live countdown.
// Checks emergency switches every second.
// ============================================================
void runSignal(int laneIdx, int vehicleCount, int greenTime) {
  setActiveLane(laneIdx);
  showSignalScreen(laneIdx, vehicleCount, greenTime);

  for (int t = greenTime; t > 0; t--) {
    if (emergencyActive()) {
      handleEmergency();

      // Restore this lane's normal state once emergency ends
      setActiveLane(laneIdx);
      showSignalScreen(laneIdx, vehicleCount, t);
    }

    updateCountdown(t);
    delay(1000);
  }
}

// ============================================================
// True if ANY emergency slide switch is active (pin == LOW)
// ============================================================
bool emergencyActive() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(lanes[i].emergencyPin) == LOW) return true;
  }
  return false;
}

// ============================================================
// Emergency mode handler.
// Keeps running until ALL emergency switches are released.
// Supports switching between lanes mid-emergency.
// ============================================================
void handleEmergency() {
  while (emergencyActive()) {
    int emLane = -1;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(lanes[i].emergencyPin) == LOW) {
        emLane = i;
        break;   // first active switch found gets priority
      }
    }

    for (int i = 0; i < 4; i++) {
      digitalWrite(lanes[i].redPin, LOW);
      digitalWrite(lanes[i].greenPin, (i == emLane) ? HIGH : LOW);
      digitalWrite(lanes[i].bluePin,  (i == emLane) ? LOW  : HIGH);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("** EMERGENCY **");
    lcd.setCursor(0, 1);
    lcd.print(lanes[emLane].name);
    lcd.print(" - GREEN");

    delay(300);   // re-poll switches periodically
  }

  lcd.clear();
}

// ============================================================
// LCD: draw the static part of the normal signal screen
// Line 1: "<Lane> - GREEN"
// Line 2: "V:nn T:nnns"
// ============================================================
void showSignalScreen(int laneIdx, int vehicleCount, int initialTime) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lanes[laneIdx].name);
  lcd.print(" - GREEN");

  char buf[5];

  lcd.setCursor(0, 1);
  lcd.print("V:");
  sprintf(buf, "%2d", vehicleCount);
  lcd.print(buf);

  lcd.print(" T:");
  sprintf(buf, "%3d", initialTime);
  lcd.print(buf);
  lcd.print("s");
}

// ============================================================
// LCD: update only the countdown digits (cols 7-9 of row 1)
// ============================================================
void updateCountdown(int t) {
  char buf[4];
  sprintf(buf, "%3d", t);

  lcd.setCursor(7, 1);
  lcd.print(buf);
  lcd.print("s");
}

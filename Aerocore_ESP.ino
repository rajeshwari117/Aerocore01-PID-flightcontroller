#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ----------------- OLED SETUP -----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----------------- PIN SETUP -----------------
const int trigPin = 5;
const int echoPin = 18;

const int greenPin = 25;
const int yellowPin = 26;
const int redPin = 27;

// ----------------- FILTERING -----------------
const int FILTER_SIZE = 5;
float readings[FILTER_SIZE];
int readIndex = 0;

// ----------------- PID VARIABLES -----------------
float targetHeight = 20.0;   
float Kp = 2.0;
float Ki = 0.5;
float Kd = 1.0;

float previousError = 0;
float integral = 0;
unsigned long lastPIDTime = 0;

// ----------------- STATE MACHINE -----------------
enum FlightMode { IDLE, TAKEOFF, HOLD, LANDING, EMERGENCY };
FlightMode currentMode = IDLE;

int stableCount = 0;          // counts consecutive stable readings near target
int invalidCount = 0;         // counts consecutive invalid sensor readings

// SETUP

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);

  // Initialize filter array with a first raw reading
  float firstReading = readRawDistance();
  if (firstReading < 0) firstReading = 0; 
  for (int i = 0; i < FILTER_SIZE; i++) {
    readings[i] = firstReading;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found - check SDA/SCL wiring");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Flight Controller");
  display.println("Type 'help' in");
  display.println("Serial Monitor");
  display.display();

  lastPIDTime = millis();

  Serial.println("=== ESP32 Mini Flight Controller ===");
  printHelp();
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  start        - begin takeoff toward target height");
  Serial.println("  land         - begin landing (only works in HOLD mode)");
  Serial.println("  reset        - recover from EMERGENCY mode");
  Serial.println("  kp <value>   - set Kp live, e.g. kp 3.5");
  Serial.println("  ki <value>   - set Ki live, e.g. ki 0.8");
  Serial.println("  kd <value>   - set Kd live, e.g. kd 1.2");
  Serial.println("  target <val> - set target height in cm, e.g. target 15");
  Serial.println("  status       - print current Kp/Ki/Kd/target/mode");
}

// SENSOR: RAW READING

float readRawDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1; // -1 means "invalid reading"

  float distanceCm = duration * 0.034 / 2;
  return distanceCm;
}

// SENSOR: FILTERED READING (Moving Average)

float readFilteredDistance() {
  float raw = readRawDistance();

  if (raw < 0 || raw > 400) {
    invalidCount++;
    return -1;
  }
  invalidCount = 0;

  readings[readIndex] = raw;
  readIndex = (readIndex + 1) % FILTER_SIZE;

  float sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    sum += readings[i];
  }
  return sum / FILTER_SIZE;
}

// PID CALCULATION

float computePID(float current) {
  unsigned long now = millis();
  float dt = (now - lastPIDTime) / 1000.0;
  if (dt <= 0) dt = 0.001;

  float error = targetHeight - current;

  integral += error * dt;
  float derivative = (error - previousError) / dt;

  float output = (Kp * error) + (Ki * integral) + (Kd * derivative);

  previousError = error;
  lastPIDTime = now;

  return output;
}

// STATE MACHINE LOGIC

void updateStateMachine(float current, float error) {
  if (invalidCount >= 5 && currentMode != EMERGENCY) {
    currentMode = EMERGENCY;
    Serial.println(">>> EMERGENCY: invalid sensor readings <<<");
    return;
  }

  switch (currentMode) {
    case IDLE:
      break;

    case TAKEOFF:
      if (abs(error) < 2.0) {
        stableCount++;
        if (stableCount > 10) {
          currentMode = HOLD;
          Serial.println(">>> Reached HOLD mode <<<");
          stableCount = 0;
        }
      } else {
        stableCount = 0;
      }
      break;

    case HOLD:
      break;

    case LANDING:
      if (current >= 0 && current < 5.0) {
        currentMode = IDLE;
        Serial.println(">>> Landed. Back to IDLE <<<");
      }
      break;

    case EMERGENCY:
      break;
  }
}


void handleSerialCommands() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "start" && currentMode == IDLE) {
      currentMode = TAKEOFF;
      integral = 0;
      Serial.println(">>> TAKEOFF initiated <<<");
    }
    else if (cmd == "land" && currentMode == HOLD) {
      currentMode = LANDING;
      Serial.println(">>> LANDING initiated <<<");
    }
    else if (cmd == "reset" && currentMode == EMERGENCY) {
      currentMode = IDLE;
      invalidCount = 0;
      Serial.println(">>> Reset to IDLE <<<");
    }
    else if (cmd.startsWith("kp ")) {
      Kp = cmd.substring(3).toFloat();
      Serial.print(">>> Kp updated to: "); Serial.println(Kp);
    }
    else if (cmd.startsWith("ki ")) {
      Ki = cmd.substring(3).toFloat();
      Serial.print(">>> Ki updated to: "); Serial.println(Ki);
    }
    else if (cmd.startsWith("kd ")) {
      Kd = cmd.substring(3).toFloat();
      Serial.print(">>> Kd updated to: "); Serial.println(Kd);
    }
    else if (cmd.startsWith("target ")) {
      targetHeight = cmd.substring(7).toFloat();
      Serial.print(">>> Target height updated to: "); Serial.println(targetHeight);
    }
    else if (cmd == "status") {
      Serial.print("Mode: ");
      switch (currentMode) {
        case IDLE: Serial.println("IDLE"); break;
        case TAKEOFF: Serial.println("TAKEOFF"); break;
        case HOLD: Serial.println("HOLD"); break;
        case LANDING: Serial.println("LANDING"); break;
        case EMERGENCY: Serial.println("EMERGENCY"); break;
      }
      Serial.print("Kp: "); Serial.print(Kp);
      Serial.print(" | Ki: "); Serial.print(Ki);
      Serial.print(" | Kd: "); Serial.println(Kd);
      Serial.print("Target height: "); Serial.println(targetHeight);
    }

  }
}

// LEDS: REFLECT CURRENT STATE

void updateLEDs() {
  digitalWrite(greenPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(redPin, LOW);

  switch (currentMode) {
    case IDLE:
      digitalWrite(greenPin, HIGH);
      break;
    case TAKEOFF:
      digitalWrite(yellowPin, (millis() / 300) % 2);
      break;
    case HOLD:
      digitalWrite(greenPin, HIGH);
      break;
    case LANDING:
      digitalWrite(yellowPin, HIGH);
      break;
    case EMERGENCY:
      digitalWrite(redPin, (millis() / 200) % 2);
      break;
  }
}

// OLED: LIVE TELEMETRY

void updateOLED(float current, float error, float pidOutput) {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("Mode: ");
  switch (currentMode) {
    case IDLE: display.println("IDLE"); break;
    case TAKEOFF: display.println("TAKEOFF"); break;
    case HOLD: display.println("HOLD"); break;
    case LANDING: display.println("LANDING"); break;
    case EMERGENCY: display.println("EMERGENCY"); break;
  }

  display.print("Target: "); display.print(targetHeight); display.println(" cm");
  display.print("Current: "); display.print(current); display.println(" cm");
  display.print("Error: "); display.println(error);
  display.print("PID out: "); display.println(pidOutput);

  display.display();
}


void loop() {
  handleSerialCommands();

  float current = readFilteredDistance();
  float error = 0;
  float pidOutput = 0;

  if (current >= 0) {
    error = targetHeight - current;

    if (currentMode == TAKEOFF || currentMode == HOLD) {
      pidOutput = computePID(current);
    }

    updateStateMachine(current, error);
  }

  updateLEDs();
  updateOLED(current, error, pidOutput);

  
  Serial.print("Target:"); Serial.print(targetHeight);
  Serial.print(",Current:"); Serial.print(current);
  Serial.print(",PIDOutput:"); Serial.println(pidOutput);

  delay(200);
}
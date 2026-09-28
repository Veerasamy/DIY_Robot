/*
 * SpeedBot ESP32 Motor Bridge Firmware
 * ======================================
 * Receives serial commands from Jetson Orin Nano (USB Serial)
 * and outputs PWM signals to:
 *   - Steering servo (channel 1)
 *   - Throttle ESC   (channel 2)
 *
 * Serial Protocol (115200 baud):
 *   Command format:  "S<steer_pwm>,T<throttle_pwm>\n"
 *   Example:         "S1500,T1500\n"  (neutral position)
 *   Response:        "OK S1500 T1500\n"
 *
 *   Special commands:
 *     "STOP\n"     — set both to neutral immediately
 *     "ARM\n"      — run ESC arming sequence
 *     "PING\n"     — heartbeat check, responds "PONG\n"
 *     "STATUS\n"   — responds with current PWM values + uptime
 *
 * Safety Features:
 *   - Watchdog: auto-stops if no command received for 500ms
 *   - PWM clamping: prevents values outside safe range
 *   - ESC arming sequence on startup
 *   - LED status indicator
 *
 * Wiring:
 *   ESP32 Pin 18 → Steering Servo signal (orange wire)
 *   ESP32 Pin 19 → Throttle ESC signal (white wire)
 *   ESP32 GND    → Servo GND + ESC GND (brown/black wire)
 *   (Power servo and ESC from battery, NOT from ESP32!)
 *
 * Board: ESP32 Dev Module
 * Upload: Arduino IDE or PlatformIO
 */

#include <ESP32Servo.h>

// ══════════════════════════════════════════════════
//  PIN CONFIGURATION — Change these for your wiring
// ══════════════════════════════════════════════════
#define STEERING_PIN      18    // GPIO for steering servo
#define THROTTLE_PIN      19    // GPIO for throttle ESC
#define STATUS_LED_PIN    2     // Built-in LED (GPIO2 on most ESP32 boards)

// ══════════════════════════════════════════════════
//  PWM LIMITS — Adjust for your servo and ESC
// ══════════════════════════════════════════════════
// Steering servo
#define STEER_MIN         1000  // full left (μs)
#define STEER_CENTER      1500  // center (μs)
#define STEER_MAX         2000  // full right (μs)

// Throttle ESC
#define THROTTLE_MIN      1000  // full reverse (μs)
#define THROTTLE_NEUTRAL  1500  // neutral / stop (μs)
#define THROTTLE_MAX      2000  // full forward (μs)

// ══════════════════════════════════════════════════
//  SAFETY
// ══════════════════════════════════════════════════
#define WATCHDOG_TIMEOUT_MS   500   // auto-stop if no command for this long
#define SERIAL_BAUD           115200
#define CMD_BUFFER_SIZE       64

// ══════════════════════════════════════════════════
//  GLOBAL STATE
// ══════════════════════════════════════════════════
Servo steeringServo;
Servo throttleESC;

int currentSteerPWM = STEER_CENTER;
int currentThrottlePWM = THROTTLE_NEUTRAL;

unsigned long lastCommandTime = 0;
unsigned long startTime = 0;
bool isArmed = false;
bool watchdogTriggered = false;

char cmdBuffer[CMD_BUFFER_SIZE];
int cmdIndex = 0;

// ══════════════════════════════════════════════════
//  SETUP
// ══════════════════════════════════════════════════
void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) { delay(10); }

    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Attach servos
    // ESP32Servo library handles LEDC channel allocation automatically
    steeringServo.setPeriodHertz(50);   // Standard 50Hz servo
    throttleESC.setPeriodHertz(50);     // ESC also expects 50Hz

    steeringServo.attach(STEERING_PIN, STEER_MIN, STEER_MAX);
    throttleESC.attach(THROTTLE_PIN, THROTTLE_MIN, THROTTLE_MAX);

    // Start at neutral
    steeringServo.writeMicroseconds(STEER_CENTER);
    throttleESC.writeMicroseconds(THROTTLE_NEUTRAL);

    startTime = millis();
    lastCommandTime = millis();

    // Auto-arm ESC on startup
    armESC();

    Serial.println("SPEEDBOT_ESP32_READY");
    Serial.println("Protocol: S<steer>,T<throttle>\\n");

    // Status LED on = ready
    digitalWrite(STATUS_LED_PIN, HIGH);
}

// ══════════════════════════════════════════════════
//  MAIN LOOP
// ══════════════════════════════════════════════════
void loop() {
    // Read serial commands
    while (Serial.available() > 0) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (cmdIndex > 0) {
                cmdBuffer[cmdIndex] = '\0';
                processCommand(cmdBuffer);
                cmdIndex = 0;
            }
        } else if (cmdIndex < CMD_BUFFER_SIZE - 1) {
            cmdBuffer[cmdIndex++] = c;
        } else {
            // Buffer overflow — reset
            cmdIndex = 0;
        }
    }

    // Watchdog — stop if no commands received recently
    checkWatchdog();

    // Status LED blink pattern
    updateStatusLED();

    // Small delay to prevent tight loop
    delay(1);
}

// ══════════════════════════════════════════════════
//  COMMAND PROCESSING
// ══════════════════════════════════════════════════
void processCommand(const char* cmd) {
    lastCommandTime = millis();
    watchdogTriggered = false;

    // ── STOP command ──
    if (strcmp(cmd, "STOP") == 0) {
        stopMotors();
        Serial.println("OK STOPPED");
        return;
    }

    // ── PING command ──
    if (strcmp(cmd, "PING") == 0) {
        Serial.println("PONG");
        return;
    }

    // ── ARM command ──
    if (strcmp(cmd, "ARM") == 0) {
        armESC();
        Serial.println("OK ARMED");
        return;
    }

    // ── STATUS command ──
    if (strcmp(cmd, "STATUS") == 0) {
        sendStatus();
        return;
    }

    // ── Drive command: S<steer>,T<throttle> ──
    int steerVal = 0, throttleVal = 0;
    if (parseDriveCommand(cmd, &steerVal, &throttleVal)) {
        setMotors(steerVal, throttleVal);

        // Compact acknowledgement
        Serial.print("OK S");
        Serial.print(currentSteerPWM);
        Serial.print(" T");
        Serial.println(currentThrottlePWM);
    } else {
        Serial.print("ERR PARSE: ");
        Serial.println(cmd);
    }
}

bool parseDriveCommand(const char* cmd, int* steer, int* throttle) {
    // Expected format: "S1500,T1500"
    if (cmd[0] != 'S') return false;

    // Find 'S' value
    const char* sStart = cmd + 1;
    char* comma = strchr(sStart, ',');
    if (comma == NULL) return false;

    // Find 'T' value
    if (*(comma + 1) != 'T') return false;
    const char* tStart = comma + 2;

    *steer = atoi(sStart);
    *throttle = atoi(tStart);

    // Validate ranges
    if (*steer < 500 || *steer > 2500) return false;
    if (*throttle < 500 || *throttle > 2500) return false;

    return true;
}

// ══════════════════════════════════════════════════
//  MOTOR CONTROL
// ══════════════════════════════════════════════════
void setMotors(int steerPWM, int throttlePWM) {
    // Clamp to safe ranges
    currentSteerPWM = constrain(steerPWM, STEER_MIN, STEER_MAX);
    currentThrottlePWM = constrain(throttlePWM, THROTTLE_MIN, THROTTLE_MAX);

    steeringServo.writeMicroseconds(currentSteerPWM);
    throttleESC.writeMicroseconds(currentThrottlePWM);
}

void stopMotors() {
    currentSteerPWM = STEER_CENTER;
    currentThrottlePWM = THROTTLE_NEUTRAL;

    steeringServo.writeMicroseconds(STEER_CENTER);
    throttleESC.writeMicroseconds(THROTTLE_NEUTRAL);
}

void armESC() {
    /*
     * ESC Arming Sequence:
     * Most hobby ESCs need to see a neutral throttle signal for 2-3 seconds
     * on power-up before they accept throttle commands.
     *
     * Modify this sequence if your ESC has a different arming procedure.
     */
    Serial.println("ARMING ESC...");

    throttleESC.writeMicroseconds(THROTTLE_NEUTRAL);
    steeringServo.writeMicroseconds(STEER_CENTER);

    // Hold neutral for 3 seconds
    for (int i = 0; i < 30; i++) {
        delay(100);
        digitalWrite(STATUS_LED_PIN, (i % 2 == 0) ? HIGH : LOW);  // blink during arm
    }

    isArmed = true;
    Serial.println("ESC ARMED");
}

// ══════════════════════════════════════════════════
//  SAFETY — WATCHDOG
// ══════════════════════════════════════════════════
void checkWatchdog() {
    if (millis() - lastCommandTime > WATCHDOG_TIMEOUT_MS) {
        if (!watchdogTriggered) {
            watchdogTriggered = true;
            stopMotors();
            Serial.println("WATCHDOG STOP");
        }
    }
}

// ══════════════════════════════════════════════════
//  STATUS
// ══════════════════════════════════════════════════
void sendStatus() {
    unsigned long uptime = (millis() - startTime) / 1000;

    Serial.print("STATUS S");
    Serial.print(currentSteerPWM);
    Serial.print(" T");
    Serial.print(currentThrottlePWM);
    Serial.print(" ARMED=");
    Serial.print(isArmed ? "YES" : "NO");
    Serial.print(" WDG=");
    Serial.print(watchdogTriggered ? "TRIPPED" : "OK");
    Serial.print(" UP=");
    Serial.print(uptime);
    Serial.println("s");
}

// ══════════════════════════════════════════════════
//  STATUS LED
// ══════════════════════════════════════════════════
void updateStatusLED() {
    unsigned long now = millis();

    if (watchdogTriggered) {
        // Fast blink = watchdog tripped (no commands)
        digitalWrite(STATUS_LED_PIN, (now / 100) % 2 == 0 ? HIGH : LOW);
    } else if (!isArmed) {
        // Slow blink = not armed
        digitalWrite(STATUS_LED_PIN, (now / 500) % 2 == 0 ? HIGH : LOW);
    } else {
        // Solid = armed and receiving commands
        digitalWrite(STATUS_LED_PIN, HIGH);
    }
}

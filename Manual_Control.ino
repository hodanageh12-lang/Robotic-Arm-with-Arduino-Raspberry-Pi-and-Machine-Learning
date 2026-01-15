#include <Arduino.h>
#include <Stepper.h>

//Control with Joystick 
const int JoyStick_X = 0;   // Analog A0
const int JoyStick_Y = 1;   // Analog A1
const int JoyStick_Z = 13;  // Digital button

// Offsets to fix joystick drift (tune if needed)
const int x_fix = 21;
const int y_fix = 9;

// Deadzone threshold
const int DEADZONE = 100;


// Stepper settings

const int SmallStep = 4096; // for 28BYJ-48 type stepper
const int BigStep   = 200;  // for NEMA/standard stepper


// 4 Stepper Motors

// Big motors (Arm joints)
Stepper bm1(BigStep, 46, 47, 48, 49); // Shoulder / Arm A
Stepper bm2(BigStep, 50, 51, 52, 53); // Elbow   / Arm B

// Small motors
Stepper sm2(SmallStep, 23, 25, 27, 29); // Gripper/Bucket (Klo)
Stepper sm3(SmallStep, 30, 31, 32, 33); // Base Rotation

// Button toggle logic

bool gripperClosed = false;     // current state
int lastButtonState = HIGH;     // because we use pull-up
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Speed and step sizes

const int SPEED = 4;            // stepper speed
const int ROT_STEP = 20;        // rotation step amount
const int GRIP_STEP = 20;       // gripper step amount
const int ARM_STEP = 1;         // big motors step amount

void setup() {
  // Joystick pins
  pinMode(JoyStick_X, INPUT);
  pinMode(JoyStick_Y, INPUT);
  pinMode(JoyStick_Z, INPUT);

  // Enable internal pull-up for button (common wiring)
  digitalWrite(JoyStick_Z, HIGH);

  // Stepper speeds
  bm1.setSpeed(SPEED);
  bm2.setSpeed(SPEED);
  sm2.setSpeed(SPEED);
  sm3.setSpeed(SPEED);

  // Serial debug
  Serial.begin(9600);
  Serial.println("READY: 1 Joystick + 4 Steppers");
}

void loop() {
  // Read joystick analogs and center them around 0
  int x = analogRead(JoyStick_X) - 511 - x_fix;
  int y = analogRead(JoyStick_Y) - 511 - y_fix;

  // Read button
  int buttonState = digitalRead(JoyStick_Z);

  // =======================
  // Button debounce + toggle
  // =======================
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Detect a press (HIGH -> LOW)
    if (lastButtonState == HIGH && buttonState == LOW) {
      gripperClosed = !gripperClosed;

      if (gripperClosed) {
        Serial.println("Gripper: CLOSE");
        sm2.step(-GRIP_STEP);
      } else {
        Serial.println("Gripper: OPEN");
        sm2.step(GRIP_STEP);
      }
    }
  }

  lastButtonState = buttonState;

  // Base Rotation (X axis)
  
  if (x < -DEADZONE) {
    Serial.print(x); Serial.println("  X: Rotate Left");
    sm3.step(-ROT_STEP);
  } 
  else if (x > DEADZONE) {
    Serial.print(x); Serial.println("  X: Rotate Right");
    sm3.step(ROT_STEP);
  }

  
  // Arm Up/Down (Y axis)
  // Shoulder + Elbow together

  if (y < -DEADZONE) {
    Serial.print(y); Serial.println("  Y: Arm Down");
    bm1.step(-ARM_STEP);
    bm2.step(-ARM_STEP);
  } 
  else if (y > DEADZONE) {
    Serial.print(y); Serial.println("  Y: Arm Up");
    bm1.step(ARM_STEP);
    bm2.step(ARM_STEP);
  }

  // Small delay to reduce jitter
  delay(10);
}

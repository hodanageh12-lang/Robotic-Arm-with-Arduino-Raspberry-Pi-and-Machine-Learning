#include <Arduino.h>
#include <Stepper.h>

// Manual control ( Joystick) 
const int JoyStick_X = 0;   // Analog A0
const int JoyStick_Y = 1;   // Analog A1
const int JoyStick_Z = 13;  // Digital button

const int x_fix = 21;
const int y_fix = 9;
const int DEADZONE = 100;

// Stepper settings 
const int SmallStep = 4096;
const int BigStep   = 200;

// 4 Stepper Motors 
Stepper bm1(BigStep, 46, 47, 48, 49);      // Shoulder / Arm A
Stepper bm2(BigStep, 50, 51, 52, 53);      // Elbow   / Arm B
Stepper sm2(SmallStep, 23, 25, 27, 29);    // Gripper/Bucket
Stepper sm3(SmallStep, 30, 31, 32, 33);    // Base Rotation

//  Button toggle (manual gripper) 
bool gripperClosed = false;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Speeds & step sizes 
const int SPEED = 4;
const int ROT_STEP = 20;
const int GRIP_STEP = 20;
const int ARM_STEP = 1;

// Auto/Manual via Raspberry Pi 
bool autoMode = false;
String cmd = "";

// Read commands: "AUTO" or "MANUAL" from Raspberry Pi
void readSerialCmd() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      cmd.trim();
      if (cmd == "AUTO")   autoMode = true;
      if (cmd == "MANUAL") autoMode = false;
      cmd = "";
    } else if (c != '\r') {
      cmd += c;
    }
  }
}

// Simple automatic dig cycle (tune steps for your arm)
void digCycle() {
  // Down
  bm1.step(-120);
  bm2.step(-120);

  // Close bucket/gripper
  sm2.step(-200);

  // Up
  bm1.step(120);
  bm2.step(120);

  // Rotate to dump
  sm3.step(300);

  // Open to dump
  sm2.step(200);

  // Rotate back
  sm3.step(-300);

  autoMode = false; // back to manual after finishing
}

void setup() {
  pinMode(JoyStick_X, INPUT);
  pinMode(JoyStick_Y, INPUT);
  pinMode(JoyStick_Z, INPUT);
  digitalWrite(JoyStick_Z, HIGH); // pull-up for button

  bm1.setSpeed(SPEED);
  bm2.setSpeed(SPEED);
  sm2.setSpeed(SPEED);
  sm3.setSpeed(SPEED);

  Serial.begin(9600);
  Serial.println("READY: Manual joystick + AUTO/MANUAL from Raspberry Pi");
}

void loop() {
  // 1) Always listen to Raspberry Pi commands
  readSerialCmd();

  // 2) If AUTO mode is ON, run one dig cycle then return to manual
  if (autoMode) {
    Serial.println("AUTO: digCycle()");
    digCycle();
    return;
  }

  //  Manual mode (Joystick) 
  int x = analogRead(JoyStick_X) - 511 - x_fix;
  int y = analogRead(JoyStick_Y) - 511 - y_fix;
  int buttonState = digitalRead(JoyStick_Z);

  // Gripper toggle (manual)
  if (buttonState != lastButtonState) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (lastButtonState == HIGH && buttonState == LOW) {
      gripperClosed = !gripperClosed;
      if (gripperClosed) sm2.step(-GRIP_STEP);
      else               sm2.step(GRIP_STEP);
    }
  }
  lastButtonState = buttonState;

  // Base rotation (X)
  if (x < -DEADZONE) sm3.step(-ROT_STEP);
  else if (x > DEADZONE) sm3.step(ROT_STEP);

  // Arm up/down (Y) - shoulder + elbow together
  if (y < -DEADZONE) { bm1.step(-ARM_STEP); bm2.step(-ARM_STEP); }
  else if (y > DEADZONE) { bm1.step(ARM_STEP); bm2.step(ARM_STEP); }

  delay(10);
}

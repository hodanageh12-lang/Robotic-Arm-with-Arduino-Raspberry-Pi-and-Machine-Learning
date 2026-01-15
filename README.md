## Project Overview
This project implements a robotic arm system designed for the IAM3D R.O.V.E.R competition.
The system is capable of excavating and collecting resources using a combination of
manual joystick control and automatic operation based on computer vision.
System Architecture
## System Architecture
- Arduino Mega: Low-level control of stepper motors, joystick input, and execution of arm movements.
- Raspberry Pi: High-level decision making using a camera and computer vision to detect resources.
- Serial Communication: AUTO and MANUAL commands are sent from Raspberry Pi to Arduino.

## Control Modes
- Manual Mode: The robotic arm is controlled directly using a joystick for precise movements.
- Automatic Mode: When a resource is detected by the Raspberry Pi camera, the arm executes
  an automatic excavation cycle and then returns to manual control.

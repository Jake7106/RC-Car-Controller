# RC-Car-Controller
This repository contains the source code for my Arduino-based car controller.

This controller is designed for an Arduino Uno and allows an RC car to be controlled using an IR remote. The controller also integrates
several sensors for obstacle detection, allowing for autonomous driving modes.

Hardware Requirements:
- IR receiver
- IR remote
- TT Motor
- L9110 Motor Driver
- HC-SR04 Ultrasonic Module
- HW-201 IR module
- HW-511 Tracking Sensor

Note: This code was written and tested specifically for the hardware above. While exact component matches are not necessary,
using different hardware may require modifications to the code.

The code also requires the IR remote library '#include <IRremote.h>' to work, which can be directly installed through the Arduino IDE library manager.

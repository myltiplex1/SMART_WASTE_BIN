# SMART_WASTE_BIN

A compact IoT-enabled smart waste bin using an ESP8266 (INVENT ONE / NodeMCU style) that:
- Detects people approaching (IR proximity sensor).
- Measures bin fill-level with an ultrasonic sensor.
- Controls the lid with a servo motor.
- Shows fill status using RGB LEDs.
- Sends notifications via IFTTT and can integrate with Blynk for remote monitoring.

This repository contains:
- `circuits/` — block diagrams and schematic (PNG & PDF).
- `IR_test/IResp.ino` — IR sensor test.
- `led_test/blink_esp.ino` — LED test.
- `servo_test/servo_esp.ino` — Servo test.
- `smart3_IFTTT/smart3_IFTTT.ino` — Sketch that demonstrates Wi‑Fi, IFTTT notifications, servo, ultrasonic, IR, and LED logic.
- `smartx/smartx.ino` — Full integration (main script).

## Hardware / Parts
- ESP8266 development board (INVENT ONE, NodeMCU, or similar)
- HC-SR04 (or similar) ultrasonic distance sensor
- IR proximity sensor
- Small hobby servo (5V)
- RGB LEDs (or three separate LEDS: Red, Blue, Green) with appropriate resistors
- Power source (5V for servo and sensors — careful with ESP8266 USB power limits)
- Wires, breadboard, enclosure as required

## Circuit Diagrams
See `circuits/` folder for schematic and block diagrams

## Pinout (as used in `smart3_IFTTT.ino` and ' `smartx.ino')
The project uses Arduino-style D labels (match to your board). Update pins if your board differs.

- IR_SENSOR_PIN = D4
- ULTRASONIC_TRIG_PIN = D9
- ULTRASONIC_ECHO_PIN = D7
- SERVO_PIN = D8
- RED_LED_PIN = D3
- BLUE_LED_PIN = D2
- GREEN_LED_PIN = D1

Note: Some ESP8266 boards expose D0–D8 mapping, but others differ — confirm your board's pin labels. If your board doesn't have D9/D7 labels, change the constants to available GPIO pins (e.g., D5, D6, etc.) and update wiring accordingly.

## Software / Sketches
Required Arduino libraries (install via Arduino Library Manager or PlatformIO):
- ESP8266WiFi
- ESP8266HTTPClient
- Servo
- (Optional/recommended) WiFiManager — for captive-portal Wi‑Fi provisioning
- (Optional) Blynk — if you plan to use Blynk dashboard features

Primary sketches:
- `smartx/smartx.ino` — combines sensors, servo, LED logic, counting people, and IFTTT notifications.
- 'smart3_IFTTT/smart3_IFTTT.ino' - test IFTTT integrations
- `IR_test/IResp.ino` — test IR sensor behavior.
- `led_test/blink_esp.ino` — test LEDs.
- `servo_test/servo_esp.ino` — test servo motion.

## Configuration — credentials & keys
Open `smartx.ino` and set your credentials before uploading.

Replace placeholders with real values:

Example:
```cpp
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// Replace with your IFTTT Webhooks event name and key
const String IFTTT_EVENT_NAME = "bin_full";
const String IFTTT_KEY = "YOUR_IFTTT_KEY";
```

## Demo video

Watch the project demo on Vimeo: [Watch the smart_waste_bin demo](https://vimeo.com/1116561993?share=copy)

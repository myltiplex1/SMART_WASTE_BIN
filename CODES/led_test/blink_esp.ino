// Pin numbers for the LEDs
const int ledPin1 = D1; // GPIO pin 2
const int ledPin2 = D2; // GPIO pin 0
const int ledPin3 = D3; // GPIO pin 5

// Time in milliseconds for LED blink interval
const int blinkInterval = 500;

void setup() {
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  
  // Turn off the LEDs during setup
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
}

void loop() {
  // Blink LED 1
  digitalWrite(ledPin1, HIGH); // Turn LED 1 ON
  delay(blinkInterval);        // Wait for the specified interval
  digitalWrite(ledPin1, LOW);  // Turn LED 1 OFF
  delay(blinkInterval);        // Wait for the specified interval

  // Blink LED 2
  digitalWrite(ledPin2, HIGH); // Turn LED 2 ON
  delay(blinkInterval);        // Wait for the specified interval
  digitalWrite(ledPin2, LOW);  // Turn LED 2 OFF
  delay(blinkInterval);        // Wait for the specified interval

  // Blink LED 3
  digitalWrite(ledPin3, HIGH); // Turn LED 3 ON
  delay(blinkInterval);        // Wait for the specified interval
  digitalWrite(ledPin3, LOW);  // Turn LED 3 OFF
  delay(blinkInterval);        // Wait for the specified interval
}

//Function to restart the ESP8266
void restartESP() {
  ESP.restart();
}

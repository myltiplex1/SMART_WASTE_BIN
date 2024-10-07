#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const int IR_SENSOR_PIN = D4;
const int ULTRASONIC_TRIG_PIN = D9;
const int ULTRASONIC_ECHO_PIN = D7;
const int SERVO_PIN = D8;
const int RED_LED_PIN = D3;
const int BLUE_LED_PIN = D2;
const int GREEN_LED_PIN = D1;

Servo servoMotor;
long ultrasonicDuration;
int ultrasonicDistance;
int totalPeopleApproached = 0;
bool binFull = false;

const int NUM_TIME_READINGS = 5;
unsigned long timeReadings[NUM_TIME_READINGS];
int timeReadingsIndex = 0;
unsigned long timeTakenInSeconds = 0;
unsigned long timeEmptyStart = 0;
unsigned long timeFullStart = 0;

bool binEmptyMessagePrinted = false;
bool bin50PercentFullMessagePrinted = false;

// Replace with your Wi-Fi credentials
const char* WIFI_SSID = "Timflash";
const char* WIFI_PASSWORD = "multiplee";

// Replace with your IFTTT Webhooks event name and key
const String IFTTT_EVENT_NAME = "Waste_Bin_Level";
const String IFTTT_KEY = "cQFScQ7xcY6TnPCZ8PTQWE";

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  for (int i = 0; i < NUM_TIME_READINGS; i++) {
    timeReadings[i] = 0;
  }

  connectToWiFi();
}

void loop() {
  countPeople();
  ultrasonicDistance = measureUltrasonicDistance();

  if (ultrasonicDistance > 13) {
    // Bin is empty
    if (!binEmptyMessagePrinted) {
      Serial.println("Bin is empty.");
      binEmptyMessagePrinted = true;
      timeEmptyStart = millis();
    }
    binFull = false;
    bin50PercentFullMessagePrinted = false;
  }

  if (ultrasonicDistance < 5) {
    // Bin is full
    if (!binFull) {
      binFull = true;
      Serial.println("Bin is full.");
      Serial.print("Total people approached: ");
      Serial.println(totalPeopleApproached);
      timeFullStart = millis();
      servoMotor.write(150); // Close the lid if the bin is full
      delay(1000);           // Adjust the delay based on the servo speed and mechanics

      // Calculate the time taken in seconds for the bin to become full
      timeTakenInSeconds = (timeFullStart - timeEmptyStart) / 1000;

      // Send the IFTTT notification when the bin is full
      sendIFTTTNotification();
    }
  }

  controlLEDs();
}

int measureUltrasonicDistance() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  ultrasonicDuration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  // Speed of sound = 343 m/s or 0.0343 cm/μs
  // Distance = (Duration/2) * Speed of sound
  int distance = ultrasonicDuration * 0.0343 / 2;
  return distance;
}

void countPeople() {
  static unsigned long lastDetectionTime = 0;
  static bool personDetected = false;
  const unsigned long detectionInterval = 100; // Adjust this value based on your setup and sensor behavior

  if (digitalRead(IR_SENSOR_PIN) == HIGH) {
    // Person detected by the IR sensor
    if (!personDetected) {
      // Increment the total number of people if not already detected in the current interval
      totalPeopleApproached++;
      personDetected = true;
      lastDetectionTime = millis();
    }
  } else {
    // No person detected by the IR sensor
    if (personDetected && millis() - lastDetectionTime > detectionInterval) {
      // Reset person detection after the detection interval has passed
      personDetected = false;
    }
  }
}

void controlLEDs() {
  if (servoMotor.read() == 150) {
    // If the lid is closed, control the LEDs based on the ultrasonic
    if (ultrasonicDistance > 13) {
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(BLUE_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
    } else if (ultrasonicDistance > 5 && ultrasonicDistance <= 13) {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
    } else {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
    }
  } else {
    // If the lid is open, turn off all LEDs
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
}

void connectToWiFi() {
  Serial.println();
  Serial.print("Connecting to Wi-Fi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi");
}

void sendIFTTTNotification() {
  // Create a WiFiClient object
  WiFiClient wifiClient;

  // Create an HTTPClient object
  HTTPClient http;

  // Direct URL to IFTTT Webhooks
  String url = "http://maker.ifttt.com/trigger/" + IFTTT_EVENT_NAME + "/with/key/" + IFTTT_KEY;

  // Create the JSON data to be sent in the POST request
  String jsonData = "{ \"value1\":\"Bin_level = 100%[ WASTE BIN IS FULL!!!]\",\n";
  jsonData += "\"value2\":\"TotalPeopleApproached = " + String(totalPeopleApproached) + "\",\n";
  jsonData += "\"value3\":\"TimeTaken : " + String(timeTakenInSeconds) + " seconds\" }";

  // Begin the HTTP POST request
  http.begin(wifiClient, url); // Pass WiFiClient instance as an argument

  // Set the content type to JSON
  http.addHeader("Content-Type", "application/json");

  // Make the POST request with the JSON data
  int httpResponseCode = http.POST(jsonData);

  // Check the HTTP response code
  if (httpResponseCode > 0) {
    Serial.print("IFTTT notification sent. Response code: ");
    Serial.println(httpResponseCode);
    // Print the response from the server
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error sending IFTTT notification. Error code: ");
    Serial.println(httpResponseCode);
  }

  // End the HTTP request
  http.end();
}

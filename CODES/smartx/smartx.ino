#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>

#define BLYNK_AUTH_TOKEN "FJ2Sea_GwZfnFLvwGp6jYlCCxeObjGGH"

char auth[] = BLYNK_AUTH_TOKEN;
int totalPeopleApproached = 0;
bool binFull = false;
bool isLidOpen = false;
bool bin50PercentFullNotificationSent = false;

Servo servoMotor;
long ultrasonicDuration;
int ultrasonicDistance;

const int NUM_TIME_READINGS = 5;
unsigned long timeReadings[NUM_TIME_READINGS];
int timeReadingsIndex = 0;
unsigned long totalReadingTime = 0;
unsigned long timeFullStart = 0;
unsigned long timeFullEnd = 0;

const char* WIFI_SSID = "SmartBinAP";
const char* WIFI_PASSWORD = "password";

const int IR_SENSOR_PIN = D4;
const int ULTRASONIC_TRIG_PIN = D9;
const int ULTRASONIC_ECHO_PIN = D7;
const int SERVO_PIN = D8;
const int RED_LED_PIN = D3;
const int BLUE_LED_PIN = D1;
const int GREEN_LED_PIN = D2;

const int CONSECUTIVE_READINGS_THRESHOLD = 5;
int consecutiveReadings = 0;

// Replace with your IFTTT Webhooks event name and key
const String IFTTT_EVENT_NAME = "Waste_Bin_Level";
const String IFTTT_KEY = "cQFScQ7xcY6TnPCZ8PTQWE";

void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connected to Wi-Fi");
  String connectedSSID = WiFi.SSID();

  Blynk.begin(auth, connectedSSID.c_str(), WiFi.psk().c_str(), "blynk.cloud", 80);
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

  Blynk.virtualWrite(V4, 0);
}

void loop() {
  Blynk.run();
  countPeople();
  ultrasonicDistance = measureUltrasonicDistance();

  if (ultrasonicDistance < 5) {
    consecutiveReadings++;

    if (consecutiveReadings >= CONSECUTIVE_READINGS_THRESHOLD) {
      if (!binFull) {
        binFull = true;
        Serial.println("Bin is full.");
        Serial.print("Total people approached: ");
        Serial.println(totalPeopleApproached);
        //timeFullStart = millis();
        servoMotor.write(0);
        delay(500);

        Blynk.virtualWrite(V3, 1);
        Blynk.virtualWrite(V1, totalPeopleApproached);
        Blynk.virtualWrite(V0, 100);

        // Send the IFTTT notification for bin full
        sendIFTTTNotification("Waste Bin is full.", "TotalPeopleApproached = " + String(totalPeopleApproached));
      }
    }
  } else {
    consecutiveReadings = 0;

    Blynk.virtualWrite(V3, 0);

    if (digitalRead(IR_SENSOR_PIN) == HIGH) {
      if (servoMotor.read() == 180) {
        Serial.println("Lid is closed...");
        delay(5000);
        servoMotor.write(0);
        delay(500);

        if (!isLidOpen) {
          isLidOpen = true;
          Blynk.virtualWrite(V5, "Lid Closed");
        }
      } else if (ultrasonicDistance > 15) {
        if (!binFull) {
          Serial.println("Bin is empty.");
          timeFullStart = millis();
          Blynk.virtualWrite(V0, 0);
        }
      } else if (ultrasonicDistance > 5 && ultrasonicDistance <= 15) {
        if (!binFull && !bin50PercentFullNotificationSent) {
          Serial.println("Bin 50% full.");
          Blynk.virtualWrite(V0, 50);

          // Send the IFTTT notification for 50% full
          sendIFTTTNotification("Bin 50% full.", "TotalPeopleApproached = " + String(totalPeopleApproached));
          bin50PercentFullNotificationSent = true;
        }
      }
    } else {
      if (servoMotor.read() == 0) {
        Serial.println("Lid opens...");
        servoMotor.write(180);
        delay(500);

        if (isLidOpen) {
          isLidOpen = false;
          Blynk.virtualWrite(V5, "Lid Opens");
        }

        if (binFull) {
          timeFullEnd = millis();
          unsigned long timeFullDuration = timeFullEnd - timeFullStart;
          printTimeDuration(timeFullDuration);

          //Blynk.virtualWrite(V2, timeFullDuration / 1000);

          binFull = false;
          totalPeopleApproached = 0;
          bin50PercentFullNotificationSent = false;
        }
      }
    }
  }

  controlLEDs();
  String connectedSSID = WiFi.SSID();
  Blynk.virtualWrite(V6, "Wi-Fi connected to " + connectedSSID);
}

// Rest of the code remains unchanged...
int measureUltrasonicDistance() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  ultrasonicDuration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  int distance = ultrasonicDuration * 0.0343 / 2;
  return distance;
}

void countPeople() {
  static unsigned long lastDetectionTime = 0;
  static bool personDetected = false;
  const unsigned long detectionInterval = 100;

  if (digitalRead(IR_SENSOR_PIN) == HIGH) {
    if (!personDetected) {
      totalPeopleApproached++;
      personDetected = true;
      lastDetectionTime = millis();
    }
  } else {
    if (personDetected && millis() - lastDetectionTime > detectionInterval) {
      personDetected = false;
    }
  }
}
void printTimeDuration(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  String durationString = "";
  if (days > 0) {
    durationString += String(days) + " days ";
  }
  if (hours > 0) {
    durationString += String(hours % 24) + " hours ";
  }
  if (minutes > 0) {
    durationString += String(minutes % 60) + " minutes ";
  }
  durationString += String(seconds % 60) + " seconds";

  Blynk.virtualWrite(V2, durationString);  // Write to Blynk virtual pin
}

/*void printTimeDuration(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  Serial.print("Time taken: ");
  if (days > 0) {
    Serial.print(days);
    Serial.print(" days ");
  }
  if (hours > 0) {
    Serial.print(hours % 24);
    Serial.print(" hours ");
  }
  if (minutes > 0) {
    Serial.print(minutes % 60);
    Serial.print(" minutes ");
  }
  Serial.print(seconds % 60);
  Serial.println(" seconds");
}

String getTimeDurationString(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  String durationString = "";
  if (days > 0) {
    durationString += String(days) + " days ";
  }
  if (hours > 0) {
    durationString += String(hours % 24) + " hours ";
  }
  if (minutes > 0) {
    durationString += String(minutes % 60) + " minutes ";
  }
  durationString += String(seconds % 60) + " seconds";

  return durationString;
}*/

void controlLEDs() {
  // If the lid is closed, control the LEDs based on the ultrasonic distance
  if (servoMotor.read() == 0) {
    if (ultrasonicDistance > 15) {
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(BLUE_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
    } else if (ultrasonicDistance > 5 && ultrasonicDistance <= 15) {
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

BLYNK_WRITE(V4) {
  int servoPosition = param.asInt();
  servoMotor.write(servoPosition);
}

void sendIFTTTNotification(String message1, String message2) {
  WiFiClient wifiClient;
  HTTPClient http;

  String url = "http://maker.ifttt.com/trigger/" + IFTTT_EVENT_NAME + "/with/key/" + IFTTT_KEY;

  String jsonData = "{ \"value1\":\"" + message1 + "\",\n";
  jsonData += "\"value2\":\"" + message2 + "\" }";

  http.begin(wifiClient, url);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonData);

  if (httpResponseCode > 0) {
    Serial.print("IFTTT notification sent. Response code: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error sending IFTTT notification. Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

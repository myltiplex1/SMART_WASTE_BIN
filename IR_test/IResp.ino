const int IRSensor = D4; // Connect IR sensor module to Arduino pin 9

void setup() {
  Serial.begin(115200); // Init Serial at 115200 Baud
  Serial.println("Serial Working"); // Test to check if serial is working or not

  pinMode(IRSensor, INPUT); // IR Sensor pin INPUT
}

void loop() {
  int sensorStatus = digitalRead(IRSensor); // Read the IR sensor's digital output

  if (sensorStatus == HIGH) {
    Serial.println("Motion Ended!"); // Print "Motion Ended!" on the serial monitor window
  } else {
    Serial.println("Motion Detected!"); // Print "Motion Detected!" on the serial monitor window
  }
}
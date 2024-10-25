#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <SD.h>

// LCD Settings
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27 for 16x2 LCD

// RTC
RTC_DS3231 rtc;

// SD Card
const int chipSelect = 10;

// Inputs
const int potPin = A0;      // Potentiometer to simulate battery level
const int keyPin = 2;       // Button for key status
const int sideStandPin = 3; // Button for side stand status

// Outputs
const int leftIndicatorPin = 7;  // Left indicator LED
const int rightIndicatorPin = 8; // Right indicator LED
const int buzzerPin = 6;         // Buzzer for alerts

// Variables
bool keyStatus = false;
bool sideStandStatus = false;
String driveMode = "Normal";
int batteryLevel = 0;
int speed = 0;

void setup() {
  Serial.begin(9600);
  
  // LCD setup
  lcd.begin(16, 2);  // 16 columns and 2 rows
  lcd.backlight();
  lcd.print("Vehicle Cluster");

  // RTC setup
  if (!rtc.begin()) {
    lcd.setCursor(0, 1);
    lcd.print("RTC Fail");
    while (1);
  }

  // SD Card setup
  if (!SD.begin(chipSelect)) {
    lcd.setCursor(0, 1);
    lcd.print("SD Card Fail");
    while (1);
  }

  pinMode(potPin, INPUT);
  pinMode(keyPin, INPUT_PULLUP);
  pinMode(sideStandPin, INPUT_PULLUP);
  pinMode(leftIndicatorPin, OUTPUT);
  pinMode(rightIndicatorPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  delay(1000);
  lcd.clear();
}

void loop() {
  // Read Key Status
  keyStatus = !digitalRead(keyPin); // Inverted because of pull-up
  sideStandStatus = !digitalRead(sideStandPin);

  // Read Battery Level (Potentiometer)
  int potValue = analogRead(potPin);
  batteryLevel = map(potValue, 0, 1023, 0, 100);

  // Update Speed (Simulated as fixed for now)
  speed = random(0, 120); // Random speed between 0 and 120

  // Update Drive Mode
  if (speed == 0 && keyStatus) {
    driveMode = "Reverse";
  } else if (speed > 0 && keyStatus) {
    driveMode = "Forward";
  } else {
    driveMode = "Normal";
  }

  // Display on LCD
  displayLCD();

  // Log to SD Card every 5 seconds
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime > 5000) {
    logToSD();
    lastLogTime = millis();
  }

  // Alert for Low Battery and Reverse Mode
  if (batteryLevel < 20 || driveMode == "Reverse") {
    tone(buzzerPin, 1000); // Beep
  } else {
    noTone(buzzerPin); // Turn off buzzer
  }

  delay(500);
}

void displayLCD() {
  DateTime now = rtc.now();

  lcd.setCursor(0, 0);
  lcd.print("Speed: ");
  lcd.print(speed);
  lcd.print(" km/h");

  lcd.setCursor(0, 1);
  lcd.print("Battery: ");
  lcd.print(batteryLevel);
  lcd.print("%");

  lcd.setCursor(0, 2);
  lcd.print("Mode: ");
  lcd.print(driveMode);

  lcd.setCursor(0, 3);
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  lcd.print(now.second(), DEC);
}

void logToSD() {
  File dataFile = SD.open("cluster_log.txt", FILE_WRITE);
  
  if (dataFile) {
    DateTime now = rtc.now();
    dataFile.print("Time: ");
    dataFile.print(now.hour());
    dataFile.print(":");
    dataFile.print(now.minute());
    dataFile.print(":");
    dataFile.print(now.second());

    dataFile.print(" | Speed: ");
    dataFile.print(speed);
    dataFile.print(" km/h");

    dataFile.print(" | Battery: ");
    dataFile.print(batteryLevel);
    dataFile.print("%");

    dataFile.print(" | Drive Mode: ");
    dataFile.println(driveMode);

    dataFile.close();
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Log Fail");
  }
}
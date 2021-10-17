#include <Arduino.h>
#include <Servo.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Servo servo;
File dataFile;
Adafruit_BME280 bme;

const int R = 5;
const int G = 6;
const int B = 7;
const int O = 13;
const int buzzer = 12;
const int servopin = 9;
const int buttonPin = 8;
const int chipSelect = BUILTIN_SDCARD;
float alt;   //Storing current altitude
float lastAlt; //Storing previous altitude
int apogeeCount = 0; //Counter for checking if alt < lastAlt for 3 consecutive time
int x = 0; //Button state counter
bool buttonState = LOW;

void setup() {
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(O, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT);
  servo.attach(servopin);

//Do a beep and blink 3x
  for (int j = 0; j < 3; j++) {
    tone(buzzer, 3000);
    digitalWrite(R, HIGH);
    digitalWrite(G, HIGH);
    digitalWrite(B, HIGH);
    delay(500);
    noTone(buzzer);
    digitalWrite(R, LOW);
    digitalWrite(G, LOW);
    digitalWrite(B, LOW);
    delay(300);
  }

//Teensy communicates over 12 Mbit/sec USB speed
  Serial.begin(38400);

  if (!bme.begin(0x76) || !SD.begin(chipSelect)) {
    Serial.println("Error BME280 sensor/Card failed, or not present");
    tone(buzzer, 3000);
    digitalWrite(R, HIGH);
    delay(2000);
    noTone(buzzer);
    digitalWrite(R, LOW);
    while (1) delay(10);
  }

  //Servo locks
  servo.write(0);
  delay(5000);
  servo.write(90);

  //Initialize flight computer
  for (int j = 0; j < 3; j++) {
    tone(buzzer, 3000);
    digitalWrite(G, HIGH);
    delay(250);
    noTone(buzzer);
    digitalWrite(G, LOW);
    delay(250);
  }
  dataFile = SD.open("Alt.csv", FILE_WRITE);
  dataFile.print("Pressure HPA");
  dataFile.print(",");
  dataFile.println("Alt m");

  digitalWrite(G, HIGH);
  tone(buzzer, 3000);
  delay(3000);
  noTone(buzzer);
  digitalWrite(G, LOW);

}

void loop() {

  Serial.print(bme.readPressure() / 100.0F);
  Serial.print(",");
  Serial.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
  digitalWrite(O, HIGH);
  if (dataFile) {
    dataFile.print(bme.readPressure() / 100.0F);
    dataFile.print(",");
    dataFile.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
  }
  else {
    Serial.println("error opening datalog.txt");
    digitalWrite(R, HIGH);
    tone(buzzer, 3000);
    delay(500);
    noTone(buzzer);
    digitalWrite(R, LOW);
    delay(500);
  }

  alt = bme.readAltitude(SEALEVELPRESSURE_HPA);   //Get current alt
  if (alt - lastAlt < 0) {
    apogeeCount = apogeeCount + 1;
    alt = lastAlt; //Set current alt to become last alt
  }
  else if (apogeeCount >= 3) {
    servo.write(0);
    buttonState = digitalRead(buttonPin);
    tone(buzzer, 3000);
    digitalWrite(R, HIGH);
    digitalWrite(B, LOW);
    delay(200);
    noTone(buzzer);
    digitalWrite(B, HIGH);
    digitalWrite(R, LOW);
    delay(200);
    
    if (x < 1 && buttonState == 1)
    {
      x++;
      delay (200);
    }
    else if (x >= 1 && buttonState == 1)
    {
      x = 0;
      delay (200);
    }

    if (x == 1) {
      dataFile.close();
      noTone(buzzer);
      digitalWrite(R, LOW);
      digitalWrite(B, LOW);
      digitalWrite(G, LOW);
      while (1);
    }
  }
  else {
    apogeeCount = 0;
  }

}

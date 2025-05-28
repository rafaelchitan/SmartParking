#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include<MFRC522.h>
#include <SPI.h>
#include <Servo.h>


#define TRIG_PIN_SPOT 5
#define ECHO_PIN_SPOT 6

#define GREEN_PIN 2
#define RED_PIN 7

#define SS_PIN 8     // SDA pin on RFID module
#define RST_PIN 9     // RST pin on RFID module

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo barrier;

char users[20][10]; // store all users that come to the parking space
int numberOfUsers = 0;

int spaces = 10;
bool isTaken = false;

LiquidCrystal_I2C lcd(0x3F,  16, 2);

void setup() {
  Serial.begin(9600);

  barrier.attach(3);

  pinMode(TRIG_PIN_SPOT, OUTPUT);
  pinMode(ECHO_PIN_SPOT, INPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Scan an RFID card/tag");

  char buff[10] = "";
  for (int i = 0; i < 10; i++)
    strcpy(users[i], buff);

  lcd.init();
  lcd.backlight();
}

float getDistance(const int trigPin, const int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float duration = pulseIn(echoPin, HIGH);
  if (duration == 0) {
    return -1;
  } else {
    return duration * 0.0343 / 2;
  }
}

void lightSpot(float distance) {
  if (distance < 5) {
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);
  } else {
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, LOW);
  }
}

void storeUIDToCharArray(char uidStr[]) {
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    char hexByte[4];
    sprintf(hexByte, "%02X", mfrc522.uid.uidByte[i]);
    strcat(uidStr, hexByte);
  }

  Serial.print("UID as char array: ");
  Serial.println(uidStr);
}

void openBarrier() {
    barrier.write(90);
    delay(3000);
    barrier.write(0);
}

// true -> new user
bool logUser(char uid[]) {
  for (int i = 0; i < numberOfUsers; i++)
    if (!strcmp(users[i], uid)) {
      Serial.println("User leaves");
      for (int j = i + 1; j < numberOfUsers; j++)
        strcpy(users[i], users[j]);
      strcpy(users[numberOfUsers - 1], "");
      numberOfUsers--;
      return false;
    }

    strcpy(users[numberOfUsers++], uid);
    Serial.println("New user entered");
    return true;
}

void printMessage(bool isEntering) {
  lcd.clear();
  
  if (isEntering) {
    lcd.print("Welcome!");
  } else {
    lcd.print("Good Bye");
  }
  openBarrier();
}

void loop() {
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Parking");

  lcd.setCursor(0, 1);
  lcd.print("Spots: ");
  lcd.print(spaces - numberOfUsers);

  float distance = getDistance(TRIG_PIN_SPOT, ECHO_PIN_SPOT);
  lightSpot(distance);

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    char uid[10] = "";
    storeUIDToCharArray(uid);
    printMessage(logUser(uid));

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}

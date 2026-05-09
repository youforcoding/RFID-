#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define SS_PIN    10
#define RST_PIN   9
#define BUZZER    8
#define LED_VERT  6
#define LED_ROUGE 5
#define SERVO_PIN 3

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo monServo;

String lastUID = "";
unsigned long lastScan = 0;

void showHome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan carte");
}

void ouvrirBab() {
  monServo.write(180);
  delay(2000);
  monServo.write(0);
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_VERT, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);

  digitalWrite(LED_VERT, LOW);
  digitalWrite(LED_ROUGE, LOW);

  monServo.attach(SERVO_PIN);
  monServo.write(0);

  lcd.init();
  lcd.backlight();
  showHome();
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();

  if (uid == lastUID && millis() - lastScan < 2000) return;

  lastUID = uid;
  lastScan = millis();

  Serial.println(uid);

  unsigned long start = millis();
  String msg = "";

  while (millis() - start < 2000) {
    if (Serial.available()) {
      msg = Serial.readStringUntil('\n');
      msg.trim();
      break;
    }
  }

  lcd.clear();

  //  OK
  if (msg.startsWith("OK")) {

    digitalWrite(LED_VERT, HIGH);

    tone(BUZZER, 1000);
    delay(150);
    noTone(BUZZER);

    lcd.setCursor(0, 0);
    lcd.print("WELCOME");
    lcd.setCursor(0, 1);
    lcd.print(msg.substring(3));

    ouvrirBab();

    digitalWrite(LED_VERT, LOW);
  }

  //  DEJA MESCANYA
  else if (msg == "DEJA") {

    digitalWrite(LED_ROUGE, HIGH);

    tone(BUZZER, 600);
    delay(200);
    noTone(BUZZER);

    lcd.setCursor(0, 0);
    lcd.print("DEJA SCANNE");
    lcd.setCursor(0, 1);
    lcd.print("ACCESS DENIED");

    delay(2000);

    digitalWrite(LED_ROUGE, LOW);
  }

  //  card machi ma3rofa
  else if (msg == "UNKNOWN") {

    digitalWrite(LED_ROUGE, HIGH);

    tone(BUZZER, 400);
    delay(300);
    noTone(BUZZER);

    lcd.setCursor(0, 0);
    lcd.print("CARTE");
    lcd.setCursor(0, 1);
    lcd.print("INCONNUE");

    delay(2000);

    digitalWrite(LED_ROUGE, LOW);
  }

  showHome();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gate;

// ===== Broches utilisées =====
#define SERVO_PIN 6
#define TRIG_PIN 9
#define ECHO_PIN 10
#define IR_PIN 2

#define LED_VERTE A0
#define LED_ROUGE A1

// ===== Paramètres du parking =====
#define GATE_OPEN 90
#define GATE_CLOSED 0
const int maxCars = 5;

int carCount = 0;

// Variables pour éviter les déclenchements multiples
bool entryBusy = false;
bool exitBusy = false;
bool lastIRState = HIGH;

// ===== Mesure de distance avec HC-SR04 =====
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long d = pulseIn(ECHO_PIN, HIGH, 30000);

  // Retourne une grande valeur si aucune mesure
  return (d == 0) ? 999 : d * 0.034 / 2;
}

// ===== Mise à jour de l'écran LCD =====
void updateLCD() {

  lcd.setCursor(0,0);

  if (carCount >= maxCars)
    lcd.print("PARKING FULL   ");
  else
    lcd.print("Smart Parking  ");

  lcd.setCursor(0,1);
  lcd.print(String(carCount) + "/" + String(maxCars) + "      ");
}

// ===== Gestion des LEDs =====
void updateLEDs() {

  // LED rouge allumée lorsque le parking est complet
  digitalWrite(LED_ROUGE, (carCount >= maxCars));
}

// ===== Ouverture de la barrière =====
void openGate(int t) {

  gate.write(GATE_OPEN);

  digitalWrite(LED_VERTE, HIGH);

  delay(t);

  digitalWrite(LED_VERTE, LOW);
}

// ===== Fermeture de la barrière =====
void closeGate() {

  gate.write(GATE_CLOSED);

  delay(400);
}

void setup() {

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);

  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);

  gate.attach(SERVO_PIN);

  closeGate();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.print("Smart Parking");

  delay(1500);

  lcd.clear();

  updateLCD();
}

void loop() {

  long distance = getDistance();

  // ===== Détection d'une voiture à l'entrée =====
  if (!entryBusy && carCount < maxCars && distance < 4) {

    entryBusy = true;

    openGate(900);
    closeGate();

    carCount++;

    updateLEDs();
    updateLCD();

    entryBusy = false;
  }

  // ===== Détection d'une voiture à la sortie =====
  bool irNow = digitalRead(IR_PIN);

  if (!exitBusy && irNow == LOW && lastIRState == HIGH) {

    exitBusy = true;

    // Vérification pour éviter un compteur négatif
    if (carCount > 0) {

      openGate(1500);
      closeGate();

      carCount--;
    }

    updateLEDs();
    updateLCD();

    exitBusy = false;
  }

  lastIRState = irNow;

  delay(80);
}
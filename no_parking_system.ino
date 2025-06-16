#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HardwareSerial.h>

#define RST_PIN     22
#define SS_PIN      21
#define BUZZER_PIN  13

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial sim800(2);
#define GSM_RX 16
#define GSM_TX 17

String ownerNumber = "+91XXXXXXXXXX"; // Change to vehicle owner's number

void setup() {
  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" No Parking Sys ");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  sim800.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);
  delay(1000);
  sim800.println("AT");
  delay(500);
  sim800.println("AT+CMGF=1");
  delay(500);
  sim800.println("AT+CSCS=\"GSM\"");
  delay(500);

  lcd.setCursor(0, 1);
  lcd.print(" Monitoring...  ");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String uid = getUID();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Violation Alert!");
  lcd.setCursor(0, 1);
  lcd.print("UID:");
  lcd.print(uid);

  activateBuzzer(3000);

  String alertMessage = "ALERT: Your vehicle is parked in a No Parking zone. Move immediately.";
  sendSMS(ownerNumber, alertMessage);

  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" No Parking Sys ");
  lcd.setCursor(0, 1);
  lcd.print(" Monitoring...  ");
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void sendSMS(String number, String message) {
  sim800.print("AT+CMGS=\"");
  sim800.print(number);
  sim800.println("\"");
  delay(500);
  sim800.print(message);
  sim800.write(26);  // CTRL+Z
  delay(3000);
}

void activateBuzzer(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

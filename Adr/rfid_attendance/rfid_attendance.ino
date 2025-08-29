#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define RST_PIN  22  // ESP32 Reset Pin (Connect to GPIO 22)
#define SS_PIN   5 // ESP32 SS Pin (Connect to GPIO 5)

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;

// Wi-Fi Credentials
#define WIFI_SSID "NSU-WiFi-6"
#define WIFI_PASSWORD ""

const String sheet_url = "https://script.google.com/macros/s/AKfycbxA18T28X7FQTVNp8G8xWpf8BnJxOmrsVnR6Ze2s91v_pPVr7NRkMXDqznlmL0F3IOz/exec?name=";

byte readBlockData[18]; // Buffer for 16 bytes + 2 overhead

void setup() {
  Serial.begin(19200);
  delay(2000);
  // Wi-Fi Setup
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWi-Fi Connected!");
  // SPI and MFRC522 Initialization
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Ready to read RFID.");
}

void loop() {
  // Clear residual states
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // Wait for a new RFID card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return; // No card detected
  }
  memset(readBlockData, 0, sizeof(readBlockData));// Clear buffer and read data
  int blockNum = 2;
  if (ReadDataFromBlock(blockNum, readBlockData)) {
    Serial.print("Data Read: ");
    for (int i = 0; i < 16; i++) {
      Serial.write(readBlockData[i]);
    }
    Serial.println();
    SendDataToSheet(String((char*)readBlockData));
  }
  mfrc522.PICC_HaltA(); 
  mfrc522.PCD_StopCrypto1();
  delay(3000);
  Serial.println("Scan your Card...");
}

bool ReadDataFromBlock(int blockNum, byte *readBlockData) {
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; // Authenticate with default key (0xFF)
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK) {
    Serial.println("Authentication failed!");
    return false;
  }
  byte bufferLen = 18;
  if (mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen) != MFRC522::STATUS_OK) {
    Serial.println("Read failed!");
    return false;
  }
  Serial.println("Read successful!");
  return true;
}

void SendDataToSheet(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient https;
    String requestURL = sheet_url + data;
    Serial.println("Sending data to Google Sheets...");
    if (https.begin(requestURL)) {
      int httpCode = https.GET();
      if (httpCode > 0) {
        Serial.println("Data sent to Google Sheets.");
      } else {
        Serial.printf("Error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end(); 
    } else {
      Serial.println("Unable to connect to server.");
    }
  } else {
    Serial.println("Wi-Fi not connected.");
  }
}
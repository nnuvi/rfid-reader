#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// RC522 RFID setup
#define RST_PIN 22
#define SS_PIN 21
MFRC522 rfid(SS_PIN, RST_PIN);

// WiFi credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// NTP setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  Serial.begin(115200);
  SPI.begin();  // Initialize SPI bus
  rfid.PCD_Init();  // Initialize RC522
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Start NTP Client
  timeClient.begin();
  timeClient.setTimeOffset(19800);  // Adjust to your timezone in seconds (e.g., GMT+5:30 for India)
}

void loop() {
  // Update time
  timeClient.update();

  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // Read card UID
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  // Get current time and date
  String dateTime = timeClient.getFormattedTime();

  // Send data via Serial
  Serial.println("UID: " + uid + ", Time: " + dateTime);

  // Halt RFID reader
  rfid.PICC_HaltA();
}
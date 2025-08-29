#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RST_PIN = 22;  // GPIO22
constexpr uint8_t SS_PIN = 5;  // GPIO5

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;        

int blockNum = 2;  // Set the block to write data 

byte blockData [16] = {"Meherin"}; // Data to be written in Card

// Create array to read data from Block 
byte bufferLen = 18;
byte readBlockData[18];

MFRC522::StatusCode status;

void setup() {
  Serial.begin(19200); //Initialize serial communications with PC
  delay(2000);
  
  SPI.begin(); //Initialize SPI bus
  mfrc522.PCD_Init();
  Serial.println("Scan a MIFARE 1K Tag to write data...");
} 

void loop() {
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++){
    key.keyByte[i] = 0xFF;
  }
  // Look for new cards 
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial()){return;}

  Serial.println("Card Detected");
  Serial.println("Writing to Data Block...");
  WriteDataToBlock(blockNum, blockData);
  Serial.println("Reading from Data Block...");
  ReadDataFromBlock(blockNum, readBlockData); // Read data from the same block 

  Serial.print("Data in Block:");
  Serial.print(blockNum);
  Serial.print(" --> ");
  for (int j=0 ; j<16 ; j++){
    Serial.write(readBlockData[j]);
  }
  Serial.print("\n");
}

void WriteDataToBlock(int blockNum, byte blockData[]) {
  // Authenticating the desired data block for write access using Key A 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);  // Write data to the block 
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {Serial.println("Data was written into Block successfully");}
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
   Serial.print("Authentication failed for Read: ");
   Serial.println(mfrc522.GetStatusCodeName(status));
   return;
  }
  
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen); // Reading data from the Block 
  if (status != MFRC522::STATUS_OK){
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {Serial.println("Block was read successfully");}
}

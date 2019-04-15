
//ESP8266 & FIREBASE LIB
#include <ESP8266WiFi.h> //WIFI LIBRARY
#include <ESP8266HTTPClient.h> //WIFI LIBRARY
#include <FirebaseArduino.h> //FIREBASE LIBRARY


#include <SPI.h>
#include <MFRC522.h>

//Firebase and Wifi Credentials
#define FIREBASE_HOST "n-a-i-a.firebaseio.com"
#define FIREBASE_AUTH "5vk6j4HGTMjU5MvxDiJiXqicEbA0T5Zjl5BMFH9b"
#define WIFI_SSID "PLDTHOMEDSLitlog"
#define WIFI_PASSWORD "b3Lg1@n1"

#define RST_PIN         D2          // Configurable, see typical pin layout above
#define SS_1_PIN        D4         // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 2
#define SS_2_PIN        D3          // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 1
#define SS_3_PIN        D1

#define WIFI_LED D0
#define RFID_LED D8

#define NR_OF_READERS   3

byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN};

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.

/**
 * Initialize.
 */
void setup() {
  pinMode(WIFI_LED, OUTPUT); 
  pinMode(RFID_LED, OUTPUT); 
  
  Serial.begin(115200); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("");
  Serial.print("CONNECTING");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("WIFI CONNECTED! ");
  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  SPI.begin();        // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
}

/**
 * Main loop.
 */
void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
    digitalWrite(WIFI_LED, HIGH);
  } else {
    Serial.println("NOT CONNECTED");
    digitalWrite(WIFI_LED, LOW);
  }
  String data;
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards

    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F(": Card UID: "));
      
      data = dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));
      
      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      
      
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader
  if (data != NULL) {
    //Send to firebase.
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["RFID_ID"] = data;
      String test = Firebase.push("/data:", root);
      Firebase.set("/rfid:", root);
      
      if (Firebase.failed()) {
          Serial.println  ("FIrebase Failed");
          Serial.println(Firebase.error());
      }
      else {
        Serial.print("data sent: ");
        Serial.println(data);
      }
  }
  
  timeLoop(millis(), 200);
  digitalWrite(RFID_LED, LOW);
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
String dump_byte_array(byte *buffer, byte bufferSize) {
  String rfid;
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : ":");
    //Serial.print(buffer[i], HEX);
    rfid += (buffer[i] < 0x10 ? "0" : "") + String(buffer[i], HEX) + (i!=(bufferSize-1) ? ":" : "");
  }
  rfid.toUpperCase();
  digitalWrite(RFID_LED, HIGH);
  Serial.print(rfid);
  return rfid;  
}

void timeLoop (long int startMillis, long int interval){ // the delay function
    // this loops until 2 milliseconds has passed since the function began
    while(millis() - startMillis < interval){} 
}

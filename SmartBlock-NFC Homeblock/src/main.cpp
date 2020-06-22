//     this example will attempt to mimic an ISO14443A smart card
//     and retrieve some basic information from a PoS or terminal,
//     this can be used to establish a communication process.
#include "main.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

//PN532 (SPI)
Adafruit_PN532 nfc(PN532_SS);

void newNeighbor();
void reset();

void setup(void) {
  Serial.begin(115200);
  Serial.println("HomeBlock");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("start");
  delay(200);
}

void loop(void) {
  boolean success;
  char apdubuffer[255] = {};
  uint8_t apdulen = 0;
  uint8_t ppse[] = {0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
  nfc.AsTarget();
  success = nfc.getDataTarget(apdubuffer, &apdulen); //Read initial APDU
  if (apdulen>0){
    bool _newNeighbor = true;
    Serial.println(apdulen);
    for (uint8_t i = 0; i < apdulen; i++){
        if (apdubuffer[i] != NEW_NEIGHBOR_CHAR) { //Example new neighbor message {'?',',','?',',','?',','}
            _newNeighbor = false;
        }
        // Serial.print(apdubuffer[i]);
    }
    if (_newNeighbor) {
        Serial.println("NEW NEIGHBOR");
        newNeighbor();
        reset();
    }
    else {
        Serial.println("NEW BLOCK");
        for (uint8_t i = 0; i < apdulen; i++){
            Serial.print(apdubuffer[i]);
        }
    }
    Serial.println("");
  }
  delay(1000);
}

void newNeighbor() {
    nfc.begin();
    nfc.SAMConfig();
    nfc.begin();

    uint8_t message[MAX_MESSAGE_LEN] = {'1',',','0',',','0',','};
    boolean success = false;
    while(!success) {
        if (nfc.inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            nfc.inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
    }
    Serial.println("NEIGHBOR ADDED");
}

void reset() {
    nfc.begin();
    nfc.SAMConfig();
}
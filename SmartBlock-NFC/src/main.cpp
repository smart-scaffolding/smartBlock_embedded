#include "main.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

//PN532 (SPI)
Adafruit_PN532 nfc(PN532_SS);

void setup(void) {

    Serial.begin(115200);
    Serial.println("Hello!");

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

    // configure board to read RFID tags
    nfc.SAMConfig();

    Serial.println("Waiting for an ISO14443A Card ...");
}

uint8_t state = RX;


uint8_t rxMessage[MAX_MESSAGE_LEN];
uint8_t txMessage[MAX_MESSAGE_LEN];

void loop(void) {
    if (state == RX) {
        // Set the max number of retry attempts to read from a card
        // This prevents us from waiting forever for a card, which is
        // the default behaviour of the PN532.
        nfc.setPassiveActivationRetries(0xFF);

        boolean success;
        nfc.AsTarget();
        uint_t messageLength
        success = nfc.getDataTarget(rxMessage, &messageLength); //Read initial APDU
        
        boolean newBlock = true;

        if (apdulen>0){
            for (uint8_t i = 0; i < messageLength; i++){
               if (rxMessage[i] != 0xff) {
                   newBlock = false;
               }
            }

            if (newBlock) {
                Serial.println("New Block");
                //TODO: send new block its location
            }
            else {
                //printing
                for (uint8_t i = 0; i < messageLength; i++){
                    Serial.print(" 0x"); Serial.print(rxMessage[i], HEX);
                }
                Serial.println("");
                //TODO: transmit message to next blocks
            }
        }
    }
    else if (state == TX) {

    }
}

void transmit(uint_8 message[MAX_MESSAGE_LEN]) {
    uint_8 responseLength;
    while (!nfc.inDataExchange(message,sizeof(message),message,&responseLength){
        continue
    }
}
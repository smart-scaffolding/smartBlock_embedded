#include "main.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

//PN532 (SPI)
Adafruit_PN532 nfc(PN532_SS);

//Function Declatation
void transmit(uint8_t message[MAX_MESSAGE_LEN]);

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

    nfc.begin();

    Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
    uint8_t len = (MAX_MESSAGE_LEN - 3) / 3; //number of bytes in message divided by three dimensions (-3 for commas)

    char x[len]; 
    char y[len];
    char z[len];

    uint8_t message[MAX_MESSAGE_LEN];

    Serial.print("x: ");
    while (Serial.available() == 0){
        continue;
    }
    Serial.readBytesUntil('\n', x, len); //put input into buffer

    Serial.print("y: ");
    while (Serial.available() == 0){
        continue;
    }
    Serial.readBytesUntil('\n', y, len); //put input into buffer

    Serial.print("z: ");
    while (Serial.available() == 0){
        continue;
    }
    Serial.readBytesUntil('\n', z, len); //put input into buffer

    for (int i = 0; i < len; i++) {
        message[i] = uint8_t(x[i]);
    }
    message[len] = uint8_t(',');
    for (int i = 0; i < len; i++) {
        message[len + 1 + i] = uint8_t(y[i]);
    }
    message[(2*len) + 1] = uint8_t(',');
    for (int i = 0; i < len; i++) {
        message[(2*len) + 2 + i] = uint8_t(z[i]);
    }
    message[(3*len) + 2] = uint8_t(',');

    boolean success = false;

    while(!success) {
        if (nfc.inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            nfc.inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
    }

    Serial.println("DONE");

}

void transmit(uint8_t message[MAX_MESSAGE_LEN]) {
    uint8_t responseLength;
    while (!nfc.inDataExchange(message,sizeof(message),message,&responseLength)){
        continue;
    }
}
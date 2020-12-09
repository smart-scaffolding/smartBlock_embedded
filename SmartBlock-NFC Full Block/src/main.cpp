#include "main.h"
#include "colors.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

//PN532 (SPI)
Adafruit_PN532 X1(10);

Adafruit_PN532 NFCs[NUM_NFC] = {X1};

void setup(void) {
    Serial.begin(115200);
    Serial.println("COLOR CHANGER");

    for (int i=0; i<NUM_NFC; i++) {

        NFCs[i].begin();

        uint32_t versiondata = NFCs[i].getFirmwareVersion();
        if (! versiondata) {
            char print[34]; //buffer to hold message
            sprintf(print, "Didn't find PN53x board");
            Serial.print(print);
            //while (1); // halt
        }

        // Got ok data, print it out!
        Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
        Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
        Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

        // configure board to read RFID tags
        NFCs[i].SAMConfig();
        NFCs[i].begin();
        delay(1000);
    }
    Serial.println("Done Setup ...");
}
void loop(void) {
    //////// Ask surounding blocks for this block's position ////////

    char message[9] = {'1','8','0','0','0','0','0','0','0'};
    Serial.print("Message: ");
    Serial.println(message);
    
    for (int i = 0; i < NUM_NFC; i++){
        NFCs[i].begin();
        NFCs[i].SAMConfig();
        NFCs[i].begin();

        //Try to find neighboring block
        boolean success = false;
        while(!success) {
            if (NFCs[i].inListPassiveTarget()) {
                NFCs[i].inDataExchange(message,sizeof(message),message,sizeof(message));
                success = true;
            }
            delay(1000);
        }
    }
    
}
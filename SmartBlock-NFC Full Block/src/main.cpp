#include "main.h"
#include "colors.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <FastLED.h>

//PN532 (SPI)
Adafruit_PN532 X1(X1_SS);
Adafruit_PN532 Y0(Y0_SS);

Adafruit_PN532 NFCs[NUM_NFC] = {X1, Y0};

//Define LEDs
CRGB leds[NUM_LEDS];

void setBlockColor(uint8_t R,uint8_t G, uint8_t B);
void setFaceColor(uint8_t face, uint8_t R,uint8_t G, uint8_t B);

void setup(void) {
    Serial.begin(115200);
    Serial.println("HOME BLOCK");

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
    //Config LEDs
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);  // GRB ordering is typical

    Serial.println("Waiting for other blocks ...");
}

//Variables to holds this block's coordinates
uint8_t thisX;
uint8_t thisY;
uint8_t thisZ;

//Variables to holds this block's color
uint8_t thisR;
uint8_t thisG;
uint8_t thisB;

void loop(void) {
    setBlockColor(GREEN);
    //////// Ask surounding blocks for this block's position ////////

    boolean success;
    char apdubuffer[255] = {};
    uint8_t apdulen = 0;
    uint8_t ppse[] = {0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
    while (apdulen == 0) {
        for (int i=0; i<NUM_NFC; i++) {
            NFCs[i].AsTarget();
            success = NFCs[i].getDataTarget(apdubuffer, &apdulen); //Read initial APDU
            if (apdulen>0){
                Serial.print("success");
            }
            delay(1000);
        }
    }
}

    

//LED Functions
void setBlockColor(uint8_t R,uint8_t G, uint8_t B){
    for (uint8_t i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB(R,G,B);
    }
    FastLED.show();
    return;
}
void setFaceColor(uint8_t face, uint8_t R,uint8_t G, uint8_t B){
    uint8_t startLED = face*NUM_LEDS_PER_FACE;
    for (uint8_t i = startLED; i < startLED+NUM_LEDS_PER_FACE; i++){
        leds[i] = CRGB(R,G,B);
    }
    FastLED.show();
    return;
}
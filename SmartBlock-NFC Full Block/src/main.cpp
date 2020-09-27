#include "main.h"
#include "colors.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <FastLED.h>

//PN532 (SPI)
Adafruit_PN532 NFC(X1_SS);

//Define LEDs
CRGB leds[NUM_LEDS];

void setBlockColor(uint8_t R,uint8_t G, uint8_t B);
void setFaceColor(uint8_t face, uint8_t R,uint8_t G, uint8_t B);

void setup(void) {
    Serial.begin(115200);
    Serial.println("SMART BLOCK");

    NFC.begin();

    uint32_t versiondata = NFC.getFirmwareVersion();
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
    NFC.SAMConfig();
    NFC.begin();

    //Config LEDs
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);  // GRB ordering is typical

    Serial.println("Waiting to be added to structure ...");
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
    setBlockColor(ORANGE);
    //////// Ask surounding blocks for this block's position ////////
    sendAll();

    char message[MAX_MESSAGE_LEN];
    for (int i = 0; i < MAX_MESSAGE_LEN; i++) {
        message[i] = NEW_NEIGHBOR_CHAR;
    }

    Serial.print("Message: ");
    Serial.println(message);

    //Try to find neighboring block
    boolean success = false;
    int blinkCount = 0;
    int blinkThreshold = 1;
    while(!success) {
        blinkCount ++;
        if (blinkCount <= blinkThreshold) {
            setBlockColor(ORANGE);
        }
        else if (blinkCount <= blinkThreshold * 2) {
            setBlockColor(BLACK);
        }
        else {
            blinkCount = 0;
        }
        if (NFC.inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            NFC.inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
    }
    setBlockColor(ORANGE)
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
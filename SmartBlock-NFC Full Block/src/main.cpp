#include "main.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <FastLED.h>

//PN532 (SPI)
Adafruit_PN532 X0(X0_SS);
Adafruit_PN532 X1(X1_SS);
Adafruit_PN532 Y0(Y0_SS);
Adafruit_PN532 Y1(Y1_SS);
Adafruit_PN532 Z0(Z0_SS);
Adafruit_PN532 Z1(Z1_SS);

// Adafruit_PN532 NFCs[NUM_NFC] = {X0, X1, Y0, Y1, Z0, Z1};
Adafruit_PN532 NFCs[NUM_NFC] = {X0}; //For testing purposes only

//Define LEDs
CRGB leds[NUM_LEDS];

//Function Declatations
//NFC funuctions
void send(uint8_t i);
void sendAll();
void recieve(uint8_t i);
void recieveAll();
void newNeighbor(uint8_t i);
void newBlock(char message[MAX_MESSAGE_LEN]);
void checkNeighbor(uint8_t i);
//LED Functions
void setBlockColor(uint8_t R,uint8_t G, uint8_t B);
void setFaceColor(uint8_t face, uint8_t R,uint8_t G, uint8_t B);

void setup(void) {
    Serial.begin(9600);
    Serial.println("SMART BLOCK");

    for (uint8_t i=0; i<NUM_NFC; i++) {
        NFCs[i].begin();

        uint32_t versiondata = NFCs[i].getFirmwareVersion();
        if (! versiondata) {
            char print[34]; //buffer to hold message
            sprintf(print, "Didn't find PN53x board number %d\n", i);
            Serial.print(print);
            while (1); // halt
        }

        // Got ok data, print it out!
        Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
        Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
        Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

        // configure board to read RFID tags
        NFCs[i].SAMConfig();
        NFCs[i].begin();
    }

    //Config LEDs
    // FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);  // GRB ordering is typical
    setBlockColor(0, 180, 0);

    /*
    // Testing purposes only
    while(true){
        for (uint8_t i = 0; i < 6; i++){
            setBlockColor(0, 180, 0); //make all faces green
            setFaceColor(i, 180, 0, 0); //make one face red;
            if (i != 5){ setFaceColor(i+1, 0, 0, 180); }
            else { setFaceColor(0, 0, 0, 180); }
            delay(500);
        }
    }
    */

    Serial.println("Waiting to be added to structure ...");
}

uint8_t thisX;
uint8_t thisY;
uint8_t thisZ;

//TODONE:check neighbor for all 6 sides
// boolean haveNeighbor[NUM_NFC] = {false,false,false,false,false,false}; //To identify attached neighbor on any side of block (except inital mounting side)
boolean haveNeighbor[NUM_NFC] = {false}; //To identify attached neighbor on any side of block (except inital mounting side)


void loop(void) {
    //Ask surounding blocks for this blocks position
    sendAll();

    char message[MAX_MESSAGE_LEN];
    for (int i = 0; i < MAX_MESSAGE_LEN; i++) {
        message[i] = NEW_NEIGHBOR_CHAR;
    }

    boolean success = false;
    uint8_t faceWithNeighbor;
    while(!success) {
        for (uint8_t i=0; i<NUM_NFC; i++) {
            if (NFCs[i].inListPassiveTarget()) {
                uint8_t responseLength = sizeof(message);
                NFCs[i].inDataExchange(message,sizeof(message),message,&responseLength);
                success = true;
                faceWithNeighbor = i;
            }
            //TODONE pay attention to which face recieved communication
            //TODO identify if more than one face has communication at the same time
            //TODO add all sorounding blocks as neighbors
        }
    }
    Serial.println("ASKING FOR MY LOCATION");
    //Recieve this blocks position
    recieveAll();

    char x_buf[COORDINATE_LEN + 1]; 
    char y_buf[COORDINATE_LEN + 1];
    char z_buf[COORDINATE_LEN + 1];
    x_buf[COORDINATE_LEN] = '\n';
    y_buf[COORDINATE_LEN] = '\n';
    z_buf[COORDINATE_LEN] = '\n';

    success = false;
    char apdubuffer[255] = {};
    uint8_t apdulen = 0;
    uint8_t ppse[] = {0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
    delay(1000);
    while (apdulen == 0) {
        NFCs[faceWithNeighbor].AsTarget();
        success = NFCs[faceWithNeighbor].getDataTarget(apdubuffer, &apdulen); //Read initial APDU
        if (apdulen>0){
            // Serial.println(apdulen);
            for (uint8_t i = 0; i < apdulen; i++){
                char digit = apdubuffer[i];
                // Serial.print(digit);
                //Put recieved coordinate into char buffer
                if (i < COORDINATE_LEN) { x_buf[i] = digit; }
                else if (i > COORDINATE_LEN && i < (2* COORDINATE_LEN)+1) { y_buf[i - (COORDINATE_LEN + 1)] = digit; }
                else if (i > 2* COORDINATE_LEN + 1 && i < (3* COORDINATE_LEN)+2) { z_buf[i - (2* COORDINATE_LEN + 2)] = digit; }
            }
            //Asign this block's position (Convert char burrers to uint_8s)
            char thisLocation[MAX_MESSAGE_LEN];
            thisX = atoi(x_buf);
            thisY = atoi(y_buf);
            thisZ = atoi(z_buf);
            Serial.print("X: ");
            Serial.print(thisX);
            Serial.print(" Y: ");
            Serial.print(thisY);
            Serial.print(" Z: ");
            Serial.print(thisZ);
            Serial.println("");
            //TODO: Use sprintf above
        }
        delay(1000);
    }
    Serial.println("END RECIEVE");

    //Message recieving/passing mode(Pass message of new block to homeblock through the other blocks)
    recieveAll();
    uint8_t checkNeighborCount = 0;
    uint8_t checkAt = 10;
    while(true){
        //Check if neighbor is still there
        if (checkNeighborCount == checkAt){
            for (uint8_t i = 0; i < NUM_NFC; i++) {
                if (haveNeighbor[i]) {
                    Serial.println("Checking Neighbor");
                    checkNeighbor(i);
                }
            }
        }
        checkNeighborCount++;

        for (uint8_t i = 0; i < NUM_NFC; i++){
            boolean success;
            char _apdubuffer[255] = {};
            uint8_t _apdulen = 0;
            uint8_t ppse[] = {0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
            NFCs[i].AsTarget();
            success = NFCs[i].getDataTarget(_apdubuffer, &_apdulen); //Read initial APDU
            
            if (_apdulen>0){
                bool _newNeighbor = true;
                bool _check = true;
                // Serial.println(_apdulen);
                for (uint8_t j = 0; j < _apdulen; j++){
                    if (_apdubuffer[j] != NEW_NEIGHBOR_CHAR) { //Example new neighbor message {'?',',','?',',','?',','}
                        _newNeighbor = false;
                    }
                    if (_apdubuffer[j] != CHECK_NEIGHBOR_CHAR) {
                        _check = false;
                    }
                }
                //New Neighboring block
                if (_newNeighbor) {
                    Serial.println("NEW NEIGHBOR");
                    newNeighbor(i);
                }
                //Neighbor checking this block
                else if (_check) {
                    Serial.print("BEING CHECKED BY NEIGHBOR");
                    _check = _check; //Do nothing
                }
                //New (non-neighboring) block added somewhere else in structure
                else {
                    Serial.println("NEW BLOCK");
                    newBlock(_apdubuffer);
                    //Print new block coordinates
                    for (uint8_t j = 0; j < _apdulen; j++){
                        Serial.print(_apdubuffer[j]);
                    }
                }
                Serial.println("");
            }
        }
        delay(1000);

    } 
}

/*
Prepares NFC module to send data 
*/
void send(uint8_t i) {
    NFCs[i].begin();
    NFCs[i].SAMConfig();
    NFCs[i].begin();
    return;
}
/*
Prepares all of the NFC modules to send data 
*/
void sendAll() {
    for (uint8_t i=0; i<NUM_NFC; i++) {
        NFCs[i].begin();
        NFCs[i].SAMConfig();
        NFCs[i].begin();
    }
    return;
}
/*
Prepares NFC module to recieve data 
*/
void recieve(uint8_t i) {
    NFCs[i].begin();
    NFCs[i].SAMConfig();
    return;
}
/*
Prepares all the NFC modules to recieve data 
*/
void recieveAll() {
    for (uint8_t i=0; i<NUM_NFC; i++) {
        NFCs[i].begin();
        NFCs[i].SAMConfig();
    }
    return;
}
/*
Function called to send new neighbor their location in the structure
Currently only increments X direction, but if in 3D structure X, Y, or Z should be incremented based off the new neighbor's relative postiion
*/
void newNeighbor(uint8_t i) {
    send(i);

    haveNeighbor[i] = true;

    //Send New Neighbor it's coordinates
    uint8_t increase[3] = {0,0,0};

     
    //TODO: How to know if new neighbor is an increase or decrease
    

    char message[MAX_MESSAGE_LEN];
    sprintf(message, "%d,%d,%d,", thisX+increase[0], thisY+increase[1], thisZ+increase[2]);
    boolean success = false;
    while(!success) {
        if (NFCs[i].inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            NFCs[i].inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
    }
    Serial.println("NEIGHBOR ADDED");
    delay(1000);

    //Send message of new block towards homeblock
    //TODO: Send to all neighbors except new neighbor
    send(i);
    success = false;
    while(!success) {
        if (NFCs[i].inListPassiveTarget()) {
            uint8_t responseLength = MAX_MESSAGE_LEN;
            Serial.println(responseLength);
            NFCs[i].inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
            success = true;
        }
    }
    recieve(i);

    return;
}

void newBlock(char message[MAX_MESSAGE_LEN]){
     //Ask surounding blocks for this blocks position
    // nfc.begin();
    // nfc.SAMConfig();
    // nfc.begin();
    sendAll();

    //TODO: Uncomment and make for all block except one with new neighbor
    boolean success = false;
    // while(!success) {
    //     if (nfc.inListPassiveTarget()) {
    //         uint8_t responseLength = MAX_MESSAGE_LEN;
    //         Serial.println(responseLength);
    //         nfc.inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
    //         success = true;
    //     }
    // }
    recieveAll();
    return;
}

/*
* Verifies that neighbor is still there
* Does not wait for message response because sucess is only true if the neighboring block recieved the message 
*/
void checkNeighbor(uint8_t i) {
    send(i);

    char message[MAX_MESSAGE_LEN];
    for (int i = 0; i < MAX_MESSAGE_LEN; i++) {
        message[i] = CHECK_NEIGHBOR_CHAR;
    }

    int maxChecks = 10;
    int checks = 0;
    boolean success = false;
    while(!success && checks < maxChecks) {
        if (NFCs[i].inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            NFCs[i].inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
        checks++;
    }
    if (!success && checks == maxChecks) {
        Serial.println("NEIGHBOR MISSING");
    }
    recieve(i);
    return;
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
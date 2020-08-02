#include "main.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

/* PINS (Teensy3.6 and Arduino UNO/ProMini)
    * SCL  10
    * MOSI 11
    * MISO 12
    * SCK  13
*/
#define SCL  10
#define MOSI 11
#define MISO 12
#define SCK 13


//PN532 (SPI)
// Adafruit_PN532 nfc(PN532_SS);

//PN532 (Software SPI)
Adafruit_PN532 nfc(SCK, MISO, MOSI, SCL);

//Function Declatation
void send();
void recieve();
void newNeighbor();
void newBlock(char message[MAX_MESSAGE_LEN]);
void checkNeighbor();

void setup(void) {
    Serial.begin(115200);
    Serial.println("NOT HOMEBLOCK");

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

int thisX;
int thisY;
int thisZ;

boolean haveNeighbor[5] = {false,false,false,false,false}; //To identify attached neighbor on any side of block (except inital mounting side)

void loop(void) {
    //Ask surounding blocks for this blocks position
    // nfc.begin();
    // nfc.SAMConfig();
    // nfc.begin();

    send();

    char message[MAX_MESSAGE_LEN];
    for (int i = 0; i < MAX_MESSAGE_LEN; i++) {
        message[i] = NEW_NEIGHBOR_CHAR;
    }
    boolean success = false;
    while(!success) {
        if (nfc.inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            nfc.inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
    }
    Serial.println("ASKING FOR MY LOCATION");
    //Recieve this blocks position
    recieve();

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
        nfc.AsTarget();
        success = nfc.getDataTarget(apdubuffer, &apdulen); //Read initial APDU
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
        }
        delay(1000);
    }
    Serial.println("END RECIEVE");

    //New block message passing
    recieve();
    int checkNeighborCount = 0;
    int checkAt = 10;
    while(true){
        //Check if neighbor is still there
        if (checkNeighborCount == checkAt){
            for (int i = 0; i < 5; i++) {
                if (haveNeighbor[i]) {
                    Serial.println("Checking Neighbor");
                    checkNeighbor();
                }
            }
            checkNeighborCount = 0;
        }
        checkNeighborCount++;

        boolean success;
        char _apdubuffer[255] = {};
        uint8_t _apdulen = 0; //TODO: Put this back to 0
        uint8_t ppse[] = {0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
        nfc.AsTarget();
        success = nfc.getDataTarget(_apdubuffer, &_apdulen); //Read initial APDU
        //TODO: Remove two lines below
        // char _apdubuffer[6] = {'1',',','2',',','3',','};
        // uint8_t _apdulen = 6;
        if (_apdulen>0){
            bool _newNeighbor = true;
            bool _check = true;
            // Serial.println(_apdulen);
            for (uint8_t i = 0; i < _apdulen; i++){
                if (_apdubuffer[i] != NEW_NEIGHBOR_CHAR) { //Example new neighbor message {'?',',','?',',','?',','}
                    _newNeighbor = false;
                }
                if (_apdubuffer[i] != CHECK_NEIGHBOR_CHAR) {
                    _check = false;
                }
            }
            //New Neighboring block
            if (_newNeighbor) {
                Serial.println("NEW NEIGHBOR");
                newNeighbor();
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
                for (uint8_t i = 0; i < _apdulen; i++){
                    Serial.print(_apdubuffer[i]);
                }
            }
            Serial.println("");
        }
        delay(1000);

    } 
}
void send() {
    nfc.begin();
    nfc.SAMConfig();
    nfc.begin();
    return;
}
void recieve() {
    nfc.begin();
    nfc.SAMConfig();
    return;
}
/*
Function called to send nnew neighbor their location in the structure
Currently only increments X direction, but if in 3D structure X, Y, or Z should be incremented based off the new neighbor's relative postiion
*/
void newNeighbor() {
    // nfc.begin();
    // nfc.SAMConfig();
    // nfc.begin();
    send();

    haveNeighbor[0] = true;

    uint8_t xInc = 1;
    uint8_t yInc = 0;
    uint8_t zInc = 0;

    char message[MAX_MESSAGE_LEN];
    sprintf(message, "%d,%d,%d,", thisX+xInc, thisY+yInc, thisZ+zInc);
    boolean success = false;
    while(!success) {
        if (nfc.inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            nfc.inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
    }
    Serial.println("NEIGHBOR ADDED");
    delay(1000);

    //Send message of new block towards homeblock
    send();
    success = false;
    while(!success) {
        if (nfc.inListPassiveTarget()) {
            uint8_t responseLength = MAX_MESSAGE_LEN;
            Serial.println(responseLength);
            nfc.inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
            success = true;
        }
    }
    recieve();

    return;
}

void newBlock(char message[MAX_MESSAGE_LEN]){
     //Ask surounding blocks for this blocks position
    // nfc.begin();
    // nfc.SAMConfig();
    // nfc.begin();
    send();

    boolean success = false;
    while(!success) {
        if (nfc.inListPassiveTarget()) {
            uint8_t responseLength = MAX_MESSAGE_LEN;
            Serial.println(responseLength);
            nfc.inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
            success = true;
        }
    }
    recieve();
    return;
}

/*
* Verifies that neighbor is still there
* Does not wait for message response because sucess is only true if the neighboring block recieved the message 
*/
void checkNeighbor() {
    send();

    char message[MAX_MESSAGE_LEN];
    for (int i = 0; i < MAX_MESSAGE_LEN; i++) {
        message[i] = CHECK_NEIGHBOR_CHAR;
    }

    int maxChecks = 10;
    int checks = 0;
    boolean success = false;
    while(!success && checks < maxChecks) {
        if (nfc.inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            nfc.inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
        checks++;
    }
    if (!success && checks == maxChecks) {
        Serial.println("NEIGHBOR MISSING");
    }
    recieve();
    return;
}
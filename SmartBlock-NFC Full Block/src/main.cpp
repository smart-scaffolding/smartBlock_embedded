#include "main.h"
#include "colors.h"
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

Adafruit_PN532 NFCs[NUM_NFC] = {X0, X1, Y0, Y1, Z0, Z1}; // X0 is oposite X1 and so one, Z0 is the bottom (Male) face, Z1 is the top (Allen key) face
// Adafruit_PN532 NFCs[NUM_NFC] = {X0}; //For testing purposes only

//TODONE:check neighbor for all 6 sides
boolean hasNeighbor[NUM_NFC] = {false,false,false,false,false,false}; //To identify attached neighbor on any side of block (except inital mounting side)
// boolean hasNeighbor[NUM_NFC] = {false}; //For testing purposes only
int neighborCoord[NUM_NFC][3];

//Arrays to keep track of orientation
int8_t increaseAxis[NUM_NFC] = {0,0,1,1,2,2}; //Can be 0,1,or2 to represent X,Y,or Z
int8_t increaseSign[NUM_NFC] = {-1,1,-1,1,-1,1}; //Can be -1 or 1

//Define LEDs
CRGB leds[NUM_LEDS];

//Function Declatations
//NFC 

void send(uint8_t i);
void sendAll();
void recieve(uint8_t i);
void recieveAll();
void newNeighbor(uint8_t i);
void passMessage(char message[MAX_MESSAGE_LEN], uint8_t recievingFace);
boolean checkNeighbor(uint8_t i);
void colorMessage(uint8_t i);
void localize(uint8_t i);
void announceMove();
void neighborMoving(int faceWithNeighbor);
//LED Functions
void setBlockColor(uint8_t R,uint8_t G, uint8_t B);
void setFaceColor(uint8_t face, uint8_t R,uint8_t G, uint8_t B);

void setup(void) {
    Serial.begin(115200);
    Serial.println("SMART BLOCK");

    for (uint8_t i=0; i<NUM_NFC; i++) {
        NFCs[i].begin();

        uint32_t versiondata = NFCs[i].getFirmwareVersion();
        if (! versiondata) {
            char print[34]; //buffer to hold message
            sprintf(print, "Didn't find PN53x board number %d\n", i);
            Serial.print(print);
            // while (1); // halt
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
    setBlockColor(HOME_R,HOME_G,HOME_B); //make block green

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

//Variables to holds this block's coordinates
uint8_t thisX = 0;
uint8_t thisY = 0;
uint8_t thisZ = 0;

//Variables to holds this block's color
uint8_t thisR = HOME_R;
uint8_t thisG = HOME_G;
uint8_t thisB = HOME_B;

void loop(void) {
    recieveAll();
    uint8_t checkNeighborCount = 0;
    uint8_t checkAt = 10;

    bool beingMoved = false;
    while(!beingMoved){
        // recieveAll();
        //Check if neighbor is still there
        if (checkNeighborCount == checkAt){
            checkNeighborCount = 0;
            for (uint8_t i = 0; i < NUM_NFC; i++) {
                if (hasNeighbor[i]) {
                    Serial.println("Checking Neighbor");
                    if (!checkNeighbor(i)) {
                        setBlockColor(RED);

                        char missingMessage[MAX_MESSAGE_LEN];
                        sprintf(missingMessage, "%d,%d,%d,%c", neighborCoord[i][0], neighborCoord[i][1], neighborCoord[i][3], BLOCK_REMOVED_CHAR);//TODO: FINISH THIS
                        passMessage(missingMessage, i);
                        //TODONE: send message of missing neighbor
                    }
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
            
            bool _newNeighbor = true;
            bool _check = true;
            bool _color = true;
            bool _localize = true;
            bool _beingMoved = true;
            bool _neighborMove = true;

            if (_apdulen>0){
                // TODO: Check for messages that were already recieved
                Serial.print("Message: ");
                Serial.println(_apdubuffer);

                for (uint8_t j = 0; j < _apdulen; j++){
                    if (_apdubuffer[j] != NEW_NEIGHBOR_CHAR) { //Example new neighbor message {'?','?','?','?'}
                        _newNeighbor = false;
                    }
                    if (_apdubuffer[j] != CHECK_NEIGHBOR_CHAR) {
                        _check = false;
                    }
                    if (_apdubuffer[j] != CHANGE_COLOR_CHAR) {
                        _color = false;
                    }
                    if (_apdubuffer[j] != LOCALIZE_CHAR) {
                        _localize = false;
                    }
                    if (_apdubuffer[j] != THIS_BLOCK_BEING_MOVED_CHAR) {
                        _beingMoved = false;
                    }
                    if (_apdubuffer[j] != NEIGHBOR_BEING_MOVED_CHAR) {
                        _neighborMove = false;
                    }
                }
                //New Neighboring block
                if (_newNeighbor) {
                    Serial.println("NEW NEIGHBOR");
                    newNeighbor(i);
                }
                //Neighbor checking this block
                else if (_check) {
                    Serial.println("BEING CHECKED BY NEIGHBOR");
                    _check = _check; //Do nothing
                }
                //Robot changing color of this block
                else if (_color) {
                    Serial.println("BLOCK COLOR BEING CHANGED BY ROBOT");
                    colorMessage(i);
                }
                //Localize Robot
                else if (_localize) {
                    Serial.println("LOCALIZING ROBOT");
                    localize(i);
                }
                //This block is being moved by the robot
                else if (_beingMoved) {
                    announceMove(); //Tell neighbors that this block is being moved
                    beingMoved = true; //Break while loop condition and reset to asking for location
                    //TODO: add delay here if block reconnects to current neighbor
                }
                else if (_neighborMove) {
                    neighborMoving(i);
                }

                //if no identfying chars are sent, pass message to neighbors
                else {
                    Serial.println("NEW BLOCK");
                    passMessage(_apdubuffer, i);
                    //Print message
                    for (uint8_t j = 0; j < _apdulen; j++){
                        Serial.print(_apdubuffer[j]);
                    }
                }
                Serial.println("");
            }
            else {
                Serial.println("no message recieved");
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
Param i: index of face with new neighbor
*/
void newNeighbor(uint8_t i) {
    send(i);

    hasNeighbor[i] = true;

    //Send New Neighbor it's coordinates
    uint8_t increase[3] = {0,0,0}; 
    increase[increaseAxis[i]] = increaseSign[i]; //choose which axis will increase based on which face has neighbor

    int newCoord[3] = {thisX+increase[0], thisY+increase[1], thisZ+increase[2]}; //calulate new neighbor's coordinates

    // Add new neighboors coordinate to list of neighbor coordinates
    for (int j=0; j<3; j++) {
        neighborCoord[i][j] = newCoord[j];
    }
     
    char message[MAX_MESSAGE_LEN];
    sprintf(message, "%d,%d,%d,%c", newCoord[0], newCoord[1], newCoord[2], BLOCK_ADDED_CHAR);
    boolean success = false;
    while(!success) {
        if (NFCs[i].inListPassiveTarget()) {
            uint8_t responseLength = sizeof(message);
            NFCs[i].inDataExchange(message,sizeof(message),message,&responseLength);
            success = true;
        }
    }
    delay(1000);
    
    //TODONE: send orientation
    send(i);
    
    char orient_message[2];
    sprintf(orient_message, "%d%d", increaseAxis[i], -1*increaseSign[i]); //send message of axis and sign
    //Sign of neighboring face will be opposite because the faces are oreinted in oposite directions along the same axis 
    success = false;
    while(!success) {
        if (NFCs[i].inListPassiveTarget()) {
            uint8_t responseLength = sizeof(orient_message);
            NFCs[i].inDataExchange(orient_message,sizeof(orient_message),orient_message,&responseLength);
            success = true;
        }
    }

    //Send Block it's color
    
    char color_message[MAX_MESSAGE_LEN];
    sprintf(color_message, "%d,%d,%d,", thisR, thisG, thisB); //send message of axis and sign
    //Sign of neighboring face will be opposite because the faces are oreinted in oposite directions along the same axis 
    success = false;
    while(!success) {
        if (NFCs[i].inListPassiveTarget()) {
            uint8_t responseLength = sizeof(color_message);
            NFCs[i].inDataExchange(color_message,sizeof(color_message),color_message,&responseLength);
            success = true;
        }
    }
    Serial.println("NEIGHBOR ADDED");

    //Send message of new block towards homeblock
    passMessage(message, i); //send message of new block to all other faces

    recieveAll();
    return;
}

/*
* Passes message to neighboring blocks
*/
void passMessage(char message[MAX_MESSAGE_LEN], uint8_t recievingFace){
    sendAll();

    for (int i = 0; i < NUM_NFC; i++) {
        // Relay message if face is not the one that recieved the message and the face has a neighboring block
        if (i != recievingFace && hasNeighbor[i]){
            boolean success = false;
            while(!success) {
                if (NFCs[i].inListPassiveTarget()) {
                    uint8_t responseLength = MAX_MESSAGE_LEN;
                    Serial.println(responseLength);
                    NFCs[i].inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
                    success = true;
                }
            }
        }
    }
    
    recieveAll();
    return;
}

/*
* Verifies that neighbor is still there
* Does not wait for message response because sucess is only true if the neighboring block recieved the message 
* Returns true if neighbor is found
*/
boolean checkNeighbor(uint8_t i) {
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
    recieve(i);
    if (!success && checks == maxChecks) {
        Serial.println("NEIGHBOR MISSING");
        return false;
    }
    return true;
}

/*
* Recieve change color message
*/
void colorMessage(uint8_t i) {
    boolean success = false;
    char apdubuffer[255] = {};
    uint8_t apdulen = 0;
    uint8_t ppse[] = {0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
    
    char color_buf[COORDINATE_LEN + 1];
    uint8_t R;
    uint8_t G;
    uint8_t B;

    while (apdulen == 0) {
        NFCs[i].AsTarget();
        success = NFCs[i].getDataTarget(apdubuffer, &apdulen); //Read initial APDU
        if (apdulen>0){
            uint8_t j = 0;
            uint8_t k = 0;
            for (uint8_t i = 0; i < apdulen; i++){
                char digit = apdubuffer[i];
                //Put recieved color into char buffer
                if (digit != ',') {
                    color_buf[j] = digit;
                    j++;
                }
                else {
                    color_buf[j] = '\n'; //null terminator for atoi()
                    //assign value to appropriate coordinate
                    switch (k) {
                        case 0:
                            R = atoi(color_buf);
                        case 1:
                            G = atoi(color_buf);
                        default:
                            B = atoi(color_buf);
                    }
                    j=0; 
                    k++;
                }
                
            }
            // char buffer[MAX_MESSAGE_LEN + 12];
            // sprintf(buffer, "X: %d Y: %d Z: %d\n", R, G, B);
        }
    }

    setBlockColor(R, G, B);
}

/*
* Function to localize robot
*/
void localize(uint8_t i) {
    char message[MAX_MESSAGE_LEN];
    sprintf(message, "%d,%d,%d,", thisX, thisY, thisZ);

    sendAll();

    // Relay message if face is not the one that recieved the message and the face has a neighboring block
    boolean success = false;
    while(!success) {
        if (NFCs[i].inListPassiveTarget()) {
            uint8_t responseLength = MAX_MESSAGE_LEN;
            Serial.println(responseLength);
            NFCs[i].inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
            success = true;
        }
    }
    
    recieveAll();
    return;
}
/*
* Anounces to neighboring block that this block is being moved
*/
void announceMove() {
    sendAll();

    char message[MAX_MESSAGE_LEN];

    //Fill char with correct char
    for (int i=0; i < MAX_MESSAGE_LEN; i++) {
        message[i] = THIS_BLOCK_BEING_MOVED_CHAR;
    }
    for (int i=0; i < NUM_NFC; i++) {
        // Relay message if the face has a neighboring block
        if (hasNeighbor[i]){
            boolean success = false;
            while(!success) {
                if (NFCs[i].inListPassiveTarget()) {
                    uint8_t responseLength = MAX_MESSAGE_LEN;
                    Serial.println(responseLength);
                    NFCs[i].inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
                    success = true;
                }
            }
        }
    }
    recieveAll();
}

/*
* Reset state of face with neighbor that is being moved
*/
void neighborMoving(int faceWithNeighbor) {
    hasNeighbor[faceWithNeighbor] = false;
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
#include "smartblock.h"

//Constructor
SmartBlock::SmartBlock() { return; }
SmartBlock::SmartBlock(uint8_t ss[NUM_FACES]) {
    //Initialize all faces
    for (int i=0; i < NUM_FACES; i++) {
        face[i] = Face(ss[i], MAX_MESSAGE_LEN);
    }

    //Config LEDs
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);  // GRB ordering is typical
    setColor(INIT_COLOR); //Set color of block
}

//Put all blocks into send message mode
void SmartBlock::configSend() {
    for (int i=0; i < NUM_FACES; i++) {
        face[i].configSend();
    }
    return;
}
//Put all blocks into recieve message mode
void SmartBlock::configRecieve() {
    for (int i=0; i < NUM_FACES; i++) {
        face[i].configRecieve();
    }
    return;
}
//Set color of block
void SmartBlock::setColor(uint8_t R,uint8_t G, uint8_t B) {
    for (uint8_t i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB(R,G,B);
    }
    FastLED.show();
    return;
}
//Wait to be placed and get localization and color information from blocks
void SmartBlock::selfOrient() {
    //////// Ask surounding blocks for this block's position ////////
    configSend();

    char message[MAX_MESSAGE_LEN];
    for (int i = 0; i < MAX_MESSAGE_LEN; i++) {
        message[i] = NEW_NEIGHBOR_CHAR;
    }

    //Try to find neighboring block
    boolean success = false;
    uint8_t faceWithNeighbor;
    int blinkCount = 0;
    int blinkThreshold = 5000;
    while(!success) {
        for (uint8_t i=0; i< NUM_FACES; i++) {
            if (face[i].trySendMessage(message)) {
                success = true;
                faceWithNeighbor = i;
                break;
            }
            //TODONE pay attention to which face recieved communication
            //TODO identify if more than one face has communication at the same time
            //TODO add all sorounding blocks as neighbors
        }
        blinkCount ++;
        if (blinkCount == blinkThreshold) {
            setColor(ORANGE);
        }
        else if (blinkCount == blinkThreshold * 2) {
            setColor(BLACK);
            blinkCount = 0;
        }
    }

    // Recieve this blocks position from the indentified neighbor
    configRecieve();
    
    memset(message, 0, MAX_MESSAGE_LEN);
    face[faceWithNeighbor].recieveMessage(message); //recieve message from neihgboring block

    char coord_buf[COORDINATE_LEN + 1]; //Buffer to hold each coordinate string
    uint8_t j = 0;
    uint8_t k = 0;
    for (uint8_t i = 0; i < MAX_MESSAGE_LEN; i++){
        char digit = message[i];
        //Put recieved coordinate into char buffer
        if (digit != ',') {
            coord_buf[j] = digit;
            j++;
        }
        else {
            coord_buf[j] = '\n'; //null terminator for atoi()
            //assign value to appropriate coordinate
            switch (k) {
                case 0:
                    x = atoi(coord_buf);
                case 1:
                    y = atoi(coord_buf);
                case 2:
                    z = atoi(coord_buf);
                default:
                    digit = digit; //Do nothing
            }
            j=0; 
            k++;
        }
    }
    // char buffer[MAX_MESSAGE_LEN + 12];
    // sprintf(buffer, "X: %d Y: %d Z: %d\n", thisX, thisY, thisZ);
    // Serial.println(buffer);

    // Recieve this block's orientation from the indetified neighbor
    configRecieve();

    memset(message, 0, MAX_MESSAGE_LEN);
    face[faceWithNeighbor].recieveMessage(message); //recieve message from neihgboring block

    if (faceWithNeighbor < 4) { //orientation cannot be determined for faces added in Z direction
        //set orientation of face with neighbor
        face[faceWithNeighbor].increaseAxis = message[0]; // Can be 0,1,or 2 to represent X,Y,or Z
        face[faceWithNeighbor].increaseSign = message[1]; //-1 or 1 to represent axis direction
        //if neighboring face is in + direction the face with the neighbor will be in the - direction

        //calculate and store coordinate of neighbor
        face[faceWithNeighbor].hasNeighbor = true;

        uint8_t increase[3] = {0,0,0}; 
        increase[face[faceWithNeighbor].increaseAxis] = face[faceWithNeighbor].increaseSign; //choose which axis will increase based on which face has neighbor
        face[faceWithNeighbor].neighborCoordinate[0] = x+increase[0]; //calulate neighbor's coordinates
        face[faceWithNeighbor].neighborCoordinate[1] = y+increase[1];
        face[faceWithNeighbor].neighborCoordinate[2] = z+increase[2];

        //Set orientation of opposite face
        int8_t oppositeFaceIndex = -1;
        if (faceWithNeighbor % 2 == 0) {
            oppositeFaceIndex = 1;
        }
        face[faceWithNeighbor+oppositeFaceIndex].increaseAxis = message[0];
        face[faceWithNeighbor+oppositeFaceIndex].increaseSign = -1*message[1];
        
        //Set orintation of perpendicular faces
        uint8_t dim;
        if (message[0] == 0) { dim = 1; }
        else { dim = 0; }

        if (faceWithNeighbor <= 1) { //set for Y0,Y1
            face[2].increaseAxis = dim;
            face[3].increaseAxis = dim;
        }
        else if (faceWithNeighbor <= 3) { //set for X0,X1
            face[0].increaseAxis = dim;
            face[1].increaseAxis = dim;
        }
        //Set increase direction (-1 or 1) for oposite face

        if (message[0] == 0) { // if face with neighbor is on X axis
            // Determine Y0 and Y1 direction from X0 and X1 direction
            if (faceWithNeighbor <= 1) {
                face[2].increaseSign = face[0].increaseSign;
                face[3].increaseSign = face[1].increaseSign;
            }
            // Determine X0 and X1 direction from Y0 and Y1 direction
            else if (faceWithNeighbor <= 3) {
                face[0].increaseSign = face[3].increaseSign;
                face[1].increaseSign = face[2].increaseSign;
            }
        }
        else if (message[0] == 1) { // if face with neighbor is on Y axis
            // Determine X0 and X1 direction from X0 direction
            if (faceWithNeighbor <= 1) {
                face[2].increaseSign = face[1].increaseSign;
                face[3].increaseSign = face[0].increaseSign;
            }
            else if (faceWithNeighbor <= 3) {
                // Determine X0 and X1 direction from Y0 and Y1 direction
                face[0].increaseSign = face[2].increaseSign;
                face[1].increaseSign = face[3].increaseSign;
            }
        }
    }

    //Recieve this block's color based on the homeblock's color
    configRecieve();

    memset(message, 0, MAX_MESSAGE_LEN);
    face[faceWithNeighbor].recieveMessage(message); //recieve message from neihgboring block

    j = 0;
    k = 0;
    char color_buf[4]; //Four color digits plus '\n'

    for (uint8_t i = 0; i < MAX_MESSAGE_LEN; i++){
        char digit = message[i];
        //Put recieved digit into char buffer
        if (digit != ',') {
            color_buf[j] = digit;
            j++;
        }
        else {
            color_buf[j] = '\n'; //null terminator for atoi()
            //assign value to appropriate color variable
            switch (k) {
                case 0:
                    r = atoi(color_buf);
                case 1:
                    g = atoi(color_buf);
                case 2:
                    b = atoi(color_buf);
                default:
                    digit = digit; //Do nothing
            }
            j=0; 
            k++;
        }
    }
    setColor(r,g,b);
}
// /*
// Function called to send new neighbor their location in the structure
// Currently only increments X direction, but if in 3D structure X, Y, or Z should be incremented based off the new neighbor's relative postiion
// Param i: index of face with new neighbor
// */
// void SmartBlock::newNeighbor(uint8_t i) {
//     configSend();

//     face[i].hasNeighbor = true;

//     //Send New Neighbor it's coordinates
//     uint8_t increase[3] = {0,0,0}; 
//     increase[increaseAxis[i]] = increaseSign[i]; //choose which axis will increase based on which face has neighbor

//     int newCoord[3] = {thisX+increase[0], thisY+increase[1], thisZ+increase[2]}; //calulate new neighbor's coordinates

//     // Add new neighboors coordinate to list of neighbor coordinates
//     for (int j=0; j<3; j++) {
//         neighborCoord[i][j] = newCoord[j];
//     }
     
//     char message[MAX_MESSAGE_LEN];
//     sprintf(message, "%d,%d,%d,%c", newCoord[0], newCoord[1], newCoord[2], BLOCK_ADDED_CHAR);
//     boolean success = false;
//     while(!success) {
//         if (NFCs[i].inListPassiveTarget()) {
//             uint8_t responseLength = sizeof(message);
//             NFCs[i].inDataExchange(message,sizeof(message),message,&responseLength);
//             success = true;
//         }
//     }
//     delay(1000);
    
//     //TODONE: send orientation
//     send(i);
    
//     char orient_message[2];
//     sprintf(orient_message, "%d%d", increaseAxis[i], -1*increaseSign[i]); //send message of axis and sign
//     //Sign of neighboring face will be opposite because the faces are oreinted in oposite directions along the same axis 
//     success = false;
//     while(!success) {
//         if (NFCs[i].inListPassiveTarget()) {
//             uint8_t responseLength = sizeof(orient_message);
//             NFCs[i].inDataExchange(orient_message,sizeof(orient_message),orient_message,&responseLength);
//             success = true;
//         }
//     }

//     //Send Block it's color
    
//     char color_message[MAX_MESSAGE_LEN];
//     sprintf(color_message, "%d,%d,%d,", thisR, thisG, thisB); //send message of axis and sign
//     //Sign of neighboring face will be opposite because the faces are oreinted in oposite directions along the same axis 
//     success = false;
//     while(!success) {
//         if (NFCs[i].inListPassiveTarget()) {
//             uint8_t responseLength = sizeof(color_message);
//             NFCs[i].inDataExchange(color_message,sizeof(color_message),color_message,&responseLength);
//             success = true;
//         }
//     }
//     Serial.println("NEIGHBOR ADDED");

//     //Send message of new block towards homeblock
//     passMessage(message, i); //send message of new block to all other faces

//     recieveAll();
//     return;
// }

// /*
// * Passes message to neighboring blocks
// */
// void passMessage(char message[MAX_MESSAGE_LEN], uint8_t recievingFace){
//     sendAll();

//     for (int i = 0; i < NUM_NFC; i++) {
//         // Relay message if face is not the one that recieved the message and the face has a neighboring block
//         if (i != recievingFace && hasNeighbor[i]){
//             boolean success = false;
//             while(!success) {
//                 if (NFCs[i].inListPassiveTarget()) {
//                     uint8_t responseLength = MAX_MESSAGE_LEN;
//                     Serial.println(responseLength);
//                     NFCs[i].inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
//                     success = true;
//                 }
//             }
//         }
//     }
    
//     recieveAll();
//     return;
// }

// /*
// * Verifies that neighbor is still there
// * Does not wait for message response because sucess is only true if the neighboring block recieved the message 
// * Returns true if neighbor is found
// */
// boolean checkNeighbor(uint8_t i) {
//     send(i);

//     char message[MAX_MESSAGE_LEN];
//     for (int i = 0; i < MAX_MESSAGE_LEN; i++) {
//         message[i] = CHECK_NEIGHBOR_CHAR;
//     }

//     int maxChecks = 10;
//     int checks = 0;
//     boolean success = false;
//     while(!success && checks < maxChecks) {
//         if (NFCs[i].inListPassiveTarget()) {
//             uint8_t responseLength = sizeof(message);
//             NFCs[i].inDataExchange(message,sizeof(message),message,&responseLength);
//             success = true;
//         }
//         checks++;
//     }
//     recieve(i);
//     if (!success && checks == maxChecks) {
//         Serial.println("NEIGHBOR MISSING");
//         return false;
//     }
//     return true;
// }

// /*
// * Recieve change color message
// */
// void colorMessage(uint8_t i) {
//     boolean success = false;
//     char apdubuffer[255] = {};
//     uint8_t apdulen = 0;
//     uint8_t ppse[] = {0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
    
//     char color_buf[COORDINATE_LEN + 1];
//     uint8_t R;
//     uint8_t G;
//     uint8_t B;

//     while (apdulen == 0) {
//         NFCs[i].AsTarget();
//         success = NFCs[i].getDataTarget(apdubuffer, &apdulen); //Read initial APDU
//         if (apdulen>0){
//             uint8_t j = 0;
//             uint8_t k = 0;
//             for (uint8_t i = 0; i < apdulen; i++){
//                 char digit = apdubuffer[i];
//                 //Put recieved color into char buffer
//                 if (digit != ',') {
//                     color_buf[j] = digit;
//                     j++;
//                 }
//                 else {
//                     color_buf[j] = '\n'; //null terminator for atoi()
//                     //assign value to appropriate coordinate
//                     switch (k) {
//                         case 0:
//                             R = atoi(color_buf);
//                         case 1:
//                             G = atoi(color_buf);
//                         default:
//                             B = atoi(color_buf);
//                     }
//                     j=0; 
//                     k++;
//                 }
                
//             }
//             // char buffer[MAX_MESSAGE_LEN + 12];
//             // sprintf(buffer, "X: %d Y: %d Z: %d\n", R, G, B);
//         }
//     }

//     setBlockColor(R, G, B);
// }

// /*
// * Function to localize robot
// */
// void localize(uint8_t i) {
//     char message[MAX_MESSAGE_LEN];
//     sprintf(message, "%d,%d,%d,", thisX, thisY, thisZ);

//     sendAll();

//     // Relay message if face is not the one that recieved the message and the face has a neighboring block
//     boolean success = false;
//     while(!success) {
//         if (NFCs[i].inListPassiveTarget()) {
//             uint8_t responseLength = MAX_MESSAGE_LEN;
//             Serial.println(responseLength);
//             NFCs[i].inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
//             success = true;
//         }
//     }
    
//     recieveAll();
//     return;
// }
// /*
// * Anounces to neighboring block that this block is being moved
// */
// void announceMove() {
//     sendAll();

//     char message[MAX_MESSAGE_LEN];

//     //Fill char with correct char
//     for (int i=0; i < MAX_MESSAGE_LEN; i++) {
//         message[i] = THIS_BLOCK_BEING_MOVED_CHAR;
//     }
//     for (int i=0; i < NUM_NFC; i++) {
//         // Relay message if the face has a neighboring block
//         if (hasNeighbor[i]){
//             boolean success = false;
//             while(!success) {
//                 if (NFCs[i].inListPassiveTarget()) {
//                     uint8_t responseLength = MAX_MESSAGE_LEN;
//                     Serial.println(responseLength);
//                     NFCs[i].inDataExchange(message,MAX_MESSAGE_LEN,message,&responseLength);
//                     success = true;
//                 }
//             }
//         }
//     }
//     recieveAll();
// }

// /*
// * Reset state of face with neighbor that is being moved
// */
// void neighborMoving(int faceWithNeighbor) {
//     hasNeighbor[faceWithNeighbor] = false;
// }
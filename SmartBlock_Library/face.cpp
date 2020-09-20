#include "face.h"

//Consctructor
Face::Face() {  return; }
Face::Face(uint8_t _ss, uint8_t _maxMessageLen) {
    maxMessageLen = _maxMessageLen;
    nfc = Adafruit_PN532(_ss);
    
    //Initialize PN532 chip
    nfc.begin();

    //check to see if chip was succesfully iniatized
    // uint32_t versiondata = NFCs[i].getFirmwareVersion();
    //     if (! versiondata) {
    //         char print[34]; //buffer to hold message
    //         sprintf(print, "Didn't find PN53x board at pin %d\n", _ss);
    //         Serial.print(print);
    //     }

    nfc.SAMConfig();
    nfc.begin();

    return;
}
//Put PN532 into message sending mode
 void Face::configSend() {
    nfc.begin();
    nfc.SAMConfig();
    nfc.begin();
    return;
}
//Put PN532 into message recieving mode
void Face::configRecieve() {
    nfc.begin();
    nfc.SAMConfig();
    return;
}
//Try to send Message with PN532 WITHOUT WAITING FOR SUCCESS
bool Face::trySendMessage(char* message) {
    bool success = false;
    if (nfc.inListPassiveTarget()) {
        uint8_t responseLength = maxMessageLen;
        //Serial.println(responseLength);
        nfc.inDataExchange(message,maxMessageLen,message,&responseLength);
        success = true;
    }
    return success;
}
//Send Message with PN532 WAITS FOR SUCCESS
void Face::sendMessage(char* message) {
    bool success = false;
    while(!success) {
        if (nfc.inListPassiveTarget()) {
            uint8_t responseLength = maxMessageLen;
            //Serial.println(responseLength);
            nfc.inDataExchange(message,maxMessageLen,message,&responseLength);
            success = true;
        }
    }
    return;
}
void Face::recieveMessage(char* message) {
    bool success = false;
    char apdubuffer[255] = {};
    uint8_t apdulen = 0;
    uint8_t ppse[] = {0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5, 0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00};
    delay(1000);
    while (apdulen == 0) {
        nfc.AsTarget();
        success = nfc.getDataTarget(message, &apdulen); //Read initial APDU
        //TODO: verify that using message in above function works correctly
    return;
    }
}
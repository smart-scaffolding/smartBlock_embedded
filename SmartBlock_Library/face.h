#include <stdint.h>
#include <Adafruit_PN532.h>
#include "colors.h"

class Face {
    private:
        uint8_t maxMessageLen;
        Adafruit_PN532 nfc = NULL;
        // int neighborCoordinate[3];
        // int8_t increaseAxis;
        // int8_t increaseSign;

    protected:

    public:
        bool hasNeighbor = false;
        uint8_t increaseAxis;
        int8_t increaseSign;
        int neighborCoordinate[3];
        
        Face();
        Face(uint8_t _ss, uint8_t _maxMessageLen);

        //Functions to put face in send/recieve mode
        virtual void configSend();
        virtual void configRecieve();

        //Functions to send/recieve messages
        virtual void sendMessage(char* message);
        virtual bool trySendMessage(char* message);
        virtual void recieveMessage(char* message);
};

//Null Face for use when a face does not have a PN532 Board
class NullFace: public Face {
    void configSend();
    void configRecieve();

    //Functions to send/recieve messages
    void sendMessage(char* message);
    void recieveMessage(char* message);
};
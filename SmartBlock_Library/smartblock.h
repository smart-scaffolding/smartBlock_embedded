#include "face.h"
#include "messages.h"
#include <FastLED.h>

#define NUM_FACES 6
//LEDs
#define NUM_LEDS 24
#define NUM_LEDS_PER_FACE 4
#define LED_PIN 9

class SmartBlock {
private:
    //Block Coordinate within structure
    int x,y,z;
    //Blocks color
    uint8_t r,g,b;
    //Define LEDs
    CRGB leds[NUM_LEDS];

    Face face[NUM_FACES];

protected:

public:
    // SmartBlock(uint8_t x0_ss, uint8_t x1_ss, uint8_t y0_ss, uint8_t y1_ss, uint8_t z0_ss, uint8_t z1_ss);
    SmartBlock();
    SmartBlock(uint8_t ss[NUM_FACES]); //pass ss pins in order X0,X1,Y0,Y1,Z0,Z1

    //Functions to put all faces in send/recieve mode
    void configSend();
    void configRecieve();

    //Function to get localization data from neighbor
    void selfOrient();

    void passMessage(uint8_t i); //send message to all neighbors except the neighbor that sent the message

    void newNeighbor(uint8_t i);
    boolean checkNeighbor(uint8_t i);
    void colorMessage(uint8_t i);
    void localize(uint8_t i);
    void announceMove();
    void neighborMoving(uint8_t faceWithNeighbor);

    void setColor(uint8_t R,uint8_t G, uint8_t B);
    void updateLEDS();

};
#include "main.h"
#include "smartblock.h"

uint8_t pinArray[NUM_FACES] = {X0_SS, X1_SS, Y0_SS, Y1_SS, Z0_SS, Z1_SS};
SmartBlock smartblock(pinArray);

void setup() {

}

void loop() {
    smartblock.selfOrient();
}
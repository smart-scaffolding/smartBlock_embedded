#define NUM_NFC 6 //Number of PN532 chips
// #define NUM_NFC 1 //For testing purposes only


//Pins
#define X0_SS (10) //PN532 chip select
#define X1_SS (9) //PN532 chip select
#define Y0_SS (8) //PN532 chip select
#define Y1_SS (7) //PN532 chip select
#define Z0_SS (6) //PN532 chip select
#define Z1_SS (5) //PN532 chip select

//LEDs
#define NUM_LEDS 24
#define NUM_LEDS_PER_FACE 4
#define LED_PIN 4 

//Block States
#define RX 0
#define TX 1

//Messages
#define COORDINATE_LEN 1 //maximum digigts in one cordinate (i.e if = 1 x can be 0 to 9)
// #define MAX_MESSAGE_LEN ((COORDINATE_LEN + 1) * 3)  + 1//Coordinates plus comma x3 + sign (+,-,?)
#define MAX_MESSAGE_LEN 255

//These value chars are sent as messages to commicate block status
#define NEW_NEIGHBOR_CHAR '?'
#define CHECK_NEIGHBOR_CHAR '*'
#define CHANGE_COLOR_CHAR '$'
#define LOCALIZE_CHAR '@'
#define THIS_BLOCK_BEING_MOVED_CHAR '%'
#define NEIGHBOR_BEING_MOVED_CHAR '!'

//These chars are sent at the end of a coordinate message to indicate whether the block has been add, is missing, or has an unhealthy/unknown status
#define BLOCK_ADDED_CHAR '+'
#define BLOCK_REMOVED_CHAR '-'
#define BLOCK_UNHEALTHY_CHAR '?'
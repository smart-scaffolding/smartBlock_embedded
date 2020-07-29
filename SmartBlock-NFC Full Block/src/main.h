// #define NUM_NFC 6 //Number of PN532 chips
#define NUM_NFC 1

//Pins
#define X0_SS (10) //PN532 chip select
#define X1_SS (9) //PN532 chip select
#define Y0_SS (8) //PN532 chip select
#define Y1_SS (7) //PN532 chip select
#define Z0_SS (6) //PN532 chip select
#define Z1_SS (5) //PN532 chip select

//Block States
#define RX 0
#define TX 1

//Messages
#define COORDINATE_LEN 1 //maximum digigts in one cordinate (i.e if = 1 x can be 0 to 9)
#define MAX_MESSAGE_LEN ((COORDINATE_LEN + 1) * 3)

#define NEW_NEIGHBOR_CHAR '?'
#define CHECK_NEIGHBOR_CHAR '*'
// #define CHECK_RESPONSE_CHAR "$"
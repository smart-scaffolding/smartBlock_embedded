//Pins
#define PN532_SS (10) //PN532 chip select

//Block States
#define RX 0
#define TX 1

//Messages
#define COORDINATE_LEN 1 //maximum digigts in one cordinate (i.e if =1 x can be 0 to 9)
#define MAX_MESSAGE_LEN ((COORDINATE_LEN + 1) * 3)

#define NEW_NEIGHBOR_CHAR '?'
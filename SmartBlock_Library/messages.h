#define COORDINATE_LEN 1 //maximum digigts in one cordinate (i.e if = 1 x can be 0 to 9)
#define MAX_MESSAGE_LEN ((COORDINATE_LEN + 1) * 3)  + 1//Coordinates plus comma x3 + sign (+,-,?)

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
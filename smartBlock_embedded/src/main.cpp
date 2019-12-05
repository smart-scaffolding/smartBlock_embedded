#include <Arduino.h>

#define HIGH 1
#define DIRECTIONPIN 13

const int directionFaces = 5;
int directionPin[directionFaces];
int directionState;

//TODO: All the below variables have to be adpated with the EPPROM logic 
int x,y,z; 
const int dimesionsLength = 3;
const int DEFAULTCOOR[dimesionsLength] = {-1,-1,-1};
int savedLocation[dimesionsLength] = {-1,-1,-1};
int location[dimesionsLength];

bool sendToHomeBlock();

void setup() {
  
  //Check if the block has been already flashed with a coordinate value
  if(savedLocation != DEFAULTCOOR){
    for(int i = 0; i < dimesionsLength-1; i++){
      location[i] = savedLocation[i];
    }
  }

  Serial.begin(9600);
  pinMode(DIRECTIONPIN, OUTPUT);
  digitalWrite(DIRECTIONPIN,HIGH); //TODO: should this set all pins on the 5 faces high?

  char response;
  while(response != '!'){
    response = Serial.read();
  }

  //wait for loaction to be recieved
  while(Serial.available() == 0){}

  digitalWrite(DIRECTIONPIN, LOW);
  x = Serial.read();
  y = Serial.read();
  z = Serial.read();

  char maleDirection[2];
  //TODO: the following code has to be saved to EEPROM
  maleDirection[0] = Serial.read(); //Saves + or - for male face direction in any axis
  maleDirection[1] = Serial.read(); //Saves x,y, or z for male face direction axis

}

void loop() {

  //Check if a direction Pin has been set HIGH to start communication
  for(int i = 0; i < directionFaces; i++){
    if(directionPin[i] == HIGH){
      directionState = i;
    }else{ //No smart block has been placed on already placed smart block
      directionState = 6;
    }
  }


  switch(directionState){
    case 1:
        Serial.write("!");
        Serial.write(char(x+1));
        Serial.write(char(y));
        Serial.write(char(z));
        Serial.write(direction of male); //ex. the male is connected in -z (from its view), the female is connected in +z (from the other blocks view)

        sendToHomeBlock(); //Transmit the location to the home block through male faces

        if(Serial.available()>0){
          Serial.write(Serial.read()); //Might need message pin
        }
      break;

    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
    default: //No block has been placed (idle state)
      break;
  }
}

/*
* Send new connected smart block location (through male face) to other smart blocks unitl home block is reached
*/
bool sendToHomeBlock(){

}
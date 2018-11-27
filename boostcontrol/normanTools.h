#include "EEPROM.h"

//Function declarations
int readIntFromEEPROM(unsigned int);
void writeIntToEEPROM(unsigned int, int);

//


int readIntFromEEPROM(unsigned int address){ //reads an int from EEPROM memory, needs the address
  byte hiByte, loByte;              //which it uses to read highByte, then adds 1 for lowByte
  hiByte = EEPROM.read(address);
  loByte = EEPROM.read(address + 1);
  //Serial.print("hiByte: "); Serial.println(hiByte);
  //Serial.print("loByte: "); Serial.println(loByte);
  int intOutput = word(hiByte,loByte);
  //Serial.print("Output: "); Serial.println(intOutput);
  return intOutput;
}

void writeIntToEEPROM(unsigned int address, int value){
  //write an int to EEPROM memory, needs the address
  //which it uses to write highByte, then adds 1 for lowByte
  EEPROM.update(address, highByte(value));
  EEPROM.update(address + 1, lowByte(value));
}


bool burnCalibration(byte positionSelect, int boostRPM, int dutyCycle){
  /*if(positionSelect == 0){
    EEPROM.update(positionSelect,highByte(boostRPM));
    EEPROM.update(positionSelect+1,lowByte(boostRPM));
    EEPROM.update(positionSelect+32,highByte(dutyCycle));
    EEPROM.update(positionSelect+33,lowByte(dutyCycle));
  }
  else if ( (positionSelect & 0x01) == 0) {  //checks if number is even or odd
    EEPROM.update(positionSelect*2,highByte(boostRPM));
    EEPROM.update(positionSelect*2+1,lowByte(boostRPM));
    EEPROM.update(positionSelect+32,highByte(dutyCycle));
    EEPROM.update(positionSelect+33,lowByte(dutyCycle));
  }
  else{
    EEPROM.update(positionSelect*3-1,highByte(boostRPM));
    EEPROM.update(positionSelect*3-2,lowByte(boostRPM));
    EEPROM.update(positionSelect+33,highByte(dutyCycle));
    EEPROM.update(positionSelect+34,lowByte(dutyCycle));
  }*/
  EEPROM.update(positionSelect+positionSelect,highByte(boostRPM));
  EEPROM.update(positionSelect+positionSelect+1,lowByte(boostRPM));
  EEPROM.update(positionSelect+positionSelect+32,highByte(dutyCycle));
    EEPROM.update(positionSelect+positionSelect+33,lowByte(dutyCycle));
  EEPROM.update(250,true);// address 8 saved calibration status
  Serial.println("All calibration values are saved");
  return true; //to be used to set calibration flag
}

bool clearCalibration(){
  for (int i = 0; i < 41; i++){
    EEPROM.update(i,highByte(0));
  }
  Serial.println("Calibration Cleared");
  return false;
}


byte convertASCIItoByte(byte input){
  byte output = input - 48;
  return output;
}


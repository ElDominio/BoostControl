#include "EEPROM.h"

//Function declarations
int readIntFromEEPROM(unsigned int);
void writeIntToEEPROM(unsigned int, int);



bool burnCalibration(byte positionSelect, int boostRPM, int dutyCycle){

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

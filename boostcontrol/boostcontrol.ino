
#include "normanTools.h"


static byte boostPin = 11;
static byte RPMinputPin = 2;
static byte boostRead = A0;

byte baseDC = 50;
byte currentDC = 101;
unsigned int currentBoostRPM = 0;
byte trigCounter;
byte cylCount = 4;

//byte potPosition = 0;

//boost vars
byte MAPupdate = 0;
unsigned int boostInt = 0;
bool diagRequest = false;
bool posRepFlag = true;
unsigned int boostRPM[10];
unsigned int dutyCycle[10];
bool RPMcorrection = true;
bool MAPcorrection = true;
int currentCell = 0;

//unsigned long pastTime = 0;
unsigned long triggerTime = 4294967295;

//Working Variables
long RPM;
long RPMold;
unsigned long times;
unsigned long timeOld;
int rateOfChange = 0;
unsigned long timesOld = 0;



void setup() {
  TCCR2B = (TCCR2B & 0b11111000) | 0x07; //set dc to 30hz
  // put your setup code here, to run once:
  pinMode(boostPin, OUTPUT);
    pinMode(RPMinputPin, INPUT);
  digitalWrite(boostPin, LOW);
  Serial.begin(115200);
  //Attach RPM input interrupt
  attachInterrupt(digitalPinToInterrupt(2), triggerCounter, RISING);
  loadCalibration();
}

void loop() {
  // put your main code here, to run repeatedly:
  MAPupdate++;
  if (MAPupdate > 50){
    boostInt = analogRead(boostRead);
    currentDC = DCcorrections(baseDC);
    MAPupdate = 0;
    currentCell = map(RPM, boostRPM[0], boostRPM[9], 0, 9);
    currentCell = constrain(currentCell, 0, 9);
    if(currentCell > 10){
      currentCell = 9;
    }
    
    currentBoostRPM = boostRPM[currentCell];
    baseDC = dutyCycle[currentCell];
    
    currentDC = DCcorrections(baseDC);
    if ((micros()-timeOld) > 191096){
      RPM = 0;
    }
  }
/*if ((millis() - triggerTime) > 1000){
    attachInterrupt(digitalPinToInterrupt(3), triggerCounter, RISING);
  }*/
  if(boostInt > 945){
      while(boostInt > 700){
        analogWrite(boostPin, 70);
       boostInt = analogRead(boostRead);
       Serial.println("OVERBOOST!");
      }
    }
  if(boostInt < 1020){
    analogWrite(boostPin, currentDC);
  }
  if (diagRequest){
      SerialDiag();
    }
    if (trigCounter > 0){
    RPMcounter();
  }
  
    if (Serial.available()){
      SerialComms();
    }
}

void loadCalibration(){
  for (int i = 0; i < 10; i++){
    Serial.print("i = ");Serial.println(i);
    /*if ( (i & 0x01) == 0) {  //checks if number is even or odd
      boostRPM[i] = readIntFromEEPROM(i);
      Serial.print("boostRPM[");Serial.print(i);Serial.print("]");Serial.println(boostRPM[i]);
      dutyCycle[i] = readIntFromEEPROM(i+32);
    }
    else{
      boostRPM[i] = readIntFromEEPROM(i+1);
      Serial.print("2boostRPM[");Serial.print(i);Serial.print("]");Serial.println(boostRPM[i]);
      dutyCycle[i] = readIntFromEEPROM(i+33);
    }*/
    boostRPM[i] = word(EEPROM.read(i+i), EEPROM.read(i+i+1));
    Serial.print("boostRPM[");Serial.print(i);Serial.print("]");Serial.println(boostRPM[i]);
    dutyCycle[i] = word(EEPROM.read(i+i+32), EEPROM.read(i+i+33));
    Serial.print("dutyCycle[");Serial.print(i);Serial.print("]");Serial.println(dutyCycle[i]);
  }
  
}


void SerialComms(){
  char commandChar = Serial.read();
  if (commandChar == 'w'){ //write command
    /* Memory Mapping
 *  0 - currentCell
 *  1 - RPM hibyte
 *  2 - RPM lobyte
 *  3 - cutTime hi
 *  4 - cutTime lo
 *  Example Message:
 *  w0;205;50;
 */
    int i = 0;
    Serial.println("Write mode activated, please enter the cell position (followed by a semicolon ';'");
    Serial.println("then the set RPMs, followed by a semicolon,");
    Serial.println("and the the desired duty cycle, followed by a semicolon.");
    Serial.println("Ex. w14;3500;50; saves a 3500 RPM and 50 duty cycle to position 14");
    while (i < 4){
      byte serialByte;
      int serialInt;
      Serial.println("main while");
      //if (Serial.available()){
        while (i == 0){
          if (Serial.available()){
            serialByte = Serial.read();
            if( serialByte == 59){
              i++;
              break;
            }
            serialByte = serialByte - 48;
            serialByte = constrain(serialByte, 0, 10);
            currentCell = serialByte;
            Serial.print("i = "); Serial.print(i);Serial.print(" ");Serial.println(currentCell);
             /* String serialRead =Serial.readStringUntil(';');
              byte serialInt = Serial.read();
              if (serialInt > 10){ serialInt = 9;}
              byte serialByte = serialInt;
             // byte serialByte = Serial.read();
            currentCell = serialByte;
            Serial.print("i = "); Serial.print(i);Serial.print(" ");Serial.println(currentCell);
            i++;*/
          }
        }
        while(i == 1){
                //Serial.println("main while3");
         if (Serial.available() > 4){
         byte readRPM[4];
          for(int i = 0 ; i < 4; i++){
            readRPM[i] = Serial.read() - 48;
            //Serial.println(readRPM[i]);
          }
          i++;
          serialByte = Serial.read();
          currentBoostRPM = readRPM[0]*1000 + readRPM[1]*100 + readRPM[2]*10 +readRPM[3];
          currentBoostRPM = constrain(currentBoostRPM, 0, 8000);
           Serial.print("i = "); Serial.print(i);Serial.print(" ");Serial.println(currentBoostRPM);

         }
        }
        while ( i == 2 ){
         if (Serial.available() > 2){
         byte readDC[2];
          for(int i = 0 ; i < 2; i++){
            readDC[i] = Serial.read() - 48;
            //Serial.println(readRPM[i]);
          }
          i++;
          //serialByte = Serial.read();
          currentDC = readDC[0]*10 + readDC[1];
          currentDC = constrain(currentDC, 0, 8000);
            currentDC = constrain(currentDC, 0, 100);
            currentDC = map(currentDC, 0, 100, 0, 255);
            Serial.print("i = "); Serial.print(i);Serial.print(" ");Serial.print(currentDC);
           Serial.print(" or in duty cycle: ");Serial.print(map(currentDC, 0, 255, 0, 100) + 1);Serial.print("%");
            Serial.println("");
         
          }
        }// end elseif
      //}// end main serial available
      if ( i == 3){
          Serial.print("i = "); Serial.print(i);Serial.print(" ");
          burnCalibration(currentCell, currentBoostRPM, currentDC);
          loadCalibration();
          i++; // i == 4
      }
   }// end while
  } // End write command
  
  else if (commandChar == 'r'){
    diagRequest = !diagRequest; // activate diag


    Serial.print("diagRequest = "); Serial.println(diagRequest);
   /* if ( (currentCell & 0x01) == 0) { 
      byte high = EEPROM.read(currentCell);
      byte low = EEPROM.read(currentCell+1);
     // Serial.print("cutRPM: ");Serial.println(word(high,low));
      high = EEPROM.read(currentCell + 32);
      low = EEPROM.read(currentCell + 33);
      //Serial.print("timeToCut: ");Serial.println(word(high,low));

     }
    else{
      byte high = EEPROM.read(currentCell+16);
      byte low = EEPROM.read(currentCell + 17);
      //Serial.print("cutRPM: ");Serial.println(word(high,low));
      high = EEPROM.read(currentCell + 48);
      low = EEPROM.read(currentCell + 49);
     // Serial.print("timeToCut: ");Serial.println(word(high,low));
    }*/
  }
  else if (commandChar == 'l'){
    loadCalibration();
  }

  else if (commandChar == 'h'){
    for(int i = 0; i < 24; i++){
       Serial.print("EEPROM[");Serial.print(i);Serial.print("]");Serial.println(EEPROM.read(i));
     /* if ( (i & 0x01) == 0) { 
        Serial.print("EEPROM["+ String(i) + "]");Serial.println(word(EEPROM.read(i), EEPROM.read(i+1)));
        if( i < 11){
          Serial.print("boostRPM["+ String(i) + "]");Serial.println(boostRPM[i]);
          Serial.print("dutyCycle["+ String(i) + "]");Serial.println(dutyCycle[i]);
        }
      }
      else{
        Serial.print("EEPROM["+ String(i) + "]");Serial.println(word(EEPROM.read(i+1), EEPROM.read(i+2)));
        if( i < 11){
          Serial.print("boostRPM["+ String(i) + "]");Serial.println(boostRPM[i]);
          Serial.print("dutyCycle["+ String(i) + "]");Serial.println(dutyCycle[i]);
        }
      }*/
    }
    for(int i = 0; i < 10; i++){
        Serial.print("word["+ String(i) + "]");Serial.println(word(EEPROM.read(i+i), EEPROM.read(i+i+1)));
    }
  }
  
  else if (commandChar == 'c'){
    byte i = 0;
    Serial.println("Enter 'd' for distributor, 'w' for wasted spark, or 'c' for Coil On Plug");
    while (i < 1){
      if (Serial.available()){
        char serialRead = Serial.read();
        if (serialRead == 'd'){
          cylCount = 1;
          EEPROM.update(70,cylCount);
          i++;
        }
        else if (serialRead == 'w'){
          cylCount = 2;
          EEPROM.update(70,cylCount);
          i++;
        }
        else if (serialRead == 'c'){
          cylCount = 4;
          EEPROM.update(70,cylCount);
          i++;
        }
      } // end serial avail
    }// end while
    Serial.print("Spark mode set! Current mode is: ");Serial.println(cylCount);
  }// end C case*/
}


byte DCcorrections(byte duty){
  
  if(RPMcorrection){
    duty = map(RPM, boostRPM[currentCell-1], boostRPM[currentCell], dutyCycle[currentCell-1], dutyCycle[currentCell]);
    duty = constrain(duty, 0, dutyCycle[7]);
    /*
    if ((RPM > 7000) && (RPM < 7699)){
      duty = duty + 1;
    }
    else if ((RPM > 7700) && (RPM < 8999)){
      duty = duty + 2;
    }*/
    if (RPM > boostRPM[9]){
      duty = dutyCycle[9];
    }
   
  if(MAPcorrection){
    if  (boostInt < 260){
      duty = 200;
    }
  }
  if (RPM < 2500){
     duty = 0;
    }
  }
   if (rateOfChange < -500){
    duty = 10;
  }
  return duty;
}

void triggerCounter(){
 // detachInterrupt(digitalPinToInterrupt(3));
  triggerTime = millis();
  trigCounter++;
  //Serial.println("TRIGGER");
}

void RPMcounter(){
 // Serial.print("micros "); Serial.print(micros());Serial.print("timeOld ");Serial.println(timeOld);
  timesOld = times;
  times = micros()-timeOld;        //finds the time 
  //Serial.print("Times: "); Serial.println(times);
  RPM = (30108000/times);         //calculates rpm
  RPM = RPM*(cylCount);
  // Serial.println(RPM);
 /* if ((RPM - RPMold) > 10000){
    RPM = RPMold;
  }*/
  timeOld = micros();
  if ((RPM > 0) && (RPM < 12000)){
    RPMold = RPM;
  }
  //Serial.println(RPM);
  trigCounter = 0;
  rateOfChange = timesOld - times;
}
/*
void RPMcounter(){
  
  if ((RPM > 0) && (RPM < 7000)){
    RPMold = RPM;
  }
  timesOld = times;
  times = millis()-timeOld;        //finds the time 
  RPM = (30108/times);         //calculates rpm
  RPM = RPM*cylCount;
  if ((RPM - RPMold) > 10000){
    RPM = RPMold;
  }
  timeOld = millis();
  //Serial.println(" INTERRUPT!");
  trigCounter = 0;
  rateOfChange = timesOld - times;
//  Serial.print(timesOld);Serial.print(" - ");Serial.print(times);Serial.print(" = ");Serial.println((rateOfChange));
  
}*/


void SerialDiag(){
  
  if (((millis() % 250) == 0) && posRepFlag){
    if (posRepFlag){
      Serial.print("RPM: "); Serial.println(RPM);
      Serial.print("MAP: ");Serial.println(boostInt);
      Serial.print("Current Duty Cycle: ");Serial.println(currentDC);
      Serial.print("CurrentCell: ");Serial.println(currentCell);
      Serial.print("Current Boost RPM: "); Serial.println(currentBoostRPM);
      Serial.print("test result = "); Serial.println(rateOfChange);
      Serial.println("");
    }
    posRepFlag = false;
  }
  else if(((millis() - 1) % 250) == 0){
    posRepFlag = true;
  }
  //  positionReport = millis();
}

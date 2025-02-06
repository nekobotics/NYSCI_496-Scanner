#include <Keyboard.h>
#include <SD.h>
#include <SPI.h>
#include "Display.h"

#define NumCardTypes 14
const int Library = 75;
int LibrarySize;

#define ClrButton 3
#define RegButton 4
#define PMButton 2
#define PMLight 31

const int chipSelect = BUILTIN_SDCARD;
File cardBackups;

char KnownCard[Library][13];
char Card[12];
const String Disposables[14] = {"aluminum can", "banana peel", "blender", "chicken bones", "envelope", "glass bottle", "soap bottle", "milk carton", "paper napkins", "soup carton", "paper tubes", "plastic bottle", "laptop computer", "smart phone"};
const String ID = "abcdefghijklmn";

//-----------------------------------------------------------------------------------------------------------------------------------


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial8.begin(9600);

  setupDisplay();

  pinMode(ClrButton,INPUT_PULLUP);
  pinMode(RegButton,INPUT_PULLUP);
  pinMode(PMButton,INPUT_PULLUP);
  pinMode(PMLight,OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      // No SD card, so don't do anything more - stay stuck here
    }
  }
  Serial.println("card initialized.");

  Pull();
}

void Push(){
  SD.remove("cardLog.txt");
  cardBackups = SD.open("cardLog.txt",FILE_WRITE);

  for(int x = 0; x < Library; x++){
    if(strlen(KnownCard[x]) == 0){
      cardBackups.print("-");
      cardBackups.close();
      return;
    }
    else{
      cardBackups.print(String(KnownCard[x]));
      delay(10);
    }
  }
}

void Pull(){
  cardBackups = SD.open("cardLog.txt");
  String Incoming;
  int x = 0;
  int y = 0;

  if(cardBackups){
    while(cardBackups.available()){
      Incoming = cardBackups.read();
      if(Incoming == "-"){
        cardBackups.close();
        LibrarySize = x + 1;
        return;
      }
      else{KnownCard[x][y] = Incoming[0];}
      Serial.println(KnownCard[x][y]);

      if(y == 12){
        y=0;
        x++;
      }
      else{y++;}
      delay(100);
    }
  }
}

void ReadRFID(){
  int x = 0;
  memset(Card,0,sizeof(Card));

  while(Serial8.available()){
    char IncomingByte = Serial8.read();
    if(IncomingByte != 2 && IncomingByte != 13 && IncomingByte != 10 && IncomingByte != 3){
      Card[x] = IncomingByte;
      Serial.print(Card[x]);
      delay(10);
      x++;
    }
    else if(IncomingByte == 3){Serial.println();}
  }
}

void Run(){
  if(Serial8.available()){
    ReadRFID();
    for(int x = 0; x <= LibrarySize; x++){
      for(int y=0; y < 12; y++){
        if(Card[y] != KnownCard[x][y]){break;}
        else if(y == 11 && strlen(KnownCard[x]) != 0){
          for(int z = 0; z < NumCardTypes; z++){
            if(KnownCard[x][12]==ID[z]){
              Keyboard.print(z);
              updateDisplay(Disposables[z],Card);
              return;
            }
          }
        }
      }
    }

    updateDisplay(F("Unregistered"),Card);
  }
}

void ReProgram(){
  static unsigned long fullClearTime;
  static unsigned long clearTime;

  int Selected = analogRead(10);
  Selected = map(Selected,0,1023,0,NumCardTypes-1);

  updateDisplay(Disposables[Selected]);

  if(digitalRead(RegButton) == LOW){AddCard();}
  else if (digitalRead(ClrButton) == LOW){
    clearTime = millis() + 1000;
    fullClearTime = millis() + 10000;
    ClearData();
    if(fullClearTime <= millis()){FullClearData();}
  }
  else if(digitalRead(ClrButton) == HIGH && millis() <= clearTime){
    Shift(0);
    clearTime = clearTime - 1000;
  }

}

void AddCard(){
  int Selected = analogRead(10);
  Selected = map(Selected,0,1023,0,NumCardTypes-1);

  if(Serial8.available() && digitalRead(RegButton) == LOW){
    ReadRFID();
    Serial.println("Sorting");

    for(int x = 0; x <= LibrarySize; x++){
      for(int y = 0; y < 12; y++){
        if(KnownCard[x][y] != Card[y] && strlen(KnownCard[x]) != 0){break;}
        else if(KnownCard[x][y] != Card[y] && strlen(KnownCard[x]) == 0){
          for(int z = 0; z < 12; z++){KnownCard[x][z] = Card[z];}
          KnownCard[x][12] = ID[Selected];
          Serial.println(String(KnownCard[x]));
          LibrarySize++;
          Push();

          updateDisplay(F("Card Assigned:"),Disposables[Selected]);
          delay(3000);
          return;
        }
        else if (y == 11){
          for(int z = 0; z < NumCardTypes; z++){
            if(KnownCard[x][12]==ID[z]){
              
              updateDisplay(F("Card Already Assigned:"),Disposables[z]);
              delay(3000);
              return;
            }
          }
        }
      }
    }
  }
}

void ClearData(){
  if(Serial8.available()){
    ReadRFID();
    Serial.println("Preparing to erase");
    for(int x = 0; x <= LibrarySize; x++){
      for(int y = 0; y < 12; y++){
        if(Card[y] != KnownCard[x][y]){break;}
        else if(y == 11){
          Shift(x);
          Serial.println("Shifted");
          return;
        }
      }
    }
  }
}

void FullClearData(){memset(KnownCard,0,sizeof(KnownCard));}

void Shift(int Replace){
  if(Replace != LibrarySize){
    for(int x = Replace; x < LibrarySize; x++){
      for(int y = 0; y < 13; y++){
        KnownCard[x][y] = KnownCard[x+1][y];
      }
    }
  }
  memset(KnownCard[LibrarySize], 0, sizeof(KnownCard[LibrarySize]));
  LibrarySize = LibrarySize - 1;
  Push();
}

void loop(){
  static bool Hold;
  static bool Program;

  if(digitalRead(PMButton) == LOW && Hold == false){
    updateDisplay();
    Program = !Program;
    Hold = true;
  }
  else if(digitalRead(PMButton) == HIGH && Hold == true){Hold = false;}

  if(Program == true){
    ReProgram();
    digitalWrite(PMLight,HIGH);

  }
  else{
    Run();
    digitalWrite(PMLight,LOW);
  }
}
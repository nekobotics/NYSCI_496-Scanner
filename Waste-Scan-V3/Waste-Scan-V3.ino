#include <Keyboard.h>
#include "Display.h"

#define ClrButton 3
#define RegButton 4
#define PMButton 2
#define PMLight 31

#define numCardTypes 14
#define librarySize 100
int currentLibrarySize = 0;

const String Disposables[14] = {"aluminum can", "banana peel", "blender", "chicken bones", "envelope", "glass bottle", "soap bottle", "milk carton", "paper napkins", "soup carton", "paper tubes", "plastic bottle", "laptop computer", "smart phone"};

struct Card{
  char ID[12];
  int slide;

  bool identical(char newCard[12]){
    for(int x = 0; x < 12; x++){
      if(newCard[x] != ID[x]){return 0;}
      else if(x == 11){
        Keyboard.print(slide);
        updateDisplay(Disposables[slide],String(ID));
        return 1;
      }
    }
  };

  void Write(char newCard[12], int selectedSlide = numCardTypes){
    for(int x = 0; x < 12; x++){ID[x] = newCard[x];}
    slide = selectedSlide;
  }
};

Card cardLibrary[librarySize];

void ReadRFID(bool programMode){
  char newCard[12];
  int x = 0;
  static bool newRead;

  if(Serial8.available()){
    while(Serial8.available()){
      char IncomingByte = Serial8.read();
      if(IncomingByte != 2 && IncomingByte != 13 && IncomingByte != 10 && IncomingByte != 3){
        newCard[x] = IncomingByte;
        delay(10);
        x++;
      }
      else if(IncomingByte == 3){Serial.println();}
    }

    if(!programMode){
      for(int x = 0; x < currentLibrarySize; x++){
        if(cardLibrary[x].identical(newCard)){return;}
      }
      updateDisplay(F("Unregistered"),String(newCard));
    }
    else{newRead = true;}
  }

  if(programMode){
    Program(newCard,newRead);
    newRead = false;
  }
}

void Program(char newCard[12], bool newRead){
  static unsigned long clrTime;
  static unsigned long fullClrTime;
  static bool holdStart;
  int Selected = analogRead(10);
  Selected = map(Selected,0,1023,0,numCardTypes-1);

  updateDisplay(Disposables[Selected]);

  if(digitalRead(RegButton) == LOW && newRead){
    for(int x = 0; x < currentLibrarySize; x++){
      if(cardLibrary[x].identical(newCard)){
        updateDisplay(F("alreadey saved to"),Disposables[cardLibrary[x].slide]);
        delay(1000);
        return;
      }
    }
    currentLibrarySize++;
    cardLibrary[currentLibrarySize - 1].Write(newCard,Selected);
    updateDisplay(F("saved to"),Disposables[Selected]);
    delay(1000);
  }
  else if(digitalRead(ClrButton) == LOW){
    if(!holdStart){
      clrTime = millis() + 1000;
      fullClrTime = millis() + 10000;
      holdStart = true;
    }

    if(newRead){for(int x = 0; x < currentLibrarySize; x++){if(cardLibrary[x].identical(newCard)){Shift(x);}}}
  }
  else if(holdStart && digitalRead(ClrButton) == HIGH){
    if(millis() < clrTime){
      Shift(0);
      clrTime = clrTime - 1000;
    }
    if(millis() > fullClrTime){Reset();}
    holdStart = false;
  }
}

void Shift(int shift){
  updateDisplay(F("Removing"),String(cardLibrary[shift].ID));
  for(int x = shift; x < currentLibrarySize - 1; x++){
    for(int y = 0; y < 12; y++){cardLibrary[x].ID[y] = cardLibrary[x+1].ID[y];}
    cardLibrary[x].slide = cardLibrary[x+1].slide;
  }
  memset(cardLibrary[currentLibrarySize - 1].ID,0,sizeof(cardLibrary[currentLibrarySize - 1].ID));
  cardLibrary[currentLibrarySize - 1].slide = 0;

  if(currentLibrarySize != 0){currentLibrarySize--;}
  delay(1000);
}

void Reset(){
  updateDisplay(F("Library Deleted"));
  for(int x = 0; x < currentLibrarySize; x++){
    memset(cardLibrary[x].ID,0,sizeof(cardLibrary[x].ID));
    cardLibrary[x].slide = 0;
  }
  currentLibrarySize = 0;
  delay(1000);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial8.begin(9600);

  setupDisplay();

  pinMode(ClrButton,INPUT_PULLUP);
  pinMode(RegButton,INPUT_PULLUP);
  pinMode(PMButton,INPUT_PULLUP);
  pinMode(PMLight,OUTPUT);
}

void loop() {
  static bool programMode;
  static bool hold;

  ReadRFID(programMode);

  if(digitalRead(PMButton) == LOW && hold == false){
    programMode = !programMode;
    if(programMode){digitalWrite(PMLight,HIGH);}
    else{digitalWrite(PMLight,LOW);}
    hold = true;
  }
  else if(digitalRead(PMButton) == HIGH && hold == true){
    hold = false;
  }
}

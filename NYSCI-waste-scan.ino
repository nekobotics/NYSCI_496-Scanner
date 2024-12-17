#include <Keyboard.h>
#include <SD.h>
#include <SPI.h>

#define NumCardOptions 18
#define NumBackupCards 2

const int chipSelect = BUILTIN_SDCARD;

File cardBackups;
char PortCard[16];

String Card;
int LastSelected;

char NewCard[16];
int CurrentByte=0;
bool buttonRead;
bool programMode;

int CurrentPrgmState;

String KnowCard[NumCardOptions][NumBackupCards];

void setup() {
  // put your setup code here, to run once:
  pinMode(2,INPUT_PULLUP); //PRGM (R)
  //pinMode(3,INPUT_PULLUP); // Clear

  

  Serial.begin(9600);
  Serial8.begin(9600);
  Serial7.begin(9600);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      // No SD card, so don't do anything more - stay stuck here
    }
  }
  Serial.println("card initialized.");

  PullBackup();

  for(int x = 0; x < NumCardOptions; x++){
    for(int y = 0; y < NumBackupCards;y++){
      Serial.print(KnowCard[x][y]);
      Serial.print(" assigned to: ");
      Serial.print(x);
      Serial.println(y);
    }
  }
}

void PullBackup(){
  int CurCard = 0;
  int CurBack = 0;
  int PullNum = 0;
  char Pulled[12];
  String Pull;
  
  cardBackups = SD.open("cardLog.txt");
  if(cardBackups){
    while(cardBackups.available()){
      Pulled[PullNum] = char(cardBackups.read());
      PullNum++;

      if(PullNum >= 12){
        //Pull = String(Pulled);
        KnowCard[CurCard][CurBack] = String(Pulled);
        Serial.println(KnowCard[CurCard][CurBack]);

        PullNum = 0;

        if(CurCard >= NumCardOptions){
          CurBack++;
          CurCard = 0;
        }
        else{CurCard++;}
      }
    }
    cardBackups.close();
  }
}

void PushBackup(){
  SD.remove("cardLog.txt");
  cardBackups = SD.open("cardLog.txt",FILE_WRITE);
  for(int y = 0; y< NumBackupCards; y++){
    for(int x=0; x< NumCardOptions ;x++){
      for(int i = 0; i < 12; i++){
        char Upload = KnowCard[x][y][i];
        Serial.println(Upload);
        cardBackups.print(Upload);
      }
    }
  }
  cardBackups.close();
}

void Read(){
  Card = "";

  while(Serial8.available()){
    char IncomingByte = Serial8.read();
    if(IncomingByte != 2 && IncomingByte != 13 && IncomingByte != 10 && IncomingByte != 3){
      NewCard[CurrentByte] = IncomingByte;
      CurrentByte++;
    }

    if(IncomingByte == 3){
      Card = String(NewCard);
      Serial.println(Card);
      Serial7.println(Card);
      CurrentByte = 0;
    }
  }
}

void Run() {
  Read();

  if(Card != ""){
    for(int x = 0; x < NumCardOptions; x++){
      for(int y = 0; y < NumBackupCards; y++){
       if(KnowCard[x][y] == Card){
          Keyboard.print(x+1);
          delay(100);
          break;
        }
      } 
    }
  }
}

void Program() {
  int SelectedCard = (analogRead(10)/int(1023/(NumCardOptions-1)));
  if(SelectedCard != LastSelected){
    Serial.println(SelectedCard);
    LastSelected = SelectedCard;
  }

  if(digitalRead(22) == LOW){
    for(int x=0; x < NumBackupCards; x++){
      KnowCard[SelectedCard][x] = "";
    }
    delay(100);
    Serial.print (SelectedCard);
    Serial.println (" card data cleared");
    PushBackup();
  }

  Read();

  if(Card != ""){
    for(int x=0; x < NumBackupCards; x++){
      if(KnowCard[SelectedCard][x] == ""){
        KnowCard[SelectedCard][x] = Card;
        Serial.print("Added Card to ");
        Serial.println(SelectedCard);
        break;
      }
      else if(KnowCard[SelectedCard][x] == Card){
        KnowCard[SelectedCard][x] = "";
        Serial.println("Removed Card");
        break;
      }
    }
    PushBackup();
  }
}

void loop(){
  if(programMode){Program();}
  else{Run();}
  
  if(digitalRead(17) == LOW && buttonRead == false){
    buttonRead = true;
    programMode = !programMode;
    Serial.println(programMode);
    delay(40);
  }
  else if(digitalRead(17)== HIGH && buttonRead == true){
    buttonRead = false;
    delay(40);
  }
}

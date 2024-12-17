#include <Keyboard.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define NumCardTypes 14
#define CardsPerType 2

#define ClrButton 3
#define RegButton 4
#define PMButton 2
#define PMLight 31

#define Screen_Width 128
#define Screen_Height 32

#define OLED_MOSI 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_Reset 13

Adafruit_SSD1306 display(Screen_Width, Screen_Height,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_Reset, OLED_CS);

const int chipSelect = BUILTIN_SDCARD;
File cardBackups;

String KnownCard[NumCardTypes][CardsPerType];
String Card;
const String Disposables[NumCardTypes] = {"aluminum can", "banana peel", "blender", "chicken bones", "envelope", "glass bottle", "soap bottle", "milk carton", "paper napkins", "soup carton", "paper tubes", "plastic bottle", "laptop computer", "smart phone"};

bool Program = false;
bool Hold = false;


//-----------------------------------------------------------------------------------------------------------------------------------


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial8.begin(9600);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

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

  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  delay(500);
  // Serial.println("Currently Registered: ");
  // for(int x=0; x<NumCardTypes; x++){
  //   for(int y=0; y<CardsPerType; y++){
  //     Serial.println(KnownCard[x][y]);
  //   }
  // }
}

void Push(){
  SD.remove("cardLog.txt");
  cardBackups = SD.open("cardLog.txt",FILE_WRITE);

  for(int x = 0; x < NumCardTypes; x++){
    for(int y = 0; y < CardsPerType; y++){
      for(int z=0; z < 12; z++){
        cardBackups.print(char(KnownCard[x][y][z]));
      }
    }
  }
  cardBackups.close();
}

void Pull(){
  cardBackups = SD.open("cardLog.txt");
  char Pulled[13];

  if(cardBackups){
    while(cardBackups.available()){
      for(int z=0; z < NumCardTypes; z++){
        for(int y=0; y < CardsPerType; y++){
          for(int x= 0; x < 13; x++){
            Pulled[x] = char(cardBackups.read());
            //Serial.print();
          }
          KnownCard[z][y] = String(Pulled); 
          Serial.println(KnownCard[z][y]);
        }
      }
    }
  }
  cardBackups.close();
}

void ReadRFID(){
  while(Serial8.available()){
    char IncomingByte = Serial8.read();
    if(IncomingByte != 2 && IncomingByte != 13 && IncomingByte != 10 && IncomingByte != 3){
      String ReadByte = IncomingByte;
      Card = Card + ReadByte;
    }
    else if (IncomingByte == 3){Serial.println(Card);}
    else if(IncomingByte == 2){Card = "";}
  }
}

void Run(){
  if(Serial8.available()){
    ReadRFID();
    for(int x = 0; x < NumCardTypes; x++){
      for(int y=0; y < CardsPerType; y++){
        if(Card == KnownCard[x][y] && Card!=""){
          Keyboard.print(x);

          display.clearDisplay();
          display.setCursor(0,0);
          display.println(Disposables[x]);
          display.println(Card);
          display.display();
          delay(200);

          return;
        }
      }
    }

    display.clearDisplay();
    display.setCursor(0,0);
    display.println(F("Unregistered"));
    display.println(Card);
    display.display();
    delay(200);
  }
}

void ReProgram(){
  int Selected = analogRead(10);
  Selected = map(Selected,0,1023,0,NumCardTypes-1);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println(Disposables[Selected]);
  display.display();
  delay(200);

  if(Serial8.available() && digitalRead(RegButton)==LOW){
    ReadRFID();
    
    for(int x = 0; x < NumCardTypes; x++){
      for(int y = 0; y < CardsPerType; y++){
        if(Card == KnownCard[x][y] && Card != ""){
          display.clearDisplay();
          display.setCursor(0,0);
          display.println(F("Card Already Assigned:"));
          display.println(Disposables[x]);
          display.display();
          delay(3000);

          return;
        }
      }
    }

    for(int x = 0; x < CardsPerType; x++){
      if(KnownCard[Selected][x] == ""){
        KnownCard[Selected][x] = Card;
        Push();

        display.clearDisplay();
        display.setCursor(0,0);
        display.println(F("Card Assigned:"));
        display.println(Disposables[Selected]);
        display.display();
        delay(3000);

        return;

      }
      else if (x == CardsPerType - 1){
        display.clearDisplay();
        display.setCursor(0,0);
        display.println(F("Max Cards registered for: "));
        display.println(Disposables[Selected]);
        display.println(F("Please clear repository"));
        display.display();
        delay(3000);
      }
    }
  }

}

void loop(){
  if(digitalRead(PMButton) == LOW && Hold == false){
    display.clearDisplay();
    display.display();
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

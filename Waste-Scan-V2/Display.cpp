#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Display.h"

static Adafruit_SSD1306 display(Screen_Width, Screen_Height,OLED_MOSI, OLED_CLK, OLED_DC, OLED_Reset, OLED_CS);

void setupDisplay(){
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
}

void updateDisplay(String lineOne = "",String lineTwo = ""){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(lineOne);
  display.println(lineTwo);
  display.display();
  delay(200);
}
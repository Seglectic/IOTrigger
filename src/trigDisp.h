#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/Picopixel.h>
#include <Fonts/FreeSansBold18pt7b.h>

// TODO - Add a battery display widget to the panel

// ╭────────────────╮
// │  Display Init  │
// ╰────────────────╯
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C // 0x3C for 128x32 or 0x3D for 128x64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ╭───────────────────────╮
// │  Global Display Vars  │
// ╰───────────────────────╯
// Status Frame - Upper region for numerical display
int16_t statusWidth = 32;
int16_t statusHeight = 32;
bool    countInversion = false;

// Panel Frame - Lower region for status, icons, 
int16_t panelX = 0;
int16_t panelY = 32;
int16_t panelHeight = 96;
int16_t panelWidth = 32;

// ╭─────────╮
// │  Setup  │
// ╰─────────╯
void displaySetup(){
  display.begin();
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setRotation(1);
  display.setCursor(0,0);
  display.display(); 
}

// Sends low level instruction for SSD1306 sleep mode
void displayShutdown(){  
  display.ssd1306_command(SSD1306_DISPLAYOFF); 
}

// ╭──────────────────────────╮
// │  Status Section (Count)  │
// ╰──────────────────────────╯
void displayStatus(){
  display.fillRect(0,0,32,32,BLACK);  // Clear the top portion of the display
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(7,24); 
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  (countInversion) ? 
    display.print(6-triggerCount):
    display.print(triggerCount);
}

// ╭──────────────────────────╮
// │  Animating timeout line  │
// ╰──────────────────────────╯
void displayTimeout(){
  long timeOut = map(millis()-lastActionTime,0,executionTimer,0,panelHeight);
  int tX = 29; //X position of the display line
  display.drawLine(tX,panelY+1,tX,panelY+panelHeight-2,BLACK); // Clear the region
  display.drawLine(tX,126,tX,timeOut+panelY,WHITE); 
}

// ╭─────────────────╮
// │  Panel Section  │
// ╰─────────────────╯
void displayPanel(){ //Lower frame drawing
  display.drawRect(panelX,panelY,panelWidth,panelHeight,WHITE);     // Draw rect around the region
  displayTimeout();
}

// ╭─────────────────────────╮
// │  Animated screen clear  │
// ╰─────────────────────────╯
void displayClear(uint8_t step){
  display.setRotation(2);
  for (size_t i = 0; i < 128; i+=step){
    display.fillRect(i,0,step,32,WHITE);
    display.display();
  }
  for (size_t i = 0; i < 128; i+=step){
    display.fillRect(i,0,step,32,BLACK); 
    display.display();
  }
}

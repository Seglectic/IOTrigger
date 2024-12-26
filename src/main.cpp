#include <Arduino.h>
#include <esp_sleep.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                       Init                                                        │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯

// ╭─────────────╮
// │  Constants  │
// ╰─────────────╯
#define WAKE_UP_PIN_BITMASK (1ULL << 1)  // GPIO 1 bitmask for ext1 wake (D1 on Xiao ESP32C6)
#define INACTIVITY_TIMEOUT 10000         // 10 seconds inactivity timeout
#define BUZZPIN 19                       // Vibrator buzzer pin
#define BATTERY_PIN 0                    // Analog pin connected to the voltage divider
#define VOLTAGE_DIVIDER_RATIO 2.0        // Adjust this based on your divider resistor values
#define ADC_MAX 4095                     // Max value for ADC (12-bit resolution on ESP32)
#define ADC_REF_VOLTAGE 4.2              // Reference voltage (depends on ESP32's power source)

// ╭───────────────╮
// │  Global Vars  │
// ╰───────────────╯
unsigned long lastActionTime = 0;     // Tracks last time a button was pressed or action occured
unsigned long buzzTime       = 0;     // Starting time of a buzz event
bool vibing                  = false; // Currently buzzin?
unsigned long buzzLength     = 200;   // Buzz ms length
int LEDBrightness = 0;


// ╭─────────────────╮
// │  Display Setup  │
// ╰─────────────────╯
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C // 0x3D for 128x64 or 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void fadeOutDisplay(uint8_t steps, uint16_t delayTime) { // Purge this, doesn't really work and creates delay on shutdown?
  for (uint8_t brightness = 255; brightness > 0; brightness -= steps) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightness); // Set brightness level
    delay(delayTime); // Wait between steps
  }
  display.ssd1306_command(SSD1306_DISPLAYOFF); // Finally, turn off the display
}

void displayShutdown(){
  display.ssd1306_command(SSD1306_DISPLAYOFF); // Finally, turn off the display
}

// Animate white pixels onto screen then black pixels
void screenClear(uint8_t step){
  for (size_t i = 0; i < 128; i+=step){
    display.fillRect(i,0,step,32,WHITE);
    display.display();
  }
  delay(100);
  for (size_t i = 0; i < 128; i+=step){
    display.fillRect(i,0,step,32,BLACK); 
    display.display();
  }
}

// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                 Helper Functions                                                  │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯

// ╭───────────────────────────────────────╮
// │  blinkLED                             │
// │  Turns on the LED for a set duration  │
// │  before turning back off (blocking)   │
// ╰───────────────────────────────────────╯
void blinkLED(uint32_t duration) {
  digitalWrite(LED_BUILTIN, LOW);  // Turn on the LED
  delay(duration);                 // LED on for the specified duration
  digitalWrite(LED_BUILTIN, HIGH); // And back off
}

// ╭─────────────────────────────────────╮
// │  blinkLoop                          │
// │  Loops blinkLED x number of times,  │
// ╰─────────────────────────────────────╯
void blinkLoop(uint32_t durationOn, uint32_t durationOff, uint16_t x){
  for (uint16_t i = 0; i < x; i++){
    blinkLED(durationOn);   
    delay(durationOff); 
  }
}

void buzz(){
    buzzTime = millis();
    vibing = true;
    digitalWrite(BUZZPIN, HIGH);
}

void buzzTick(){
  if (vibing && (millis()-buzzTime>=buzzLength)){
    digitalWrite(BUZZPIN, LOW);
    vibing = false;
  }
}

void ledTick(){ //Slowly fades the LED up
  if (LEDBrightness<255){
    analogWrite(LED_BUILTIN,255-LEDBrightness);
    LEDBrightness+=5;
  }
}

// Function to read and calculate battery voltage
void readBatteryVoltage() {
  int x =  display.getCursorX();
  int y = display.getCursorY();
  int rawADC = analogRead(BATTERY_PIN);                 // Read ADC value
  float voltage = (rawADC * ADC_REF_VOLTAGE) / ADC_MAX; // Convert ADC value to voltage
  voltage *= VOLTAGE_DIVIDER_RATIO;                     // Adjust for voltage divider
  display.setCursor(80,0);
  display.print(voltage,2);
  display.print('v');
  display.setCursor(x,y);
  display.display();
}

// Draws a line on the edge of the display that correlates with how much time is remaining before shutdown
void displayTimeout(){
  long timeOut = map(millis()-lastActionTime,0,INACTIVITY_TIMEOUT,0,32);
  display.fillRect(125,0,3,32,BLACK); // Clear the zone
  display.fillRect(125,0,3,32-timeOut,WHITE);
}


// ╭────────────────────────────────────────────────────────────╮
// │  connectToWiFi                                             │
// │  Connects ESP32 to a WiFi network using SSID and Password  │
// │  Prints the status to Serial Monitor                       │
// ╰────────────────────────────────────────────────────────────╯
void connectToWiFi(const char* ssid, const char* password) {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password); // Start the connection

  uint8_t retryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    retryCount++;
    if (retryCount > 100) { // Timeout after ~10 seconds
      Serial.println("\nFailed to connect to WiFi.");
      return;
    }
  }
  Serial.println("\nWiFi connected!");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
}


// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                       Setup                                                       │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
void setup() {
  Serial.begin(115200); 
  Serial.println("Boot OK");
  esp_sleep_enable_ext1_wakeup(WAKE_UP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH); // Apply the wake bitmask when it goes HIGH
  
  pinMode(LED_BUILTIN, OUTPUT); // Enable output for the built in LED
  pinMode(1, INPUT_PULLDOWN);   // Enable pull-down resistor on D1 wake pin
  pinMode(19,OUTPUT);           // Vibrator Buzzer Pin
  // digitalWrite(LED_BUILTIN,HIGH);
  analogWrite(LED_BUILTIN,255); //Init the LED off
  
  // Boot up buzz- Probably replace this with a pattern.
  digitalWrite(19, HIGH);
  delay(200);
  digitalWrite(19,LOW);

  // Initialize the display and print some stuff.
  display.begin();
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setRotation(2);
  display.setCursor(0,0);

  screenClear(12);

  display.println("SegSys '24");
  display.display(); 
  readBatteryVoltage(); 
}

// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                       Loop                                                        │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
void loop() {
  if (digitalRead(1) == HIGH) { // Check for trigger pull on D1
    display.print("!");
    buzz();
    display.display();
    lastActionTime = millis(); // Reset inactivity timer
  }
  displayTimeout();
  display.display();
  buzzTick();
  ledTick();
  // Check for inactivity timeout
  if (millis() - lastActionTime >= INACTIVITY_TIMEOUT) {
    Serial.println("Inactivity, entering deep sleep...");
    screenClear(12);
    displayShutdown(); 
    esp_deep_sleep_start(); // Enter deep sleep
  }

  delay(50); // Small delay for stability
}
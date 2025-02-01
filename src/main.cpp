#include <Arduino.h>
#include <esp_sleep.h>
#include <SPI.h>
#include <Wire.h>


// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                     IOTrigger                                                     │
// │                                         Multifunction IoT Trigger System                                          │
// │                                               Seglectic Systems '24                                               │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯


// TODO - Make wifi onboarding smooth to configure and be re-configurable by user
// TODO - Create 'executioner' timer that executes action after like 1s inactivity
// TODO - Separate header file for buzz
// TODO - Wake On LAN request
// TODO - Create a web request trigger
// TODO - Create something that maps number of trigger pulls to a function call

// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                       Init                                                        │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯

// ╭─────────────╮
// │  Constants  │
// ╰─────────────╯
#define WAKE_UP_PIN_BITMASK   (1ULL << 1)  // GPIO 1 bitmask for ext1 wake (D1 on Xiao ESP32C6)
#define BUZZPIN               19           // Vibrator buzzer pin
#define BATTERY_PIN           0            // Analog pin connected to the voltage divider
#define VOLTAGE_DIVIDER_RATIO 2.0          // Adjust this based on your divider resistor values
#define ADC_MAX               4095         // Max value for ADC (12-bit resolution on ESP32)
#define ADC_REF_VOLTAGE       4.2          // Reference voltage (depends on ESP32's power source)
#define DEBOUNCE_DELAY        20           // Button debounce period

// ╭───────────────╮
// │  Global Vars  │
// ╰───────────────╯
unsigned long lastActionTime = 0;          // Tracks last time a button was pressed or action occured
int executionTimer           = 1200;       // How much time should remain after an action is made
int triggerCount             = 0;          // Incremented on each button press (volatile to be avail in interrupt)
bool triggerPull             = false;      // Flag for trigger pulling event
unsigned long lastPressTime  = 0;          // Track time of last press for debouncin
int interval                 = 20 ;        // Interval for main loop

// ╭─────────────────────────╮
// │  Load custom libraries  │
// ╰─────────────────────────╯
#include <trigLED.h>
#include <trigDisp.h>
#include <trigWiFi.h>
#include <trigBuzz.h>
#include <executioner.h>

// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                 Helper Functions                                                  │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯

// ╭─────────────────────╮
// │  Trigger Interrupt  │
// ╰─────────────────────╯
void IRAM_ATTR triggerInterrupt() {triggerPull = true;}

// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                       Setup                                                       │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
void setup() {
  esp_sleep_enable_ext1_wakeup(WAKE_UP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);  // Apply wake bitmask upon going HIGH
  pinMode(LED_BUILTIN, OUTPUT);                                                 // Enable output for the built in LED
  analogWrite(LED_BUILTIN,255);                                                 // Init the LED off
  pinMode(1, INPUT_PULLDOWN);                                                   // Enable pull-down resistor on D1 wake pin
  attachInterrupt(digitalPinToInterrupt(1), triggerInterrupt, RISING);          // Trigger on RISING edge 
  pinMode(19,OUTPUT);                                                           // Vibrator Buzzer Pin
  displaySetup();
  buzz(100,255);
  // wifiConnect(); 
}

// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                         Loop                                                      │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
void loop() {
  displayStatus(); 
  displayPanel();
  buzzTick();
  ledTick();
  if(digitalRead(1)){lastActionTime=millis();}
  display.display();
  triggerExecution();
  if (millis() - lastActionTime >= executionTimer) {     // Check for inactivity timeout
    screenClear(6);                                      // Show a visual of the screen turning off
    displayShutdown();                                   // Turn off the display
    esp_deep_sleep_start();                              // Enter deep sleep
  }

  delay(interval);
}
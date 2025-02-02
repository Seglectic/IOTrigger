
void powerDown(){
    displayClear(6);                                      // Show a visual of the screen turning off
    displayShutdown();                                   // Turn off the display
    esp_deep_sleep_start();                              // Enter deep sleep
}

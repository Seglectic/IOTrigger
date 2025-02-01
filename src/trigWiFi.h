#include <WiFi.h>
#include <WiFiManager.h>

void wifiConnect() {

    display.setFont(&Picopixel);
    display.setCursor(5,40);
    display.display();
    WiFiManager wm;
    wm.setConfigPortalBlocking(false); //
    wm.setConfigPortalTimeout(10);
    if (!wm.autoConnect("IOTriggerAP")) {
        // Display message for WiFi portal creation
        display.fillRect(0, 32, 128, 32, BLACK);  // Clear bottom area
        display.drawRect(0, 32, 128, 32, WHITE);  // Draw border
        display.setCursor(5, 40);  // Text below top 32 pixels
        display.setTextColor(WHITE);
        display.println(("Portal..."));
        display.display();

        // Start the portal (blocking call)
        wm.startConfigPortal();
    }

    // Successfully connected to WiFi
    // Serial.println(F("WiFi connected!"));
    display.fillRect(0, 32, 128, 32, BLACK);  // Clear the portal status area
    display.drawRect(0, 32, 128, 32, WHITE);  // Keep the border
    display.setCursor(5, 40);
    display.println(F("WiFi Connected!"));
    display.display();
    delay(2000);  // Briefly show connection message
    display.fillRect(0, 32, 128, 32, BLACK);  // Clear bottom area
    display.display();
}
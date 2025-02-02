
struct TriggerAction { // Action struct that holds callback and parameters for execution.
    void (*callback)(const char*, const char*, const char*) = nullptr;
    const char* param1 = "";  // Domain  (e.g., "scene", "switch", or URL for HTTP)
    const char* param2 = "";  // Entity  (e.g., "light.living_room" or MAC address for WOL)
    const char* param3 = "";  // Service (e.g., "toggle", "turn_on", "turn_off" or empty for HTTP)
};

TriggerAction triggerActions[6]; // Array of TA structs

void setAction(int index, TriggerAction action){  //Sets an index in the array to the specified action
  if (index >= 0 && index < sizeof(triggerActions) / sizeof(triggerActions[0])) {
    triggerActions[index] = action;
  }
}






// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                            Send Home Assistant Request                                            │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
const char* HA_URL = "http://192.168.1.99:8123/api/services/";

void homeAssistantAction(const char* domain="",const char* entity="",const char* service=""){
  WiFiManager wm;
      
    // Attempt to connect to stored WiFi credentials
    if (!wm.autoConnect("IOTrigger", nullptr)) {  
        buzzError();  // Haptic feedback for failure
        return;
    }
    
    // Make sure the params are populated
    if (strlen(domain) == 0 || strlen(entity) == 0 || strlen(service) == 0) {
        buzzError();  // Indicate failure
        return;
    }

    // Load Home Assistant Key from file
    File haKeyFile = SPIFFS.open("/haToken.txt","r");
    String haKey = haKeyFile.readString();
    haKey.trim();
    haKeyFile.close();

    HTTPClient http;
    String url = String(HA_URL) + domain + "/" + service;
    String payload = "{\"entity_id\": \"" + String(entity) + "\"}";


    http.begin(url);
    http.addHeader("Authorization", String("Bearer ") + haKey);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload); 
    display.setFont(&Picopixel); // Replace this with a generic function in trigDisp for erroring
    display.setRotation(0);
    display.setCursor(33,5);
    display.print("Response: ");
    display.println(httpResponseCode);
    http.end();

    // If request was successful (HTTP 200 or 202), give success haptic feedback
    if (httpResponseCode == 200 || httpResponseCode == 202) {
        // buzz(300, 255);  // Success haptic feedback
    } else {
        // buzzError();  // Failure haptic feedback
        // buzzError();
    }

  display.display();
  return;
}


// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                                  Host Web Portal                                                  │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
void webPageAction(const char* unused1, const char* unused2, const char* unused3) {
  WiFiManager wm;
  if (!wm.autoConnect("IOTrigger")) {
    buzzError();
    return;
  }

  display.setRotation(0);
  display.setFont(&Picopixel);
  display.setCursor(2,5);
  display.println("Hosting Server..");
  display.println(WiFi.localIP());
  display.display();
  
  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.on("/haToken", HTTP_POST, handleToken);
  server.begin();

  // Handle clients (for a non-blocking version, integrate server.handleClient() into your main loop)
  while (true) {
    server.handleClient();     // Handle web requests
    if(digitalRead(1)){break;} // Break if you pull the trigger
    delay(1);                  // Yield to avoid watchdog issues
  }
}



void actionSetup(){ // Apply the default actions to the triggerActions array
    TriggerAction haAction = {homeAssistantAction, "light", "light.minikey", "toggle"};
    TriggerAction webPortalAction = {webPageAction};
    
    setAction(1, haAction);
    setAction(2,webPortalAction);
}
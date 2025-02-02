
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

// Action Callback pointer type for clarity.
typedef void (*ActionCallback)(const char*, const char*, const char*);

// Callback forward declarations
void homeAssistantAction(const char*, const char*, const char*);
void webPageAction(const char*, const char*, const char*);

// ╭────────────────────╮
// │  Helper functions  │
// ╰────────────────────╯

// Return a string name from the callback pointer
String getNameFromCallback(ActionCallback cb) {  
  if (cb == homeAssistantAction) return "homeAssistantAction";
  if (cb == webPageAction) return "webPageAction";
  return ""; // unknown
}
// Return a callback from its string name
ActionCallback getCallbackFromName(const String &name) { 
  if (name == "homeAssistantAction") return homeAssistantAction;
  if (name == "webPageAction") return webPageAction;
  return nullptr;
}

// ╭───────────────────────────────────────────╮
// │  Save Actions                             │
// │  Saves TriggerActions array to text file  │
// ╰───────────────────────────────────────────╯
void saveActions() {
  File file = SPIFFS.open("/trigActions.txt", "w");
  if (!file) {
    // Serial.println("Failed to open trigActions.txt for writing");
    return;
  }
  // For each action, write a line: callbackName|param1|param2|param3
  for (int i = 0; i < 6; i++) {
    if (triggerActions[i].callback == nullptr) {file.println();} // Print an empty line if the action in the array is empty
    else {
      String line = getNameFromCallback(triggerActions[i].callback) + "|" +
                    String(triggerActions[i].param1) + "|" +
                    String(triggerActions[i].param2) + "|" +
                    String(triggerActions[i].param3);
      file.println(line);
    }
  }
  file.close();
}


// ╭─────────────────────────────────────────────╮
// │  Load Actions                               │
// │  Loads the TriggerActions array from file.  │
// │  One action per line, delimited by |        │
// ╰─────────────────────────────────────────────╯
void loadActions() {
  File file = SPIFFS.open("/trigActions.txt", "r");
  if (!file) {
    // Serial.println("Failed to open trigActions.txt for reading");
    return;
  }

  for (int i = 0; i < 6; i++){
    String line = file.readStringUntil('\n');
    line.trim();
    // A blank line indicates an empty TriggerAction.
    if (line.length() == 0){ 
      triggerActions[i].callback = nullptr;
      triggerActions[i].param1 = "";
      triggerActions[i].param2 = "";
      triggerActions[i].param3 = "";
      continue;
    }

    // Expecting: callbackName|param1|param2|param3
    int firstDelim  = line.indexOf('|');
    int secondDelim = line.indexOf('|', firstDelim + 1);
    int thirdDelim  = line.indexOf('|', secondDelim + 1);
    
    if (firstDelim == -1 || secondDelim == -1 || thirdDelim == -1) { // Make sure each delim was found, else this one's blank
      // Serial.println("Malformed line in trigActions.txt");
      triggerActions[i].callback = nullptr;
      triggerActions[i].param1 = "";
      triggerActions[i].param2 = "";
      triggerActions[i].param3 = "";
      continue;
    }

    // Separate the lines from the file 
    String cbName = line.substring(0, firstDelim);
    String param1 = line.substring(firstDelim + 1, secondDelim);
    String param2 = line.substring(secondDelim + 1, thirdDelim);
    String param3 = line.substring(thirdDelim + 1);

    // Set the callback in main array from the converted str-callback function
    triggerActions[i].callback = getCallbackFromName(cbName);

    // Allocate new copies of the parameter strings.
    triggerActions[i].param1 = strdup(param1.c_str());
    triggerActions[i].param2 = strdup(param2.c_str());
    triggerActions[i].param3 = strdup(param3.c_str());
  }
  file.close();
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
  loadActions();

  // The following can set default actions when uncommented:
  
  // First create a TriggerAction struct
  // TriggerAction haAction = {homeAssistantAction, "light", "light.minikey", "toggle"};
  // TriggerAction webPortalAction = {webPageAction};

  // Then set those TriggerActions to the main actions array
  // setAction(1, haAction);
  // setAction(2,webPortalAction);

  // And optionally save them to file.
  // saveActions();
}
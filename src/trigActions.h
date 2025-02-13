// ╭─────────────────────────────────────────╮
// │  Trigger Actions                        │
// │                                         │
// │  Defines the TriggerAction Struct       │
// │  and associated functions for saving    │
// │  loading, and setup. Some actions also  │
// │  defined here.                          │
// ╰─────────────────────────────────────────╯


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

// Callback forward declarations
void homeAssistantAction(const char*, const char*, const char*);
void webPortalAction(const char*, const char*, const char*);

// ╭────────────────────╮
// │  Helper functions  │
// ╰────────────────────╯
//TODO Make this some sorta map or something

// Return a string name from the callback pointer
String getNameFromCallback(ActionCallback cb) {  
  if (cb == homeAssistantAction) return "homeAssistantAction";
  if (cb == webPortalAction) return "webPortalAction";
  return ""; // unknown
}
// Return a callback from its string name
ActionCallback getCallbackFromName(const String &name) { 
  if (name == "homeAssistantAction") return homeAssistantAction;
  if (name == "webPortalAction") return webPortalAction;
  return nullptr;
}

// ╭───────────────────────────────────────────╮
// │  Save Actions                             │
// │  Saves TriggerActions array to text file  │
// ╰───────────────────────────────────────────╯
void saveActions() {
  File file = LittleFS.open("/trigActions.txt", "w");
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
  File file = LittleFS.open("/trigActions.txt", "r");
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

    display.setFont(&Picopixel); // Replace this with a generic function in trigDisp for erroring //FIXME
    display.setRotation(0);
    display.setCursor(0,5);
    display.drawPixel(100,5,WHITE);
    display.println(domain);
    display.println(entity);
    display.println(service);
    display.display();
    delay(1000);

    // display.print("Response: ");
    // display.println(httpResponseCode);

      
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
    File haKeyFile = LittleFS.open("/haToken.txt","r");
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

    http.end();

    // If request was successful (HTTP 200 or 202), give success haptic feedback
    if (httpResponseCode == 200 || httpResponseCode == 202) {
        // buzz(300, 255);  // Success haptic feedback
    } else {
        // buzzError();  // Failure haptic feedback
        // buzzError();
    }

  return;
}






void actionSetup(){ // Apply the default actions to the triggerActions array
  loadActions();

  // The following can set default acti+ons when uncommented:
  
  // First create a TriggerAction struct
  TriggerAction portalAction = {webPortalAction};
  // TriggerAction haAction = {homeAssistantAction, "light", "light.minikey", "toggle"};

  // Then set those TriggerActions to the main actions array
  setAction(1, portalAction); // 1 aka double-tap
  // setAction(2, haAction);

  // And optionally save them to file.
  saveActions();
}
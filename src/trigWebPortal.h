//TODO - Add a wifi signal indicator to update 
//TODO - Create an update routine to be run from the main loop?


// ╭────────────────────╮
// │  Server Endpoints  │
// ╰────────────────────╯
void handleRoot() {
  File file = LittleFS.open("/index.html","r"); // Load t he index page from LittleFS
  // server.send(200, "text/html", html);
  server.streamFile(file,"text/html");
  digitalWrite(BUZZPIN,HIGH);
  delay(50);
  digitalWrite(BUZZPIN,LOW);
  file.close();
}

void handleToken() {
  String token = server.arg("haToken");          // Retrieve token from POST data ("haToken")
  File file = LittleFS.open("/haToken.txt", "w");  // Open the file for writing (Creates if not there)
  if (!file) {                                   // Handle FS error
    server.send(500, "text/html", "Failed to open file for writing.");
    return;
  }
  file.print(token);
  file.close();
  server.send(200, "text/html","Token saved!");
}


void handleSubmit() {
  // Retrieve submitted values
  String cb = server.arg("callback");
  String p1 = server.arg("param1");
  String p2 = server.arg("param2");
  String p3 = server.arg("param3");
  // Process parameters or update TriggerAction accordingly
  server.send(200, "text/html", "Action OK!");
}

// Accepts a trigger action from the web portal and saves to file
void handleTriggerAction() {
  // Retrieve URL-encoded parameters
  String indexStr = server.arg("index");
  if (indexStr.length() == 0) {
    server.send(400, "text/html", "Missing index parameter");
    return;
  }
  int index = indexStr.toInt();
  if (index < 0 || index >= 6) {
    server.send(400, "text/html", "Index out of bounds");
    return;
  }

  String cbName = server.arg("callback");
  String p1 = server.arg("param1");
  String p2 = server.arg("param2");
  String p3 = server.arg("param3");

  if (cbName.length() == 0) {
    server.send(400, "text/html", "Missing callback parameter");
    return;
  }

  // Convert the callback name to a function pointer.
  ActionCallback cb = getCallbackFromName(cbName);
  if (!cb) {
    server.send(400, "text/html", "Invalid callback name");
    return;
  }

  // Create a new TriggerAction
  TriggerAction newAction;
  newAction.callback = cb;
  newAction.param1 = strdup(p1.c_str());
  newAction.param2 = strdup(p2.c_str());
  newAction.param3 = strdup(p3.c_str());

  display.clearDisplay();
  display.setCursor(0,5);

  display.println(cbName);
  display.println(newAction.param1);
  display.println(newAction.param2);
  display.println(newAction.param3);
  display.display();


  // Update the in-memory array at the specified index.
  triggerActions[index] = newAction;

  // Save the entire actions array back to file.
  server.send(200, "text/html", "Action updated successfully!");
  
  saveActions();
}


// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                          Host Web Portal TriggerAction Callback                                   │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
void webPortalAction(const char* unused1, const char* unused2, const char* unused3) {

    display.setFont(&Picopixel); // Replace this with a generic function in trigDisp for erroring //FIXME
    display.setRotation(0);
    display.setCursor(0,5);
    display.println("portal");
    display.println(unused1);
    display.println(unused2);
    display.println(unused3);
    display.display();
    delay(1000);

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
  server.on("/updateAction", HTTP_POST, handleTriggerAction);
  server.begin();

  // Handle clients - Blocking
  //TODO Make this end after like 15 or 20 minutes
  //TODO Probably make more power efficient
  while (true) {
    server.handleClient();     // Handle web requests
    if(digitalRead(1)){break;} // Break if you pull the trigger
    delay(1);                  // Yield to avoid watchdog issues
  }
}
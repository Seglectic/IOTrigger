#include <WebServer.h>



WebServer server(80); // Web server instance

void handleRoot() {
  File file = SPIFFS.open("/index.html","r"); // Load the index page from SPIFFS
  // server.send(200, "text/html", html);
  server.streamFile(file,"text/html");
  file.close();
}

void serverSetup(){
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

void webPageAction(const char* unused1, const char* unused2, const char* unused3) {
  WiFiManager wm;
  if (!wm.autoConnect("IOTrigger")) {
    buzzError();
    return;
  }

  display.setRotation(0);
  display.setFont(&Picopixel);
  display.setCursor(33,5);
  display.println("Hosting Server..");
  display.println(WiFi.localIP());
  display.display();
  
  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.begin();

  // Handle clients (for a non-blocking version, integrate server.handleClient() into your main loop)
  while (true) {
    server.handleClient();
    delay(1); // Yield to avoid watchdog issues
  }
}


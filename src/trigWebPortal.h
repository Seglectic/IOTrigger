#include <WebServer.h>

//TODO - Add a wifi signal indicator to update 
//TODO - Create an update routine to be run from the main loop?

WebServer server(80); // Web server instance


// ╭────────────────────╮
// │  Server Endpoints  │
// ╰────────────────────╯
void handleRoot() {
  File file = SPIFFS.open("/index.html","r"); // Load t he index page from SPIFFS
  // server.send(200, "text/html", html);
  server.streamFile(file,"text/html");
  digitalWrite(BUZZPIN,HIGH);
  delay(50);
  digitalWrite(BUZZPIN,LOW);
  file.close();
}

void handleToken() {
  String token = server.arg("haToken");          // Retrieve token from POST data ("haToken")
  File file = SPIFFS.open("/haToken.txt", "w");  // Open the file for writing (Creates if not there)
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



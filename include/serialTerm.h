#include <Arduino.h>
#include <map> // Map library

// Defines a type for function pointers to be called when a command is received
typedef void (*CommandCallback)(); 

class SerialTerminal{
  private:
    std::map<const char*, CommandCallback> commands; // Map of commands to function pointers
  public:
    char charBuffer[64] = "";            // Buffer for incoming serial data
    bool prompt;                         // Flag to determine if the terminal should prompt for input live

    //Constructor
    SerialTerminal(bool prompt = false) : prompt(prompt){ 
      registerCommand("help", []() {
          Serial.println("Available commands:");
          Serial.println("help - Display this message");
          Serial.println("reboot - Reboot the device");
          Serial.println("version - Display the firmware version");
      });
    };

    // Begin (call in setup)
    void begin(){
      // Print a gaudy ready message
      if (prompt){
        Serial.println(""); //Print a new line to separate garbage 
        Serial.println(" ___             _      ");
        Serial.println("| _ \\___ __ _ __| |_  _ ");
        Serial.println("|   / -_) _` / _` | || |");
        Serial.println("|_|_\\___\\__,_\\__,_|\\_, |");
        Serial.println("                   |__/ ");
        promptInput();
      }
    }

    // Report incoming text as it's typed
    void reportInput(char inChar){ 
      if (!prompt) return;
      Serial.print(inChar);
    }

    // Display a prompt character to indicate expected input
    void promptInput(){
      if (!prompt) return;
      Serial.print(">");    // Print a prompt character
    }

    // Update to check for incoming Serial data
    void update(){
      if (Serial.available() > 0) {
        char inChar = Serial.read();                 // Get the latest byte
        switch (inChar){
          case 0x08: //Backspace
            if (strlen(charBuffer) > 0){
              int lastPos = strlen(charBuffer) - 1; // Get the pos of last char in buffer
              charBuffer[lastPos] = '\0';           // Remove the last char from buffer
              Serial.print("\b \b");                  // Remove the last char from console
            }
            break;
          case '\n': // "Enter" key
            charBuffer[strlen(charBuffer)] = '\0';     // Terminate the string with a null character
            parseCommand(charBuffer);
            promptInput();                             // Print the prompt
            memset(charBuffer, 0, sizeof(charBuffer)); // Clear the buffer
            break;
          default:
            charBuffer[strlen(charBuffer)] = inChar;     // Append to the buffer
            reportInput(inChar);
            break;
        }
      }
    }

  void registerCommand(const char* command, CommandCallback callback){
    commands[command] = callback;
  }

  private:

    void parseCommand(char command[64]){
        //Trim newline and carriage return:
        command[strcspn(command, "\n")] = 0;
        command[strcspn(command, "\r")] = 0;  // This is sent by platformio terminal
        
        if (strlen(command) == 0) { // Simply return if the command is empty
            Serial.println("Command is empty");
            return;
        }

        auto it = commands.find(command); // Find the command in the map
        if (it != commands.end()) { // If the command is found
            Serial.println("Executing command");
            it->second(); // Call the function pointer
        } else {
            Serial.print("Error: Unknown command - ");
            Serial.println(command);
        }
    }
};

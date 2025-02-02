// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                       Process Triggers and Execute Actions                                        │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯

void triggerExecution(){
  int maxTriggers = sizeof(triggerActions) / sizeof(triggerActions[0]);                   // Count array
  if (triggerCount < maxTriggers && triggerActions[triggerCount].callback != nullptr) {   // Run if # is valid and action has a non-null callback
    TriggerAction &action = triggerActions[triggerCount];
    action.callback(action.param1,action.param2,action.param3);
  }else { //TODO Maybe put stuff here when an action runs without a pointer. (Notify that it's unconfigured or error out)
    return;
  }
}



char * commands = new char[256];

void handleTriggerPull(){   // Process trigger pull and increment counter 
  unsigned long currentTime = millis(); 

  if(!digitalRead(1)){return;}

  if (triggerPull && currentTime > lastPressTime + DEBOUNCE_DELAY && triggerCount<6) { // Check the debounce    
    triggerCount++;
    lastPressTime = currentTime;
    lastActionTime = currentTime;
    buzz(100,255);
    triggerPull = false;
  }

  if (triggerCount>=6){ // Shorten the timeout if you're at the max pulls (quick shutdown)
    executionTimer = 100;
    interval = 5;
  }

}

void triggerTick(){ // Check the timer and run the executioner
  if (millis() >= executionTimer+lastActionTime) {     // Check for inactivity timeout
    triggerExecution();
    powerDown();
  }
}
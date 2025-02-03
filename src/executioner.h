// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                       Process Trigger Pulls and Execute Actions                                   │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯

void triggerExecution(){
  int maxTriggers = sizeof(triggerActions) / sizeof(triggerActions[0]);                   // Count array
  if (triggerCount < maxTriggers && triggerActions[triggerCount].callback != nullptr) {   // Run if # is valid and action has a non-null callback
    TriggerAction &action = triggerActions[triggerCount];                                 // Create 'action' reference
    action.callback(action.param1,action.param2,action.param3);                           // Execute action
  }else {                                                                                 //TODO Maybe put stuff here when an action runs without a pointer.
    return;
  }
}

TriggerAction longPull = {webPortalAction};

void handleTriggerPull(unsigned long currentMillis){   // Process trigger pull and increment counter 

  if(!digitalRead(1)){return;} // Debounce, skip if the button is still held
 
  //TODO Check if lastPress/Action time is redundant?
  if (triggerPull && currentMillis > lastPressTime + DEBOUNCE_DELAY && triggerCount<5) { // Check the debounce    
    triggerCount++;
    lastPressTime = currentMillis;
    lastActionTime = currentMillis;
    buzz(100,255);
    triggerPull = false;
  }

  if (triggerCount>=6){ // Shorten the timeout if you're at the max pulls (quick shutdown)
    executionTimer = 100;
    interval = 5;
  }

}

void triggerTick(unsigned long currentMillis){ // Check the timer and run the executioner
  
  if (!digitalRead(1) && currentMillis > lastActionTime+longPullTime){     // Button held for the longPull duration?
    longPull.callback(longPull.param1, longPull.param2, longPull.param3); // Execute longpull callback
  }

  if (currentMillis >= executionTimer+lastActionTime) {     // Check for inactivity timeout
    triggerExecution();
    powerDown();
  }
}
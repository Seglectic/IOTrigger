// ╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
// │                                       Process Triggers and Execute Actions                                        │
// ╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯

void triggerExecution(){
  unsigned long currentTime = millis(); 

  if(!digitalRead(1)){return;}

  if (triggerPull && currentTime > lastPressTime + DEBOUNCE_DELAY && triggerCount<6) { // Check the debounce    
    triggerCount++;
    lastPressTime = currentTime;
    lastActionTime = currentTime;
    buzz(100,255);
    triggerPull = false;
  }

  if (triggerCount>=6){
    executionTimer = 100;
    interval = 5;
  }

}
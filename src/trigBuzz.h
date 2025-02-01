// ╭───────────────────────────────────────────╮
// │  Buzz                                     │
// │  Buzzes the vibrator motor for specified  │
// │  duration at specified strength           │
// ╰───────────────────────────────────────────╯

unsigned long buzzTime       = 0;        // Starting time of a buzz event
bool vibing                  = false;    // Currently buzzin?

void buzz(int buzzDur, int str){
    buzzTime = millis()+buzzDur;
    vibing = true;
    analogWrite(BUZZPIN, str);
}


void buzzTick(){
  if (vibing && (millis()>=buzzTime)){
    analogWrite(BUZZPIN, 0);
    vibing = false;
  }
}


unsigned long buzzTime       = 0;        // Starting time of a buzz event
bool vibing                  = false;    // Currently buzzin?

// Buzzes the motor for specified duration & strength, nonblocking w/ buzzTick
void buzz(int buzzDur, int str){
    buzzTime = millis()+buzzDur;
    vibing = true;
    analogWrite(BUZZPIN, str);
}

// Buzz animation that pulses 3 times to indicate error, blocking
void buzzError(){ 
  digitalWrite(BUZZPIN, HIGH);
  delay(200);
  digitalWrite(BUZZPIN, LOW);
  delay(200);
  digitalWrite(BUZZPIN, HIGH);
  delay(200);
  digitalWrite(BUZZPIN, LOW);
  delay(200);
  digitalWrite(BUZZPIN, HIGH);
  delay(200);
  digitalWrite(BUZZPIN, LOW);
}


void buzzTick(){
  if (vibing && (millis()>=buzzTime)){
    analogWrite(BUZZPIN, 0);
    vibing = false;
  }
}

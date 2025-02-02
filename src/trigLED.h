
int LEDBrightness            = 0;        // Current brightness of the builtin LED

void ledTick(){ //Slowly fades the LED up
  if (LEDBrightness<255){
    analogWrite(LED_BUILTIN,255-LEDBrightness);
    LEDBrightness+=16;
  }
}
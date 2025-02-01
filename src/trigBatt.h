
// Calculates and returns battery voltage
int readBatteryVoltage() {
  int   rawADC  = analogRead(BATTERY_PIN);               // Read ADC value
  float voltage = (rawADC * ADC_REF_VOLTAGE) / ADC_MAX;  // Convert ADC value to voltage
  voltage *= VOLTAGE_DIVIDER_RATIO;                      // Adjust for voltage divider
}
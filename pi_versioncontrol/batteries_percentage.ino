
// for the esp: 

void setup() {
// initialize serial communication at 9600 bits per second:
  Serial.begin(112500);
}

float map_f(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
  float voltage  = (float)analogRead(13);
  voltage = map_f(voltage, 0.0, 4095.0, 0.0, 3.3);
  voltage = (voltage*(12.2/2.2)) + 0.535;
  Serial.println(voltage); // this gives the reading in voltages 
 
}



////////////////////////////////////////////////////////////////////// ^^ 
// for the ardunio nano 

void setup() {
// initialize serial communication at 9600 bits per second:
  Serial.begin(112500);
}

float map_f(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
  float voltage  = (float)analogRead(A1);
  voltage = map_f(voltage, 0.0, 4095.0, 0.0, 3.3);
  voltage = (voltage*(13.4/3.4)) + 0.535;
  Serial.println(voltage);
 
}

////////////////////////////////////////////////////////////////////

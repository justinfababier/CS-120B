/* Author: Justin Fababier (jfaba001@ucr.edu)
 * Discussion Section: 023
 * Assignment: Lab 1
 * Exercise Description: User provides a non-negative integer,
                          program stores value, and outputs binary 
                          representation of input value 
                          using the 4 LEDs.   
 * I acknowledge all content contained herein, 
   excluding template or example code, is my own original work.
 * Demo Link: https://youtu.be/-YeUVcM_6M8
 */

// Define LED array
int ledPin[] = {2, 3, 4, 5};

// Setup
void setup() {
  Serial.begin(9600);
  Serial.print("Enter a value to display on the 4-bit LEDs");
  Serial.print("\n(enter a value 0-15)");
  
  for (int i = 0; i < 4; ++i) {
    pinMode(ledPin[i], OUTPUT);
  }
}

// Loop
void loop() {
  if (Serial.available() > 0) {
    // String is parsed to integer
    int num = Serial.parseInt();
    
    // Calculate bitmask by shifting '1' to the 
    // left 'i' positions, then perform a bitwise AND with 'num'.
    // If the result is non-zero, 
    // set LED pin to HIGH; otherwise, set it to LOW.
    for (int i = 0; i < 4; ++i) {
      digitalWrite(ledPin[i], (num & (1 << i) ? HIGH : LOW));
    }
  }
}

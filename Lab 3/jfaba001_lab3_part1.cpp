/* Author: Justin Fababier (jfaba001@ucr.edu)
 *
 * Discussion Section: 023
 * Assignment: Lab 3 Part 1
 * 
 * Exercise Description:    Implements bathroom light switch using 
 *                          a button and photoresistor. 
 *                          The RGB LED will act as the bathroom light.
 *                          See lab manual for additional conditions.
 * 
 * I acknowledge all content contained herein, 
 * excluding template or example code, is my own original work.
 * 
 * Demo Link: https://youtu.be/mNOyMvHZqVk
 */

#include "Arduino.h"
#include "TimerISR.h"

// Define pin constants
const int DISP_PIN = A1;    // 4D7S display input
const int PHOTO_PIN = A4;   // Photoresistor input
const int POT_PIN = A5;     // Potentiometer input
const int BUTTON_PIN = 5;   // Button input
const int RED_PIN = 11;     // RGB led red output
const int BLUE_PIN = 10;    // RGB led blue output
const int GREEN_PIN = 9;    // RGB led green output

// Pin mode enum
enum PinMode {
    PIN_MODE_INPUT = INPUT,
    PIN_MODE_OUTPUT = OUTPUT
};

// Constants for threshold and timer
const int DARK_THRESHOLD = 150;
const unsigned char TIMER_DURATION = 10;
unsigned char timerCounter = 0;

// 7-segment display pins
const int SEG_PINS[] = {8, 2, 4, 12, 13, 7, 6};
const int NUMS[11][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 1, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}, // 9
    {0, 0, 0, 0, 0, 0, 0}  // off
};


// Function to output the integer x on the first digit of the 4-digit 7-segment display
void outputNum(int x) {
    for (int i = 0; i < 7; ++i) {
        digitalWrite(SEG_PINS[i], NUMS[x][i]);
    }
}

// Function prototypes
void stateMachine();
void initializePins();
void RGB(int, int, int);

// Function to write to RGB LED's pin values
void RGB(int RED_VALUE, int GREEN_VALUE, int BLUE_VALUE) {
    analogWrite(RED_PIN, RED_VALUE);
    analogWrite(GREEN_PIN, GREEN_VALUE);
    analogWrite(BLUE_PIN, BLUE_VALUE);
}

// State machine implementation
enum States {INITIAL, HOLD_ON, HOLD_OFF, PHOTO_RGB, BUTTON_RGB} State;
void stateMachine() {    // State Transitions
    switch(State) { 
        case INITIAL:
            // Check if the button is pressed or if it's dark
            if(digitalRead(BUTTON_PIN) == HIGH) {
                State = HOLD_ON;
            } 
            else if(analogRead(PHOTO_PIN) < DARK_THRESHOLD){
                State = PHOTO_RGB;
            }
            break;

        case HOLD_ON:
            // Check if the button is released
            if(digitalRead(BUTTON_PIN) == LOW) {
                State = BUTTON_RGB;
            }
            break;

        case HOLD_OFF:
            // Check if the button is released
            if(digitalRead(BUTTON_PIN) == LOW) {
                State = INITIAL;
            }
            break;

        case PHOTO_RGB:
            // Check conditions for transitioning to other states
            if(analogRead(PHOTO_PIN) < DARK_THRESHOLD) {
                State = PHOTO_RGB;
                timerCounter = 0;
            } 
            else if(timerCounter >= TIMER_DURATION) {
                State = INITIAL;
                timerCounter = 0;
            } 
            else if(digitalRead(BUTTON_PIN) == HIGH) {
                State = INITIAL;
            }
            break;

        case BUTTON_RGB:
            // Check if the button is pressed or released
            if(digitalRead(BUTTON_PIN) == HIGH) {
                State = INITIAL;
            } 
            else if (digitalRead(BUTTON_PIN) == LOW) {
                State = BUTTON_RGB;
            }
            break;

        default:
            // Error handling for unexpected state
            Serial.println("ERROR");
            State = INITIAL;
            break;
    }

    switch(State) {    // State Actions
        case INITIAL:
            // Turn off LEDs and display "off" on 7-segment display
            RGB(0, 0, 0);   // Off
            // outputNum(10); // Display "off"
            break;
        
        case HOLD_ON:
            // Set LED to blue and turn off 7-segment display
            RGB(0, 0, 255); // Blue
            // outputNum(10); // Display "off"
            break;

        case HOLD_OFF:
            // Turn off LEDs and display "off" on 7-segment display
            RGB(0, 0, 0);   // Off
            // outputNum(10); // Display "off"
            break;

        case PHOTO_RGB:
            // Set LED to white and display timer value on 7-segment display
            RGB(255, 255, 255); // White
            if(timerCounter >= TIMER_DURATION) {
                State = INITIAL;
                timerCounter = 0;
            } 
            else if (timerCounter < TIMER_DURATION) {
               	++timerCounter;
            }
            Serial.println(TIMER_DURATION - timerCounter); // Print timer value
            // outputNum(TIMER_DURATION - timerCounter); // Display timer value
            break;

        case BUTTON_RGB:
            RGB(0, 0, 255); // Blue
            break;

        default:
            // Error handling for unexpected state
            Serial.println("ERROR");
            State = INITIAL;
            break;
    }
}

// Initialize pins
void initializePins() {
    // Initialize pins for 7-segment display
    for (int i = 0; i < 7; ++i) {
        pinMode(SEG_PINS[i], PIN_MODE_OUTPUT);
    }

    // Initialize other pins
    pinMode(DISP_PIN, PIN_MODE_OUTPUT);
    pinMode(PHOTO_PIN, PIN_MODE_INPUT);
    pinMode(POT_PIN, PIN_MODE_INPUT);
    pinMode(BUTTON_PIN, PIN_MODE_INPUT);
    pinMode(RED_PIN, PIN_MODE_OUTPUT);
    pinMode(BLUE_PIN, PIN_MODE_OUTPUT);
    pinMode(GREEN_PIN, PIN_MODE_OUTPUT);
}

// Setup function
void setup() {
    Serial.begin(9600);	// Initialize serial data transmission
    State = INITIAL;    // Set initial state
    initializePins();   // Initialize pins
    TimerSet(1000);     // Set timer interval
    TimerOn();          // Start timer
}

// Loop function
void loop() {
    stateMachine();     // Execute state machine
    while(!TimerFlag) {} // Wait for timer flag
    TimerFlag = 0;      // Reset timer flag
}

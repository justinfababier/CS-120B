/* Author: Justin Fababier (jfaba001@ucr.edu)
 *
 * Discussion Section: 023
 * Assignment: Lab 7 Part 1
 * 
 * I acknowledge all content contained herein, 
 * excluding template or example code, is my own original work.
 * 
 * Demo Link: https://youtu.be/n0jGuAt4iyA
 */

#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "serialATmega.h"

// Variables for cross-task communication
int phases[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};

#define NUM_TASKS 4

// Task struct for concurrent synchSMs implementations
typedef struct _task{
	signed 		char state;			//Task's current state
	unsigned long period;			//Task period
	unsigned long elapsedTime;		//Time elapsed since last task tick
	int (*TickFct)(int);			//Task tick function
} task;

// Periods for each task
const unsigned long GCD_PERIOD = 500;		// Greatest Common Divisor for task periods
const unsigned long SM1_PERIOD = 500;	// Period of state machine 1
const unsigned long SM2_PERIOD = 1000;  // Period of state machine 2
const unsigned long SM3_PERIOD = 0;		// Period of state machine 3
const unsigned long SM4_PERIOD = 0;		// Period of state machine 4

task tasks[NUM_TASKS]; // declared task array with 5 tasks

// Enums and _Tick functions
enum SM1_States {SM1_INIT, SM1_WAIT, SM1_LTS, SM1_RTS};
int SM2_Tick (int state);
enum SM2_States {SM2_INIT, SM2_BEEP, SM2_WAIT, SM2_HORN, SM2_BEEP_ON, SM2_BEEP_OFF};
int SM2_Tick (int state);
enum SM3_States {SM3_INIT, SM3_WAIT, SM3_FORWARD, SM3_BACKWARD, SM3_CW, SM3_CCW};
int SM3_Tick (int state);
enum S4_States {SM4_INIT, SM4_WAIT, SM4_LEFT, SM4_RIGHT};
int SM4_Tick (int state);

void TimerISR() {
	for (unsigned int i = 0; i < NUM_TASKS; i++) {					// Iterate through each task in the task array
		if (tasks[i].elapsedTime == tasks[i].period) {				// Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state);		// Tick and set the next state for this task
			tasks[i].elapsedTime = 0;								// Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;							// Increment the elapsed time by GCD_PERIOD
	}
}

// State Machine 1 - Turn Signals
int SM1_Tick(int state) {
    static int blinkerCounter = 0;

    switch(state) { // State transitions
        case SM1_INIT:
            blinkerCounter = 0;
            state = SM1_WAIT;
            break;

        case SM1_WAIT:
            if (GetBit(PINC, 3) && GetBit(PINC, 4) == 1) { // No button input
                state = SM1_WAIT;
            }
            if (GetBit(PINC, 3) == 0) { // Left button input
                state = SM1_LTS;
            }
            if (GetBit(PINC, 4) == 0) { // Right button input
                state = SM1_RTS;
            }
            break;

        case SM1_LTS:
            if (GetBit(PINC, 3) == 0) {
                if (blinkerCounter % 4 == 0) {
                    PORTD = SetBit(PORTD, 5, 0);
                    PORTD = SetBit(PORTD, 7, 0);
                    PORTB = SetBit(PORTB, 0, 0);
                }
                if (blinkerCounter % 4 == 1) {
                    PORTD = SetBit(PORTD, 5, 1);
                    PORTD = SetBit(PORTD, 7, 0);
                    PORTB = SetBit(PORTB, 0, 0);
                }
                if (blinkerCounter % 4 == 2) {
                    PORTD = SetBit(PORTD, 5, 1);
                    PORTD = SetBit(PORTD, 7, 1);
                    PORTB = SetBit(PORTB, 0, 0);
                }
                if (blinkerCounter % 4 == 3) {
                    PORTD = SetBit(PORTD, 5, 1);
                    PORTD = SetBit(PORTD, 7, 1);
                    PORTB = SetBit(PORTB, 0, 1);
                }
                state = SM1_LTS;
            }
            else {
                PORTD = SetBit(PORTD, 5, 0);
                PORTD = SetBit(PORTD, 7, 0);
                PORTB = SetBit(PORTB, 0, 0);
                blinkerCounter = 0;
                state = SM1_WAIT;
            }
            break;

        case SM1_RTS:
            if (GetBit(PINC, 4) == 0) {
                if (blinkerCounter % 4 == 0) {
                    PORTD = SetBit(PORTD, 4, 0);
                    PORTD = SetBit(PORTD, 3, 0);
                    PORTD = SetBit(PORTD, 2, 0);
                }
                if (blinkerCounter % 4 == 1) {
                    PORTD = SetBit(PORTD, 4, 1);
                    PORTD = SetBit(PORTD, 3, 0);
                    PORTD = SetBit(PORTD, 2, 0);
                }
                if (blinkerCounter % 4 == 2) {
                    PORTD = SetBit(PORTD, 4, 1);
                    PORTD = SetBit(PORTD, 3, 1);
                    PORTD = SetBit(PORTD, 2, 0);
                }
                if (blinkerCounter % 4 == 3) {
                    PORTD = SetBit(PORTD, 4, 1);
                    PORTD = SetBit(PORTD, 3, 1);
                    PORTD = SetBit(PORTD, 2, 1);
                }
                state = SM1_RTS;
            }
            else {
                PORTD = SetBit(PORTD, 4, 0);
                PORTD = SetBit(PORTD, 3, 0);
                PORTD = SetBit(PORTD, 2, 0);
                blinkerCounter = 0;
                state = SM1_WAIT;
            }
            break;

        default:
            break;
    } // State transitions end

    switch(state) { // State actions
        case SM1_INIT:
            break;

        case SM1_WAIT:
            break;
        
        case SM1_LTS:
            blinkerCounter++;
            break;

        case SM1_RTS:
            blinkerCounter++;
            break;

        default:
            break;
    } // State actions end

    return state;
}

// State Machine 2 - Horn & Beeper
int SM2_Tick(int state) {
    static int beeperCounter = 0;
    static int beepDurationCounter = 0;
    TCCR0A |= (1 << COM0A1);    // Use Channel A
    TCCR0A |= (1 << WGM01) | (1 << WGM00);  // Set fast PWM Mode

    switch(state) { // State transitions
        case SM2_INIT:
            state = SM2_WAIT;
            break;

        case SM2_WAIT:
            if (GetBit(PINC, 2) == 0) {
                state = SM2_HORN;
            }
            else if (ADC_read(0) <= 384) {
                beepDurationCounter = 0;
                beeperCounter = 0;
                state = SM2_BEEP_ON;
            }
            else {
                state = SM2_WAIT;
            }
            break;

        case SM2_HORN:
            if (GetBit(PINC, 2) == 0) {
                state = SM2_HORN;
            }
            else {
                state = SM2_WAIT;
            }
            break;

        case SM2_BEEP_ON:
            if (beepDurationCounter < 1) { // Beep for 1 second
                beepDurationCounter++;
            }
            else {
                beeperCounter = 0;
                state = SM2_BEEP_OFF;
            }
            break;

        case SM2_BEEP_OFF:
            if (beeperCounter < 1) { // Wait for 1 second
                beeperCounter++;
            }
            else {
                if (ADC_read(0) <= 384) {
                    beepDurationCounter = 0;
                    state = SM2_BEEP_ON;
                }
                else {
                    state = SM2_WAIT;
                }
            }
            break;

        default:
            state = SM2_INIT;
            break;
    } // State transitions end

    switch(state) { // State actions
        case SM2_INIT:
            OCR0A = 255; // Ensure the output is off initially
            break;

        case SM2_WAIT:
            OCR0A = 255; // Ensure the output is off in wait state
            break;

        case SM2_HORN:
            OCR0A = 128; // Set duty cycle to 50% for the horn
            TCCR0B = (TCCR0B & 0xF8) | 0x04; // Set prescaler to 256
            break;

        case SM2_BEEP_ON:
            OCR0A = 128; // Set duty cycle to 50% for the beeper
            TCCR0B = (TCCR0B & 0xF8) | 0x03; // Set prescaler to 64
            break;

        case SM2_BEEP_OFF:
            OCR0A = 255; // Turn off the beeper
            break;

        default:
            break;
    } // State actions end

    return state;
}

// State Machine 3 - Step Motor
int SM3_Tick(int state) {
    static unsigned int motorCnt = 0;
    static int phase = 0;
    unsigned int speed1 = map_value(640, 1024, 0, 15, ADC_read(0));
    unsigned int speed2 = map_value(0, 384, 0, 15, ADC_read(0));

    switch(state) { // State transitions
        case SM3_INIT:
            state = SM3_WAIT;
            break;

        case SM3_WAIT:
            if (ADC_read(0) >= 640) {
                state = SM3_CW;
            } 
            else if (ADC_read(0) <= 328) {
                state = SM3_CCW;
            }
            break;

        case SM3_CW:
            if (ADC_read(0) < 640) {
                state = SM3_WAIT;
            }
            break;

        case SM3_CCW:
            if (ADC_read(0) > 384) {
                state = SM3_WAIT;
            }
            break;

        default:
            state = SM3_INIT;
            break;
    } // State transitions end

    switch(state) { // State actions
        case SM3_INIT:
            break;

        case SM3_WAIT:
            
            break;

        case SM3_CW:
            motorCnt++;
            if (motorCnt >= 15 - speed1) {
                PORTB = (PORTB & 0x03) | (phases[phase] << 2);
                phase = (phase + 1) % 8;
                motorCnt = 0;
            }
            break;

        case SM3_CCW:
            motorCnt++;
            if (motorCnt >= speed2) {
                PORTB = (PORTB & 0x03) | (phases[phase] << 2);
                phase = (phase - 1 + 8) % 8;
                motorCnt = 0;
            }
            break;

        default:
            break;
    } // State actions end

    return state;
}

// State Machine 4 - Micro Servo
int SM4_Tick(int state) {
    TCCR1A |= (1 << WGM11) | (1 << COM1A1); //COM1A1 sets it to channel A
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); //CS11 sets the prescaler to be 8
    //WGM11, WGM12, WGM13 set timer to fast pwm mode
    ICR1 = 39999;   // 20ms pwm period

    switch(state) { // State transitions
        case SM4_INIT:
            state = SM4_WAIT;
            break;

        case SM4_WAIT:
            if (ADC_read(1) >= 648) {
                state = SM4_LEFT;
            }
            else if (ADC_read(1) <= 380) {
                state = SM4_RIGHT;
            }
            else {
                OCR1A = 2999;
                state = SM4_WAIT;
            }
            break;

        case SM4_LEFT:
            if (ADC_read(1) >= 648) {
                state = SM4_LEFT;
            }
            else {
                state = SM4_WAIT;
            }
            break;

        case SM4_RIGHT:
            if (ADC_read(1) <= 380) {
                state = SM4_RIGHT;
            }
            else {
                state = SM4_WAIT;
            }
            break;

        default:
            break;
    } // State transitions end

    switch(state) { // State actions
        case SM4_INIT:
            break;

        case SM4_WAIT:
            break;

        case SM4_LEFT:
            OCR1A = map_value(648, 1024, 2999, 4999, ADC_read(1));
            break;

        case SM4_RIGHT:
            OCR1A = map_value(0, 380, 999, 2999, ADC_read(1));
            break;

        default:
            break;
    } // State actions end

    return state;
}

int main(void) {
	// Initialize inputs and outputs
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0x00; PORTC = 0xFF;
	DDRD = 0xFF; PORTD = 0x00;

	ADC_init();         // initializes ADC
	serial_init(9600);  // initializes serial

	// Task 1
	unsigned char i = 0;                      // Index for task array (tasks[0])
	tasks[i].state = SM1_INIT;                // Set the initial state of the first task (SM1)
	tasks[i].period = SM1_PERIOD;             // Set the period of the first task
	tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
	tasks[i].TickFct = &SM1_Tick;             // Assign the tick function for the first task

	// Task 2
	i++;                                      // Move to the next task index (tasks[2])
	tasks[i].state = SM2_INIT;                // Set the initial state of the second task (SM2)
	tasks[i].period = SM2_PERIOD;             // Set the period of the second task
	tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
	tasks[i].TickFct = &SM2_Tick;             // Assign the tick function for the second task

	// Task 3
	i++;                                      // Move to the next task index (tasks[2])
	tasks[i].state = SM3_INIT;                // Set the initial state of the third task (SM3)
	tasks[i].period = SM3_PERIOD;             // Set the period of the third task
	tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
	tasks[i].TickFct = &SM3_Tick;             // Assign the tick function for the third task

	// Task 4
	i++;                                      // Move to the next task index (tasks[3])
	tasks[i].state = SM4_INIT;                // Set the initial state of the fourth task (SM4)
	tasks[i].period = SM4_PERIOD;             // Set the period of the fourth task
	tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
	tasks[i].TickFct = &SM4_Tick;             // Assign the tick function for the fourth task

	TimerSet(GCD_PERIOD);
	TimerOn();

	while (1) {}

	return 0;
}

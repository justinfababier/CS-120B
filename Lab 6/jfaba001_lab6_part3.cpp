/* Author: Justin Fababier (jfaba001@ucr.edu)
 *
 * Discussion Section: 023
 * Assignment: Lab 6 Part 3
 * 
 * I acknowledge all content contained herein, 
 * excluding template or example code, is my own original work.
 * 
 * Demo Link: https://youtu.be/u_yWh_moLo0
 */

#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "LCD.h"
#include "serialATmega.h"
#include <stdio.h>

// Variables for cross-task communication
unsigned int MODE;  // Global flag: 0 for MANUAL mode, 1 for AUTO mode
unsigned int i, j, H, L;
int percentage, measuredTemp;
double tempK;
int thresholdTemp = 78;
int tempDiff = measuredTemp - thresholdTemp;

/* You have 5 tasks to implement for this lab */
#define NUM_TASKS 5

// Task struct for concurrent synchSMs implementations
typedef struct _task{
	signed 		char state;			//Task's current state
	unsigned long period;			//Task period
	unsigned long elapsedTime;		//Time elapsed since last task tick
	int (*TickFct)(int);			//Task tick function
} task;

// Periods for each task
const unsigned long GCD_PERIOD = 1;     // Greatest Common Divisor for task periods
const unsigned long SM1_PERIOD = 100;   // Period of state machine 1
const unsigned long SM2_PERIOD = 100;   // Period of state machine 2
const unsigned long SM3_PERIOD = 1;     // Period of state machine 3
const unsigned long SM4_PERIOD = 1;     // Period of state machine 4
const unsigned long SM5_PERIOD = 100;   // Period of state machine 5

task tasks[NUM_TASKS]; // declared task array with 5 tasks

// Enums and _Tick functions
enum SM1_States {SM1_INIT, SM1_S0, SM1_S1, SM1_S2, SM1_S3};
int SM2_Tick (int state);
enum SM2_States {SM2_INIT, SM2_S0, SM2_S1, SM2_S2, SM2_S3, SM2_S4};
int SM2_Tick (int state);
enum SM3_States {SM3_INIT, SM3_S0, SM3_S1, SM3_S2, SM3_S3, SM3_S4};
int SM3_Tick (int state);
enum S4_States {SM4_INIT, SM4_S0, SM4_S1, SM4_S2, SM4_S3, SM4_S4, SM4_S5};
int SM4_Tick (int state);
enum S5_States {SM5_INIT, SM5_S0, SM5_S1, SM5_S2, SM5_S3};
int SM5_Tick (int state);

void TimerISR() {
	for (unsigned int i = 0; i < NUM_TASKS; i++) {					// Iterate through each task in the task array
		if (tasks[i].elapsedTime == tasks[i].period) {				// Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state);		// Tick and set the next state for this task
			tasks[i].elapsedTime = 0;								// Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;							// Increment the elapsed time by GCD_PERIOD
	}
}

// Temperature control function
void updateTemperatureControl() {
    int tempReading = ADC_read(0);

    // Calculate temperature in Kelvin
    tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
    tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK)) * tempK);

    // Convert Kelvin to Fahrenheit
    measuredTemp = (tempK - 273.15) * 9 / 5 + 32;
    tempDiff = measuredTemp - thresholdTemp;
}

// State Machine 1
int SM1_Tick (int state) {
    char line0[20], line1[16], line2[16];
    switch (state) {    // State transitions
        case SM1_INIT:
            lcd_clear();
            state = SM1_S0;
            break;
        
        case SM1_S0:
            if (GetBit(PINC, 2) == 1) {
                state = SM1_S0;
            }
            else if (GetBit(PINC, 2) == 0) {
                state = SM1_S1;
                lcd_clear();
            }
            break;

        case SM1_S1:
            if (GetBit(PINC, 2) == 1) {
                state = SM1_S2;
            }
            break;

        case SM1_S2:
            if (GetBit(PINC, 2) == 1) {
                state = SM1_S2;
            }
            else if (GetBit(PINC, 2) == 0) {
                state = SM1_S3;
                lcd_clear();
            }
            break;

        case SM1_S3:
            if (GetBit(PINC, 2) == 1) {
                state = SM1_S0;
            }
            break;

        default:
            break;
    }   // State transitions end

    switch (state) {    // State actions
        case SM1_INIT:
            break;
        
        case SM1_S0:
            MODE = 0; // Set mode to MANUAL
            percentage = H * 10; // Update percentage value based on H
            lcd_goto_xy(0, 0);
            lcd_write_str("Mode: MANUAL");
            lcd_goto_xy(1, 0);
            sprintf(line0, "FAN: %d%%   ", percentage);
            lcd_write_str(line0);
            break;

        case SM1_S1:
            break;

        case SM1_S2:
            MODE = 1; // Set mode to AUTO
            lcd_goto_xy(0, 0);
            sprintf(line1, "Mode: AUTO");
            lcd_write_str(line1);
            lcd_goto_xy(1, 0);
            sprintf(line2, "Curr:%dF ", measuredTemp);
            lcd_write_str(line2);
            sprintf(line2, "Th:%dF", thresholdTemp);
            lcd_write_str(line2);
            break;

        case SM1_S3:
            break;

        default:
            break;
    }   // State actions end

    return state;
}

int SM2_Tick (int state) {
    switch (state) {    // State transitions
        case SM2_INIT:
            j = 0;
            state = SM2_S0;
            break;
        
        case SM2_S0:
            PORTB = SetBit(PORTB, 0, 0);
            if (GetBit(PINC, 2) == 0) {
                state = SM2_S1;
            }
            if (ADC_read(1) > 768) {
                state = SM2_S2;
            }
            else if (ADC_read(1) < 256) {
                state = SM2_S3;
            }
            break;

        case SM2_S1:
            if (GetBit(PINC, 2) == 1) {
                state = SM2_S4;
            }
            break;

        case SM2_S2:
            if (ADC_read(1) <= 768) {
                state = SM2_S4;
            }
            break;

        case SM2_S3:
            if (ADC_read(1) >= 256) {
                state = SM2_S4;
            }
            break;

        case SM2_S4:
            if (j < 1) {
                state = SM2_S4;
            }
            state = SM2_S0;
            break;

        default:
            break;
    }   // State transitions end

    switch (state) {    // State actions
        case SM2_INIT:
            break;
        
        case SM2_S0:
            break;

        case SM2_S1:
            break;

        case SM2_S2:
            break;

        case SM2_S3:
            break;

        case SM2_S4:
            PORTB = SetBit(PORTB, 0, 1);
            serial_println("beep");
            j++;
            break;

        default:
            break;
    }   // State actions end

    return state;
}

int SM3_Tick(int state) {
    switch (state) {    // State transitions
        case SM3_INIT:
            i = 0;
            H = 0;
            L = 10;
            state = SM3_S0;
            break;
        
        case SM3_S0:
            if (MODE != 0) {
                return state;
            }
            if (ADC_read(1) > 768) {
                if ((H == 10) && (L == 0)) {
                    state = SM3_S0;
                }
                else {
                    state = SM3_S1;
                }
            }
            else if (ADC_read(1) < 256) {
                if ((H == 0) && (L == 10)) {
                    state = SM3_S0;
                }
                else {
                    state = SM3_S4;
                }
            }
            else {
                state = SM3_S0;
            }                                                                                                          
            break;

        case SM3_S1:
            if (ADC_read(1) <= 768) {
                state = SM3_S2;
                H++;
                L--;
            }
            break;

        case SM3_S2:
            if (i < H) {
                PORTB = SetBit(PORTB, 2, 1);
                state = SM3_S2;
            }
            else {
                i = 0;
                state = SM3_S3;
            }
            break;

        case SM3_S3:
            if (i < L) {
                PORTB = SetBit(PORTB, 2, 0);
                state = SM3_S3;
            }
            else {
                i = 0;
                state = SM3_S0;
            }
            break;

        case SM3_S4:
            if (ADC_read(1) >= 256) {
                state = SM3_S2;
                H--;
                L++;
            }
            break;

        default:
            break;
    }   // State transitions end

    switch (state) {    // State actions
        case SM3_INIT:
            PORTB = SetBit(PORTB, 2, 0);
            break;
        
        case SM3_S0:
            break;

        case SM3_S1:
            break;

        case SM3_S2:
            i++;
            break;

        case SM3_S3:
            i++;
            break;

        case SM3_S4:
            break;

        default:
            break;
    }   // State actions end

    return state;
}

int SM4_Tick(int state) {
    switch (state) {    // State transitions
        case SM4_INIT:
            i = 0;
            H = 0;
            L = 10;
            state = SM4_S0;
            break;
        
        case SM4_S0:
            if (MODE != 1) {
                return state;
            }
            updateTemperatureControl();
            if (measuredTemp <= thresholdTemp) {
                H = 0;
                L = 10;
                state = SM4_S2;
                break;
            }
            else if (measuredTemp - thresholdTemp >= 10) {
                H = 10;
                L = 0;
            }
            else {
                H = tempDiff;
                L = 10 - tempDiff;
            }
            state = SM4_S1;
            break;

        case SM4_S1:
            if (i < H) {
                state = SM4_S1;
            }
            else {
                i = 0;
                state = SM4_S2;
            }
            break;

        case SM4_S2:
            if (i < L) {
                state = SM4_S2;
            }
            else {
                i = 0;
                state = SM4_S0;
            }
            break;
        
        default:
            break;
    }   // State transitions end

    switch (state) {    // State actions
        case SM4_INIT:
            break;
        
        case SM4_S0:
            break;

        case SM4_S1:
            PORTB = SetBit(PORTB, 2, 1);
            ++i;
            break;

        case SM4_S2:
            PORTB = SetBit(PORTB, 2, 0);
            ++i;
            break;

        default:
            break;
    }   // State actions end

    return state;
}

int SM5_Tick(int state) {
    switch (state) {    // State transitions
        case SM5_INIT:
            state = SM5_S0;
            break;
        
        case SM5_S0:
            if (MODE != 1) {
                return state;
            }
            if (ADC_read(1) > 768) {
                state = SM5_S1;
                break;
            }
            if (ADC_read(1) < 256) {
                state = SM5_S2;
                break;
            }
            break;

        case SM5_S1:
            if (ADC_read(1) <= 768) {
                thresholdTemp++;
            }
            state = SM5_S0;
            break;

        case SM5_S2:
            if (ADC_read(1) >= 256) {
                thresholdTemp--;
            }
            state = SM5_S0;
            break;

        default:
            break;
    }   // State transitions end

    switch (state) {    // State actions
        case SM5_INIT:
            break;
        
        case SM5_S0:
            break;

        case SM5_S1:
            break;

        case SM5_S2:
            break;
        
        case SM5_S3:
            break;

        default:
            break;
    }   // State actions end

    return state;
}

int main(void) {
	// Initialize inputs and outputs
    DDRB = 0x07; PORTB = 0x00;
    DDRC = 0xF8; PORTC = 0xFF;

	ADC_init();		    // initializes ADC
    lcd_init();         // initializes LCD
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

    // Task 5
    i++;                                      // Move to the next task index (tasks[4])
    tasks[i].state = SM5_INIT;                // Set the initial state of the fifth task (SM5)
    tasks[i].period = SM5_PERIOD;             // Set the period of the fifth task
    tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
    tasks[i].TickFct = &SM5_Tick;             // Assign the tick function for the fifth task

	TimerSet(GCD_PERIOD);
	TimerOn();

	while (1) {}

	return 0;
}

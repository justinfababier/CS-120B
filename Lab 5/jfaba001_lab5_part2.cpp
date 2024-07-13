/* Author: Justin Fababier (jfaba001@ucr.edu)
 *
 * Discussion Section: 023
 * Assignment: Lab 5 Part 2
 * 
 * Exercise Description:    The purpose of this exercise is to implement tasks 3 and task 4 of lab 5's syncSM.
 *                          Tasks 3 and 4 implement the RGB LED's red and green channel via pulse width modulation.
 *                          Depending on the distance read by the ultrasonic sensor, 
 *                          the red and green channels' PWM signals will change behavior.
 * 
 * I acknowledge all content contained herein, 
 * excluding template or example code, is my own original work.
 * 
 * Demo Link: https://youtu.be/x6ZiyD2Jl48
 */

#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "serialATmega.h"

// Variables for cross-task communication
int distance;

#define NUM_TASKS 4

//Task struct for concurrent synchSMs implmentations
typedef struct _task {
    signed char state;              // Task's current state
    unsigned long period;           // Task period
    unsigned long elapsedTime;      // Time elapsed since last task tick
    int (*TickFct)(int);            // Task tick function
} task;

const unsigned long GCD_PERIOD = 1;     // Greatest Common Divisor for task periods, used for timer configuration
const unsigned long SM1_PERIOD = 1000;  // Period of state machine 1
const unsigned long SM2_PERIOD = 1;     // Period of state machine 2
const unsigned long SM3_PERIOD = 1;     // Period of state machine 3
const unsigned long SM4_PERIOD = 1;     // Period of state machine 4
const unsigned long SM5_PERIOD = 0;     // Period of state machine 5, zero if not used

task tasks[NUM_TASKS]; // Declared task array with 5 tasks

enum SM1_States {SM1_INIT, SM1_S0, SM1_S1, SM1_S2};
int SM1_Tick (int state);

enum SM2_States {SM2_INIT, SM2_S0, SM2_S1, SM2_S2, SM2_S3};
int SM2_Tick(int state);

enum SM3_States {SM3_INIT, SM3_S0, SM3_S1};
int SM3_Tick(int state);

enum S4_States {SM4_INIT, SM4_S0, SM4_S1};
int SM4_Tick(int state);

enum S5_States {SM5_INIT, SM5_S0, SM5_S1};
int SM5_Tick(int state);

void TimerISR() {

    for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {            // Iterate through each task in the task array
        if ( tasks[i].elapsedTime == tasks[i].period ) {        // Check if the task is ready to tick
            tasks[i].state = tasks[i].TickFct(tasks[i].state);  // Tick and set the next state for this task
            tasks[i].elapsedTime = 0;                           // Reset the elapsed time for the next tick
        }
        tasks[i].elapsedTime += GCD_PERIOD;                     // Increment the elapsed time by GCD_PERIOD
    }
}

int main(void) {

    DDRB = 0xFE;    // Set PB1 as output, others as input
    PORTB = 0x01;   // Initialize PORTB to ensure all inputs are low
    DDRC = 0xFC;    // Set only PC2 as output, others as input
    PORTC = 0x03;   // Initialize PORTC to ensure all outputs are low
    DDRD = 0xFF;    // Set all pins of PORTD as outputs
    PORTD = 0x00;         // Initialize PORTD to ensure all outputs are low

    // Initialize task 1
    unsigned char i = 0;                      // Index for task array (tasks[0])
    tasks[i].state = SM1_INIT;                // Set the initial state of the first task (SM1)
    tasks[i].period = SM1_PERIOD;             // Set the period of the first task
    tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
    tasks[i].TickFct = &SM1_Tick;             // Assign the tick function for the first task

    // Initialize task 2
    i++;                                      // Move to the next task index (tasks[2])
    tasks[i].state = SM2_INIT;                // Set the initial state of the second task (SM2)
    tasks[i].period = SM2_PERIOD;             // Set the period of the second task
    tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
    tasks[i].TickFct = &SM2_Tick;             // Assign the tick function for the second task

    // Initialize task 3
    i++;                                      // Move to the next task index (tasks[2])
    tasks[i].state = SM3_INIT;                // Set the initial state of the third task (SM3)
    tasks[i].period = SM3_PERIOD;             // Set the period of the third task
    tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
    tasks[i].TickFct = &SM3_Tick;             // Assign the tick function for the third task

    // Initialize task 4
    i++;                                      // Move to the next task index (tasks[3])
    tasks[i].state = SM4_INIT;                // Set the initial state of the fourth task (SM4)
    tasks[i].period = SM4_PERIOD;             // Set the period of the fourth task
    tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
    tasks[i].TickFct = &SM4_Tick;             // Assign the tick function for the fourth task

    // Initialize task 5
    i++;                                      // Move to the next task index (tasks[4])
    tasks[i].state = SM5_INIT;                // Set the initial state of the fifth task (SM5)
    tasks[i].period = SM5_PERIOD;             // Set the period of the fifth task
    tasks[i].elapsedTime = tasks[i].period;   // Initialize elapsedTime to the period for the timer
    tasks[i].TickFct = &SM5_Tick;             // Assign the tick function for the fifth task

    TimerSet(GCD_PERIOD);
    TimerOn();

    serial_init(9600);
    ADC_init();
    init_sonar();   

    while (1) {}

    return 0;
}

int SM1_Tick(int state) {
    // State transitions
    switch (state) {    
        case SM1_INIT:  // If the state is the initial state
            state = SM1_S0; // Transition to state SM1_S0
            break;

        case SM1_S0:    // If the state is SM1_S0
            state = SM1_S0; // Transition to state SM1_S1
            break;

        default:    // Default case to handle unexpected states
            break;
    } // End of transitions block

    // State actions
    switch (state) {    
        case SM1_INIT:  // Actions for the initial state
            break;

        case SM1_S0:    // Actions for state SM1_S0
            distance = read_sonar();
            serial_println(distance);
            break;

        default:    // Default case to handle unexpected states
            break;
    } // End of actions block

    return state; // Return the new state
}

int SM2_Tick(int state) {
    // State transitions
    switch (state) {
        case SM2_INIT:  // Initial state
            state = SM2_S0;
            break;

        case SM2_S0:    // D1
            state = SM2_S1;
            break;

        case SM2_S1:    // D2
            state = SM2_S2;
            break;

        case SM2_S2:    // D3
            state = SM2_S3;
            break;

        case SM2_S3:    // D4
            state = SM2_S0;
            break;

        default:
            break;
    } // End of transitions block

    // State actions
    switch (state) {
        case SM2_INIT:  // Initial state
            break;

        case SM2_S0:
            PORTB = SetBit(PORTB, 5, 1);
            outNum(distance % 10);
            PORTB = SetBit(PORTB, 2, 0);
            break;

        case SM2_S1:
            PORTB = SetBit(PORTB, 2, 1);
            outNum((distance / 10) % 10);
            PORTB = SetBit(PORTB, 3, 0);
            break;

        case SM2_S2:
            PORTB = SetBit(PORTB, 3, 1);
            outNum((distance / 100) % 10);
            PORTB = SetBit(PORTB, 4, 0);
            break;

        case SM2_S3:
            PORTB = SetBit(PORTB, 4, 1);
            outNum((distance / 1000) % 10);
            PORTB = SetBit(PORTB, 5, 0);
            break;

        default:
            break;
    } // End of actions block

    return state;
}

int SM3_Tick(int state) {
    // Local variables
    unsigned cnt = 0;
    unsigned H;

    // State transitions
    switch (state) {    // Red - Task Period: 1 ms; PWM period: 10 ms

        case SM3_INIT:  // Initial state
            cnt = 0;
            state = SM3_S0;
            break;

        case SM3_S0:
            if (distance < 10) {
                H = 10;
            }
            else if ((distance >= 10) && (distance < 20)) {
                H = 9;
            }
            else if (distance >= 20) {
                H = 0;
            }
            PORTC = SetBit(PORTC, 5, (cnt < H));
            break;

        default:
            break;
    } // End of transitions block

    // State actions
    switch (state) {

        case SM3_INIT:                  // Initial state
            PORTC = SetBit(PORTC, 5, (cnt < H));
            break;

        case SM3_S0:
            cnt++;
            break;

        default:
            break;
    } // End of actions block

    return state;
}

int SM4_Tick(int state) {
    // Local variables
    unsigned cnt = 0;
    unsigned H;

    // State transitions
    switch (state) {    // Red - Task Period: 1 ms; PWM period: 10 ms

        case SM4_INIT:  // Initial state
            cnt = 0;
            state = SM4_S0;
            break;

        case SM4_S0:
            if (distance < 10) {
                H = 0;
            }
            else if ((distance >= 10) && (distance < 20)) {
                H = 3;
            }
            else if (distance >= 20) {
                H = 10;
            }
            PORTC = SetBit(PORTC, 4, (cnt < H));
            break;

        default:
            break;
    } // End of transitions block

    // State actions
    switch (state) {

        case SM4_INIT:  // Initial state
            PORTC = SetBit(PORTC, 4, (cnt < H));
            break;

        case SM4_S0:
            cnt++;
            break;

        default:
            break;
    } // End of actions block

    return state;
}

int SM5_Tick(int state) {
    // State transitions
    switch (state) {
        case SM5_INIT:  // Initial state
            state = SM5_S0;
            break;

        case SM5_S0:    // Joystick up
            state = SM5_S0;
            break;

        default:
            break;
    } // End of transitions block

    // State actions
    switch (state) {
        case SM5_INIT:  // Initial state
            state = SM5_S0;
            break;

        case SM5_S0:    // Joystick up
            state = SM5_S0;
            break;

        default:
            break;
    } // End of actions block

    return state;
}
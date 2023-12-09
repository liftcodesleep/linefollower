/**************************************************************
1 second turn = 180
.5 second turn = 90?


 **************************************************************/

#include <pigpio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "assignment2.h"

#define TRIG2 5 // SIDE
#define ECHO2 6
#define TRIG1 17 // FRONT
#define ECHO1 27

volatile uint32_t pulse_tick1, pulse_tick2;
int previous = 0;
int previous1 = 0, previous2 = 0;

int local_sleep(int microseconds)
{
    if (gpioSleep(PI_TIME_RELATIVE, microseconds, 0))
    {
        printf("Failed gpioSleep! Exiting.\n");
        return 1;
    }
    return 0;
}

int local_write(int pin, int pin_state)
{
    if (gpioWrite(pin, pin_state))
    {
        printf("Failed gpioWrite! Exiting.\n");
        return 1;
    }
    return 0;
}

// Rewrite this so we can only use one sonar at a time - Change sonar when attempting to naviate obstacle
// int pulse(void)
// {
//     if (local_write(TRIG1, PI_OFF))
//     {
//         return 1;
//     };
//     gpioDelay(5);
//     if (local_write(TRIG1, PI_ON))
//     { // send pulse
//         return 1;
//     };
//     gpioDelay(10); // Pulse must be at least 10 microseconds
//     if (local_write(TRIG1, PI_OFF))
//     {
//         return 1;
//     };
//     gpioDelay(5);
//     pulse_tick1 = gpioTick(); // get time of pulse for future reference
//     return 0;
// }

int pulse(int pin)
{
    if (local_write(pin, PI_OFF))
    {
        return 1;
    }

    gpioDelay(5);
    if (local_write(pin, PI_ON))
    { // send pulse
        return 1;
    }

    gpioDelay(10); // Pulse must be at least 10 microseconds
    if (local_write(pin, PI_OFF))
    {
        return 1;
    }

    gpioDelay(5);
    pulse_tick1 = gpioTick(); // get time of pulse for future reference
    return 0;
}

int median(int a, int b, int c)
{
    if ((a <= b && b <= c) || (c <= b && b <= a))
    {
        return b;
    }
    else if ((b <= a && a <= c) || (c <= a && a <= b))
    {
        return a;
    }
    else
    {
        return c;
    }
}

void get_distance(int pin, int pin_state, uint32_t time)
{
    static uint32_t begin, end;
    uint32_t distance;
    if (time > pulse_tick1)
    { // don't measure the sensor firing
        /*
          When ECHO pin state changes to HIGH, mark the time
          When ECHO pin state changes to LOW, measure the distance using the delta
        */
        if (pin_state == PI_ON)
        {
            begin = time; // mark timestamp of ECHO firing
        }
        else if (pin_state == PI_OFF)
        {
            end = time;                    // mark timestamp of ECHO receiving
            distance = (end - begin) / 58; // convert to cm by dividing delta by 58
            previous = distance;        // record distance
            printf("DISTANCE DETECT FROM PIN %d: %d\n", pin, previous); // Debugging print statement
            if (distance >= 420)
            { // HC-SR04 has a range of 2 to 400 cm
                printf("Distance out of range\n");
                previous = 0;
            }
        }
    }
}

// void obstacleAvoidance(int argc, char *argv[])
int obstacleAvoidance(void)
{
    printf("INSIDE SONARDEMO\n");
    // init GPIO
    if (gpioInitialise() < 0)
    {
        printf("Failed to initialize GPIO! Exiting.\n");
        return 1;
    }

    // TRIG and ECHO pins for Sensor 1
    if (gpioSetMode(TRIG1, PI_OUTPUT) || gpioSetMode(ECHO1, PI_INPUT))
    {
        printf("Failed to set TRIG1 or ECHO1 pins! Exiting.\n");
        return 1;
    }

    // TRIG and ECHO pins for Sensor 2
    if (gpioSetMode(TRIG2, PI_OUTPUT) || gpioSetMode(ECHO2, PI_INPUT))
    {
        printf("Failed to set TRIG2 or ECHO2 pins! Exiting.\n");
        return 1;
    }

    // take measurements as ECHO flips
    if (gpioSetAlertFunc(ECHO1, get_distance))
    {
        printf("Failed to set alert function for Sensor 1! Exiting.\n");
        return 1;
    }

    // take measurements as ECHO flips
    if (gpioSetAlertFunc(ECHO2, get_distance))
    {
        printf("Failed to set alert function for Sensor 2! Exiting.\n");
        return 1;
    }

    // Goal: Refine the obstacle avoidance function so that it is not a predetermined 4x4 turn.
    // We need to handle an unorthodox container or situation.
    while (1)
    {
        stop();

        if (pulse(TRIG1))
        {
            break;
        }
        if (local_sleep(1))
        {
            break;
        }

        if (previous > 0)
        {
            printf("INSIDE PREVIOUS LOOP\n");
            if (previous < 50)
            {
                if (previous < 20)
                { // if it gets too close // if it gets too close
                    printf("Too close! Reverse reverse!\n");

                    // Brief pause
                    stop();
                    // usleep(1000000);
                    sleep(2);
                    accelerate_backward();
                    // usleep(1000000);
                    sleep(1);
                    stop();
                    // Pause to measure the distance after a one second reverse
                    printf("After reverse - FWD ");
                }

                printf("Initial obstacle detection!\n");
                stop();
                turn_left();
                // usleep(100000); // figure out a proper time for this
                sleep(1); // Almost 90 degree turn?
                // may need a stop here - we do for testing
                stop();
                sleep(5);
                printf("Here we go!\n");

                // Part of original code - keep for reference while working on new loop
                // accelerate_forward();
                // usleep(1000000);
                // sleep(1);
                // stop(); // again

                // This block is to repeat for the remaining turns to navigate around the object. We are assuming that
                // two right turns need to be performed before detecting the line again
                // Currently it is only using the center line sensor as the condition to break the loop
                while (!gpioRead(LINE_CENTER)) // repeat this until the car is past the object - utilizing side sonar only
                {
                    if (pulse(TRIG2)) // Triggering sensor on the right side
                    {
                        break;
                    }

                    if (local_sleep(1))
                    {
                        break;
                    }

                    if (previous < 50)
                    {
                        printf("Crusing along the obstacle\n");
                        accelerate_forward();
                        sleep(1);
                        stop();
                        sleep(5);
                    } else {
                        printf("We believe the car is past the obstacle\n");
                        accelerate_forward();
                        sleep(1);
                        stop();
                        sleep(5);
                        turn_right();
                        sleep(1);
                        stop();
                        sleep(5);
                    }
                }

                // We've found a line
                printf("We found a line!\n");

                turn_left(); // Assuming the line will be continue to the left based on our turns (L,R,R)
                sleep(1);
                stop();
                sleep(5);
                accelerate_forward();
                // usleep(1000000);
                sleep(1);
                stop();
                
                // turn off override and hopefully the line sensor kicks in and realins the car
                // otherwise turn it left
            }
        }
    }

    //gpioTerminate(); // main.c should be responsible for gpioTerminate
    return 0;
}

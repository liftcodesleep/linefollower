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

#define TRIG2 23
#define ECHO2 24
#define TRIG1 17
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

int pulse(void)
{
    if (local_write(TRIG, PI_OFF))
    {
        return 1;
    };
    gpioDelay(5);
    if (local_write(TRIG, PI_ON))
    { // send pulse
        return 1;
    };
    gpioDelay(10); // Pulse must be at least 10 microseconds
    if (local_write(TRIG, PI_OFF))
    {
        return 1;
    };
    gpioDelay(5);
    pulse_tick = gpioTick(); // get time of pulse for future reference
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
    if (time > pulse_tick)
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
            previous = distance;           // record distance
            if (distance >= 420)
            { // HC-SR04 has a range of 2 to 400 cm
                printf("Distance out of range\n");
                previous = 0;
            }
        }
    }
}

void sonarDemo(int argc, char *argv[])
{
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

    while (1)
    {

        if (pulse())
        {
            break;
        }
        if (local_sleep(1))
        {
            break;
        }
        if (previous > 0)
        {
            if (previous < 50)
            {
                if (previous < 10)
                { // if it gets too close // if it gets too close
                    accelerate_backward();
                    usleep(500000);
                }
                stop();
                turn_left();
                usleep(500000); // figure out a proper time for this
                // may need a stop here
                accelerate_forward();
                usleep(500000);
                turn_right();
                usleep(500000); // with this the car should be facing the same direction as the start but to the side a bit

                while (1)
                {
                    // repeat this until the car is past the object
                    stop();
                    accelerate_forward();
                    usleep(1000000);
                    stop();
                    turn_right();
                    usleep(500000);
                    stop();

                    if (pulse())
                    { // check if car is past the obstacle
                        break;
                    }
                    if (local_sleep(1))
                    {
                        break;
                    }
                    if (previous == 0 || previous > 100)
                    { // the car is past the obstacle
                        break;
                    }
                    else
                    {
                        turn_left();
                        usleep(500000);
                        stop();
                    }
                }
                accelerate_forward();
                usleep(1000000);
                stop();
                // turn off override and hopefully the line sensor kicks in and realins the car
                // otherwise turn it left
            }
        }
    }

    gpioTerminate();
    return 0;
}

/**************************************************************
 * Class: CSC-615-01 Fall 2023
 * Name: Jacob Lawrence
 * Student ID: 922384785
 * Github ID: liftcodesleep
 * Project: Assignment 2 - Tapeless Rule
 *
 * File:
 *
 * Description: Measure distance using the Echo Sensor (HC-SR04). Display
 * distance in centimeters in your terminal. Show the object moving and the
 * distance on the console in the video. This should be in a loop so as you
 * move the item, the distance is printed on the console. Also use a ton of
 * defensive functions.
 *
 **************************************************************/
 
#include <pigpio.h>
#include <stdio.h>

#define TRIG1 23
#define ECHO1 24
#define TRIG2 17
#define ECHO2 27

volatile uint32_t pulse_tick1, pulse_tick2;
int previous1 = 0, previous2 = 0;

int local_sleep(int microseconds) {
    if (gpioSleep(PI_TIME_RELATIVE, microseconds, 0)) {
        printf("Failed gpioSleep! Exiting.\n");
        return 1;
    }
    return 0;
}

int local_write(int pin, int pin_state) {
    if (gpioWrite(pin, pin_state)) {
        printf("Failed gpioWrite! Exiting.\n");
        return 1;
    }
    return 0;
}


int pulse(int trig, int echo, int *previous) {
    if (local_write(trig, PI_OFF)) {
        return 1;
    };
    gpioDelay(5);
    if (local_write(trig, PI_ON)) {
        return 1;
    };
    gpioDelay(10);
    if (local_write(trig, PI_OFF)) {
        return 1;
    };
    gpioDelay(5);

    uint32_t pulse_tick = gpioTick();
    uint32_t begin, end;
    uint32_t distance;

    if (previous == &previous1) {
        begin = pulse_tick1;
        end = pulse_tick;
        pulse_tick1 = pulse_tick;
    } else {
        begin = pulse_tick2;
        end = pulse_tick;
        pulse_tick2 = pulse_tick;
    }

    distance = (end - begin) / 58;

    *previous = distance;

    if (distance >= 420) {
        printf("Distance out of range\n");
        *previous = 0;
    }

    return 0;
}

int median(int a, int b, int c) {
    if ((a <= b && b <= c) || (c <= b && b <= a)) {
        return b;
    } else if ((b <= a && a <= c) || (c <= a && a <= b)) {
        return a;
    } else {
        return c;
    }
}

void get_distance(int pin, int pin_state, uint32_t time) {
    static uint32_t begin1, begin2, end1, end2;
    uint32_t distance1, distance2;

    if (pin == ECHO1) {
        if (pin_state == PI_ON) {
            begin1 = time;
        } else if (pin_state == PI_OFF) {
            end1 = time;
            distance1 = (end1 - begin1) / 58;
            previous1 = distance1;
            if (distance1 >= 420) {
                printf("Distance from Sensor 1 out of range\n");
                previous1 = 0;
            }
        }
    } else if (pin == ECHO2) {
        if (pin_state == PI_ON) {
            begin2 = time;
        } else if (pin_state == PI_OFF) {
            end2 = time;
            distance2 = (end2 - begin2) / 58;
            previous2 = distance2;
            if (distance2 >= 420) {
                printf("Distance from Sensor 2 out of range\n");
                previous2 = 0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // init GPIO
    if (gpioInitialise() < 0) {
        printf("Failed to initialize GPIO! Exiting.\n");
        return 1;
    }

    // TRIG and ECHO pins for Sensor 1
    if (gpioSetMode(TRIG1, PI_OUTPUT) || gpioSetMode(ECHO1, PI_INPUT)) {
        printf("Failed to set TRIG1 or ECHO1 pins! Exiting.\n");
        return 1;
    }

    // TRIG and ECHO pins for Sensor 2
    if (gpioSetMode(TRIG2, PI_OUTPUT) || gpioSetMode(ECHO2, PI_INPUT)) {
        printf("Failed to set TRIG2 or ECHO2 pins! Exiting.\n");
        return 1;
    }

    // take measurements as ECHO flips
    if (gpioSetAlertFunc(ECHO1, get_distance)) {
        printf("Failed to set alert function for Sensor 1! Exiting.\n");
        return 1;
    }

    // take measurements as ECHO flips
    if (gpioSetAlertFunc(ECHO2, get_distance)) {
        printf("Failed to set alert function for Sensor 2! Exiting.\n");
        return 1;
    }


    if (pulse(TRIG1, ECHO1, &previous1) || pulse(TRIG2, ECHO2, &previous2)) {
            break;
        }
        if (local_sleep(1)) {
            break;
        }

        int median_distance = median(previous1, previous2, 0);

        if (median_distance > 0) {
      if(median_distance < 50){
        if(median_distance < 10){ // if it gets too close
          accelerate_backward();
          ulseep(500000);
        }
        stop();
        turn_left();
        usleep(1000000);  // figure out a proper time for this
        accelerate_forward;
        turn_right();
        usleep(500000); // with this the car should be facing the same direction as the start but to the side a bit

        while(1){
          // repeat this until the car is past the object
          stop();
          accelerate_forward();
          usleep(1000000);
          stop();
          turn_right();
          usleep(1000000);
          stop();

          if (pulse(TRIG1, ECHO1, &previous1) || pulse(TRIG2, ECHO2, &previous2)) {  // check if car is past the obstacle
            break;
          }
          if (local_sleep(1)) {
            break;
          }
          if (median_distance == 0 || median_distance > 100){ // the car is past the obstacle
            break;
          }else{
            turn_left();
            usleep(1000000);
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

    gpioTerminate();
    return 0;
}

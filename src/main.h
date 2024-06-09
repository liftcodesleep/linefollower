#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib/DEV_Config.h"
#include "lib/PCA9685.h"

#define CREATE 0
#define DESTROY 1
typedef enum DIRECTION { STRAIGHT, LEFT, RIGHT, STALL } DIRECTION;
typedef enum GEAR { FORWARD, REVERSE, PARK } GEAR;

typedef struct sensor_data {
  int pin_state;
  int GPIO;
} sensor_data;
#define NUM_BUTTONS 1
#define BUTTON_PIN 23
#define NUM_INPUT 4
#define NUM_OUTPUT 1
#define NUM_LINES 3
#define NUM_ECHOES 1
#define LINE_LEFT 16
#define LINE_RIGHT 20
#define LINE_CENTER 21
#define ECHO 19
#define TRIG 26
#define NUM_SENSORS (NUM_LINES + NUM_ECHOES)
#define DISTANCE_THRESHOLD 50
#define PWM_MOTOR_LEFT PCA_CHANNEL_0
#define M1_REV PCA_CHANNEL_1
#define M1_FWD PCA_CHANNEL_2
#define M2_FWD PCA_CHANNEL_3
#define M2_REV PCA_CHANNEL_4
#define PWM_MOTOR_RIGHT PCA_CHANNEL_5

#endif /*MAIN_H*/

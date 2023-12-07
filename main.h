#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib/DEV_Config.h"
#include "lib/PCA9685.h"

#define CREATE 0
#define DESTROY 1

typedef struct sensor_data {
  int pin_state;
  int GPIO;
} sensor_data;

#define NUM_BUTTONS 1
#define NUM_SENSORS 3
#define NUM_GPIO_PINS NUM_BUTTONS + NUM_SENSORS
#define BUTTON_PIN 23
#define LINE_LEFT 16
#define LINE_RIGHT 20
#define LINE_CENTER 21
#define ECHO_RIGHT
#define ECHO_LEFT
#define TRIG_LEFT
#define TRIG_RIGHT

#define PWM_CHANNEL_A PCA_CHANNEL_0
#define M1_REV_CHANNEL PCA_CHANNEL_1
#define M1_FWD_CHANNEL PCA_CHANNEL_2
#define M2_FWD_CHANNEL PCA_CHANNEL_3
#define M2_REV_CHANNEL PCA_CHANNEL_4
#define PWM_CHANNEL_B PCA_CHANNEL_5

#endif /*MAIN_H*/

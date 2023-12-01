#include "main.h"

#include <pigpio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUTTON_PIN 4
#define PWM_CHANNEL_A PCA_CHANNEL_0
#define M1_REV_CHANNEL PCA_CHANNEL_1
#define M1_FWD_CHANNEL PCA_CHANNEL_2
#define M2_FWD_CHANNEL PCA_CHANNEL_3
#define M2_REV_CHANNEL PCA_CHANNEL_4
#define PWM_CHANNEL_B PCA_CHANNEL_5

#define SERVO_PIN 27

int pressed = 0;

// Function to accelerate motors forward
void accelerate_forward() {
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 100);
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 100);
  PCA9685_SetLevel(M1_FWD_CHANNEL, 1);
  PCA9685_SetLevel(M1_REV_CHANNEL, 0);
  PCA9685_SetLevel(M2_FWD_CHANNEL, 1);
  PCA9685_SetLevel(M2_REV_CHANNEL, 0);
}

// Function to accelerate motors backward
void accelerate_backward() {
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 100);
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 100);
  PCA9685_SetLevel(M1_FWD_CHANNEL, 0);
  PCA9685_SetLevel(M1_REV_CHANNEL, 1);
  PCA9685_SetLevel(M2_FWD_CHANNEL, 0);
  PCA9685_SetLevel(M2_REV_CHANNEL, 1);
}

// Function to turn left
void turn_left() {
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 100);
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 100);
  PCA9685_SetLevel(M1_FWD_CHANNEL, 1);
  PCA9685_SetLevel(M1_REV_CHANNEL, 0);
  PCA9685_SetLevel(M2_FWD_CHANNEL, 0);
  PCA9685_SetLevel(M2_REV_CHANNEL, 1);
}

// Function to turn right
void turn_right() {
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 100);
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 100);
  PCA9685_SetLevel(M1_FWD_CHANNEL, 0);
  PCA9685_SetLevel(M1_REV_CHANNEL, 1);
  PCA9685_SetLevel(M2_FWD_CHANNEL, 1);
  PCA9685_SetLevel(M2_REV_CHANNEL, 0);
}

// Function to stop motors
void stop() {
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 0);
  PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 0);
  PCA9685_SetLevel(M1_FWD_CHANNEL, 0);
  PCA9685_SetLevel(M2_FWD_CHANNEL, 0);
  PCA9685_SetLevel(M1_REV_CHANNEL, 0);
  PCA9685_SetLevel(M2_REV_CHANNEL, 0);
}

// Function to control the motors as per the specified sequence
void motor_control_sequence(int pin, int pin_state, uint32_t time) {
  if (!pressed && pin_state == 1) {
    // Accelerate both motors forward
    printf("Accelerating both motors forward to top speed\n");
    accelerate_forward();
    usleep(1000000);  // Sleep for 1 second

    // Stop both motors
    printf("Stopping both motors\n");
    stop();
    usleep(1000000);  // Sleep for 1 second

    // Accelerate both motors in reverse
    printf("Accelerating both motors in reverse to top speed\n");
    accelerate_backward();
    usleep(1000000);  // Sleep for 1 second

    // Stop both motors
    printf("Stopping both motors\n");
    stop();
    usleep(1000000);  // Sleep for 1 second

    printf("Turning right\n");
    turn_right();
    usleep(1000000);  // Sleep for 1 second

    printf("Turning left\n");
    turn_left();
    usleep(1000000);  // Sleep for 1 second

    pressed = 1;
  }
}

void PCA_Init(void) {
  PCA9685_Init(0x40);
  PCA9685_SetPWMFreq(100);
}

int main() {
  // Initialize and configure PCA9685, pigpio, and motor components as needed
  if (DEV_ModuleInit()) {
    printf("Failed to initialize DEV_Module! Exiting.\n");
    exit(0);
  }
  if (gpioInitialise() < 0) {
    printf("Failed to initialize pigpio! Exiting.\n");
    exit(0);
  }
  // PCA_Init();

  // Servo Demo
  gpioSetMode(SERVO_PIN, PI_INPUT);
  gpioServo(SERVO_PIN, 550);
  sleep(2);
  gpioServo(SERVO_PIN, 2350);
  sleep(2);

  // gpioSetMode(BUTTON_PIN, PI_INPUT);
  // gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);
  // if (gpioSetAlertFunc(BUTTON_PIN, motor_control_sequence)) {
  //   printf("Failed to set alert function! Exiting.\n");
  //   exit(0);
  // }
  // printf("Press the button to control the motors.\n");
  // while (!pressed) {
  //   usleep(500000);  // Sleep to debounce
  // }

  // Clean up and exit
  printf("Cleaning up and exiting\n");
  gpioTerminate();
  stop();  // Stop motors before exiting
  DEV_ModuleExit();

  return 0;
}

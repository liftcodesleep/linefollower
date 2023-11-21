#include "main.h"

#include <pigpio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUTTON_PIN 4
#define M1_FWD_CHANNEL PCA_CHANNEL_0
#define M2_FWD_CHANNEL PCA_CHANNEL_1
#define M1_REV_CHANNEL PCA_CHANNEL_2
#define M2_REV_CHANNEL PCA_CHANNEL_3
#define PWM_CHANNEL_A PCA_CHANNEL_4
#define PWM_CHANNEL_B PCA_CHANNEL_5

int pressed = 0;

// Function to control the motors as per the specified sequence
void MotorControlSequence(int pin, int pin_state, uint32_t time) {
  if (!pressed && pin_state == 1) {
    // Accelerate both motors forward
    printf("Accelerating both motors forward to top speed\n");
    PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 100);
    PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 100);
    PCA9685_SetLevel(M1_FWD_CHANNEL, 1);
    PCA9685_SetLevel(M1_REV_CHANNEL, 0);
    PCA9685_SetLevel(M2_FWD_CHANNEL, 1);
    PCA9685_SetLevel(M2_REV_CHANNEL, 0);

    usleep(2000000);  // Sleep for 1 second

    // Stop both motors
    printf("Stopping both motors\n");
    PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 0);
    PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 0);
    PCA9685_SetLevel(M1_FWD_CHANNEL, 0);
    PCA9685_SetLevel(M2_FWD_CHANNEL, 0);
    PCA9685_SetLevel(M1_REV_CHANNEL, 0);
    PCA9685_SetLevel(M2_REV_CHANNEL, 0);

    usleep(2000000);  // Sleep for 1 second

    // Accelerate both motors in reverse
    printf("Accelerating both motors in reverse to top speed\n");
    PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 100);
    PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 100);
    PCA9685_SetLevel(M1_FWD_CHANNEL, 0);
    PCA9685_SetLevel(M1_REV_CHANNEL, 1);
    PCA9685_SetLevel(M2_FWD_CHANNEL, 0);
    PCA9685_SetLevel(M2_REV_CHANNEL, 1);

    usleep(2000000);  // Sleep for 1 second

    // Stop both motors
    printf("Stopping both motors\n");
    PCA9685_SetPwmDutyCycle(PWM_CHANNEL_A, 0);
    PCA9685_SetPwmDutyCycle(PWM_CHANNEL_B, 0);
    PCA9685_SetLevel(M1_FWD_CHANNEL, 0);
    PCA9685_SetLevel(M2_FWD_CHANNEL, 0);
    PCA9685_SetLevel(M1_REV_CHANNEL, 0);
    PCA9685_SetLevel(M2_REV_CHANNEL, 0);

    usleep(2000000);  // Sleep for 1 second

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
  PCA_Init();
  gpioSetMode(BUTTON_PIN, PI_INPUT);
  gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);
  if (gpioSetAlertFunc(BUTTON_PIN, MotorControlSequence)) {
    printf("Failed to set alert function! Exiting.\n");
    exit(0);
  }
  printf("Press the button to control the motors.\n");
  while (!pressed) {
    usleep(500000);  // Sleep to debounce
  }

  // Clean up and exit
  printf("Cleaning up and exiting\n");
  gpioTerminate();
  PCA9685_SetLevel(M1_FWD_CHANNEL, 0);
  PCA9685_SetLevel(M2_FWD_CHANNEL, 0);
  PCA9685_SetLevel(M1_REV_CHANNEL, 0);
  PCA9685_SetLevel(M2_REV_CHANNEL, 0);
  PCA9685_SetLevel(PWM_CHANNEL_A, 0);
  PCA9685_SetLevel(PWM_CHANNEL_B, 0);
  DEV_ModuleExit();

  return 0;
}

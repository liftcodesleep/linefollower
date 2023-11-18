#include "main.h"

#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // Include for boolean type

#define BUTTON_PIN 4
#define MOTOR1_FORWARD_CHANNEL PCA_CHANNEL_0
#define MOTOR1_REVERSE_CHANNEL PCA_CHANNEL_1
#define MOTOR2_FORWARD_CHANNEL PCA_CHANNEL_2
#define MOTOR2_REVERSE_CHANNEL PCA_CHANNEL_3

int pressed = 0;

// Function to control the motor as per the specified sequence
void MotorControlSequence(int pin, int pin_state, uint32_t time) {
    if (!pressed && pin_state == 1) {
        // Accelerate Motor 1 forward
        printf("Accelerating Motor 1 forward to top speed\n");
        PCA9685_SetPwmDutyCycle(MOTOR1_FORWARD_CHANNEL, 100);
        PCA9685_SetLevel(MOTOR1_REVERSE_CHANNEL, 0);

        // Accelerate Motor 2 forward
        printf("Accelerating Motor 2 forward to top speed\n");
        PCA9685_SetPwmDutyCycle(MOTOR2_FORWARD_CHANNEL, 100);
        PCA9685_SetLevel(MOTOR2_REVERSE_CHANNEL, 0);

        usleep(2000000);  // Sleep for 2 seconds

        // Brake both motors
        printf("Braking both motors\n");
        PCA9685_SetLevel(MOTOR1_FORWARD_CHANNEL, 0);
        PCA9685_SetLevel(MOTOR2_FORWARD_CHANNEL, 0);

        usleep(1000000);  // Sleep for 1 second

        // Turn left (Motor 1 backward, Motor 2 forward)
        printf("Turning left\n");
        PCA9685_SetPwmDutyCycle(MOTOR1_REVERSE_CHANNEL, 50);
        PCA9685_SetLevel(MOTOR2_FORWARD_CHANNEL, 100);

        usleep(2000000);  // Sleep for 2 seconds

        // Turn right (Motor 1 forward, Motor 2 backward)
        printf("Turning right\n");
        PCA9685_SetPwmDutyCycle(MOTOR1_FORWARD_CHANNEL, 100);
        PCA9685_SetLevel(MOTOR2_FORWARD_CHANNEL, 0);
        PCA9685_SetPwmDutyCycle(MOTOR2_REVERSE_CHANNEL, 50);

        usleep(2000000);  // Sleep for 2 seconds

        // Reverse both motors
        printf("Reversing both motors\n");
        PCA9685_SetLevel(MOTOR1_FORWARD_CHANNEL, 0);
        PCA9685_SetPwmDutyCycle(MOTOR2_REVERSE_CHANNEL, 100);

        usleep(2000000);  // Sleep for 2 seconds

        // Stop both motors
        printf("Stopping both motors\n");
        PCA9685_SetLevel(MOTOR1_FORWARD_CHANNEL, 0);
        PCA9685_SetLevel(MOTOR2_FORWARD_CHANNEL, 0);

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
    PCA9685_SetPwmDutyCycle(MOTOR1_FORWARD_CHANNEL, 0);
    PCA9685_SetPwmDutyCycle(MOTOR2_FORWARD_CHANNEL, 0);
    DEV_ModuleExit();

    return 0;
}

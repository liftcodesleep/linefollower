#include "main.h"

#include <pigpio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int pressed = 0;
int reading = 0;
// Mutex for synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int GPIO_pins[NUM_GPIO_PINS] = {LINE_CENTER, LINE_LEFT, LINE_RIGHT, BUTTON_PIN};

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
void motor_control_demo(int pin, int pin_state, uint32_t time) {
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
    // Stop both motors
    printf("Stopping both motors\n");
    stop();
    usleep(1000000);  // Sleep for 1 second
    printf("Turning left\n");
    turn_left();
    usleep(1000000);  // Sleep for 1 second
    pressed = 1;
  }
}

void handle_sigint(int sig_num) { reading = 1; }

// package pertinent sensor info per thread
sensor_data *handle_packs(int num_sensors, int mode) {
  static sensor_data *packs = NULL;
  if (mode == CREATE) {
    packs = (packs == NULL) ? calloc(num_sensors, sizeof(sensor_data)) : packs;
    // validate init
    if (packs == NULL) {
      perror("handle_packs: packs calloc failed\n");
      return NULL;
    }
    // set IDs
    for (int i = 0; i < num_sensors; i++) {
      packs[i].GPIO = GPIO_pins[i];
    }
  } else if (mode == DESTROY) {
    if (packs != NULL) {
      free(packs);
    }
  }
  return packs;
}

// read the sensor identified in the pack and store its pin state
void *check_sensors(void *ptr) {
  sensor_data *sensor = (sensor_data *)ptr;
  while (reading == 0) {
    // read appropriate pin
    int pin_state = gpioRead(sensor->GPIO);
    if (pin_state != PI_BAD_GPIO) {
      // update pin state
      sensor->pin_state = pin_state;
    } else {
      printf("check_sensors: gpioRead(%d) returned PI_BAD_GPIO.\n",
             sensor->GPIO);
    }
  }
  return NULL;
}

int setup() {
  if (DEV_ModuleInit()) {
    printf("setup: DEV_Moduleinit() failed! Exiting.\n");
    return 1;
  }
  // initialize GPIO interface
  if (gpioInitialise() < 0) {
    printf("setup: gpioInitialise() failed! Exiting.\n");
    return 1;
  }
  PCA9685_Init(0x40);
  PCA9685_SetPWMFreq(100);
  for (int i = 0; i < NUM_GPIO_PINS; i++) {
    if (gpioSetMode(GPIO_pins[i], PI_INPUT) < 0) {
      printf("setup: gpioSetMode() failed! Exiting.\n");
      return 1;
    }
  }
  if (gpioSetAlertFunc(BUTTON_PIN, motor_control_demo)) {
    printf("gpioSetAlertFunc() failed! Exiting.\n");
    return 1;
  }
  return 0;
}

int cleanup() {
  stop();  // Stop motors before exiting
  gpioTerminate();
  DEV_ModuleExit();
  handle_packs(NUM_GPIO_PINS, DESTROY);
}

int main() {
  // Initialize and configure PCA9685, pigpio, and motor components as needed
  if (setup() > 0) {
    return 1;
  }
  printf("Press the button to run the motors.\n");
  while (!pressed) {
    usleep(500000);  // Sleep to debounce
  }
  pthread_t threads[NUM_SENSORS];
  sensor_data *sensor_packs = handle_packs(NUM_SENSORS, CREATE);
  signal(SIGINT, handle_sigint);
  // thread pin input reads
  // totally not GreatValue SetAlertFunc
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (pthread_create(&threads[i], NULL, &check_sensors,
                       (void *)&sensor_packs[i]) != 0) {
      printf("main: pthread_create %d failed, exiting.\n", i);
      return 1;
    }
  }

  // constantly output pin states
  while (reading == 0) {
    time_sleep(1);
    for (int i = 0; i < NUM_SENSORS; i++) {
      printf("%-10s%d%s%10s\n", "GPIO PIN: ", sensor_packs[i].GPIO, " Line? ",
             sensor_packs[i].pin_state ? "YES" : "NO");
    }
  }

  // join threads and clean up
  printf("Cleaning up and exiting\n");
  for (int i = 1; i < NUM_SENSORS; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      printf("main: pthread_join failed, exiting.\n");
      return 1;
    }
  }
  cleanup();
  return 0;
}

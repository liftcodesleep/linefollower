#include "main.h"

#include <pigpio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

DIRECTION current_direction = STALL;
DIRECTION prior_direction = STALL;
int consecutive_readings = 0;
int pressed = 0;
int reading = 0;
int previous = 0;              // stores last recorded distance
volatile uint32_t pulse_tick;  // stores time of each pulse
volatile bool obstacle_detected = false;

// Mutex for synchronization
int input_pins[NUM_INPUT] = {LINE_CENTER, LINE_LEFT, LINE_RIGHT, ECHO};
int output_pins[NUM_OUTPUT] = {TRIG};
int buttons[NUM_BUTTONS] = {BUTTON_PIN};

// Function to accelerate motors forward
void accelerate_forward(int milliseconds) {
  printf("accelerate_forward:\n");
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_LEFT, 100);
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_RIGHT, 100);
  PCA9685_SetLevel(M1_FWD, 1);
  PCA9685_SetLevel(M1_REV, 0);
  PCA9685_SetLevel(M2_FWD, 1);
  PCA9685_SetLevel(M2_REV, 0);
  prior_direction = current_direction;
  current_direction = STRAIGHT;
}
// Function to accelerate motors backward
void accelerate_backward() {
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_LEFT, 100);
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_RIGHT, 100);
  PCA9685_SetLevel(M1_FWD, 0);
  PCA9685_SetLevel(M1_REV, 1);
  PCA9685_SetLevel(M2_FWD, 0);
  PCA9685_SetLevel(M2_REV, 1);
}
// Function to turn left
void turn_left(int duty) {
  printf("turn_left:\n");
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_LEFT, duty);
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_RIGHT, 0);
  PCA9685_SetLevel(M1_FWD, 1);
  PCA9685_SetLevel(M1_REV, 0);
  PCA9685_SetLevel(M2_FWD, 1);
  PCA9685_SetLevel(M2_REV, 0);
  prior_direction = current_direction;
  current_direction = LEFT;
}
// Function to turn right
void turn_right(int duty) {
  printf("turn_right:\n");
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_LEFT, 0);
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_RIGHT, duty);
  PCA9685_SetLevel(M1_FWD, 1);
  PCA9685_SetLevel(M1_REV, 0);
  PCA9685_SetLevel(M2_FWD, 1);
  PCA9685_SetLevel(M2_REV, 0);
  prior_direction = current_direction;
  current_direction = RIGHT;
}
// Function to stop motors
void stop_motors() {
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_LEFT, 0);
  PCA9685_SetPwmDutyCycle(PWM_MOTOR_RIGHT, 0);
  PCA9685_SetLevel(M1_FWD, 0);
  PCA9685_SetLevel(M2_FWD, 0);
  PCA9685_SetLevel(M1_REV, 0);
  PCA9685_SetLevel(M2_REV, 0);
  prior_direction = current_direction;
  current_direction = STALL;
}

void commence() { pressed = 1; }

void handle_sigint(int sig_num) { reading = 1; }

int local_write(int pin, int pin_state) {
  if (gpioWrite(pin, pin_state)) {
    printf("Failed gpioWrite! Exiting.\n");
    return 1;
  }
  return 0;
}

int local_sleep(int microseconds) {
  if (gpioSleep(PI_TIME_RELATIVE, microseconds, 0)) {
    printf("Failed gpioSleep! Exiting.\n");
    return 1;
  }
  return 0;
}

int pulse() {
  if (local_write(TRIG, PI_OFF)) {
    return 1;
  };
  gpioDelay(5);
  if (local_write(TRIG, PI_ON)) {  // send pulse
    return 1;
  };
  gpioDelay(10);  // Pulse must be at least 10 microseconds
  if (local_write(TRIG, PI_OFF)) {
    return 1;
  };
  // delay to minimize interference
  gpioDelay(10);
  pulse_tick = gpioTick();  // get time of pulse for future reference
  // printf("pulse: pulse_tick = %d\n", pulse_tick);
  return 0;
}

// package pertinent sensor info per thread
sensor_data *handle_packs(int num_sensors, int mode) {
  static sensor_data *packs = NULL;
  printf("handling packs\n");
  if (mode == CREATE) {
    packs = (packs == NULL) ? calloc(num_sensors, sizeof(sensor_data)) : packs;
    // validate init
    if (packs == NULL) {
      perror("handle_packs: packs calloc failed\n");
      return NULL;
    }
    // set IDs
    for (int i = 0; i < num_sensors; i++) {
      packs[i].GPIO = input_pins[i];
    }
  } else if (mode == DESTROY) {
    if (packs != NULL) {
      free(packs);
    }
  }
  printf("handling packs\n");
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

void get_distance(int pin, int pin_state, uint32_t time) {
  static uint32_t begin, end;
  uint32_t distance;
  if (time > pulse_tick) {  // don't measure the sensor firing
    /*
      When ECHO pin state changes to HIGH, mark the time
      When ECHO pin state changes to LOW, measure the distance using the delta
    */
    if (pin_state == PI_ON) {
      begin = time;  // mark timestamp of ECHO firing
    } else if (pin_state == PI_OFF) {
      end = time;                     // mark timestamp of ECHO receiving
      distance = (end - begin) / 58;  // convert to cm by dividing delta by 58
      // printf("get_distance: distance = %d\n", distance);
      // printf("get_distance: previous = %d\n", previous);
      previous = distance;  // record distance
    }
  }
}

void *check_obstacle(void *ptr) {
  while (reading == 0) {
    // printf("pulse: pulsing!\n");
    if (pulse() > 0) {
      printf("pulse: pulse > 0\n");
      break;
    }
    if (previous < DISTANCE_THRESHOLD) {
      obstacle_detected = true;
    }
    if (local_sleep(1) > 0) {
      printf("pulse: sleep > 0\n");
      break;
    }
    if (previous > 0) {
      // printf("Distance in cm: %d\n", previous);
    }
  }
  printf("echoing NULL!\n");
  return NULL;
}

void avoid_obstacle() {
  while (previous < DISTANCE_THRESHOLD) {
    turn_left(90);
  }
  accelerate_forward(1000000);
  while (previous > DISTANCE_THRESHOLD) {
    turn_right(90);
  }
  stop_motors();
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
  for (int i = 0; i < NUM_INPUT; i++) {
    if (gpioSetMode(input_pins[i], PI_INPUT) < 0) {
      printf("setup: gpioSetMode():inputs failed! Exiting.\n");
      return 1;
    }
  }
  for (int i = 0; i < NUM_OUTPUT; i++) {
    if (gpioSetMode(output_pins[i], PI_OUTPUT) < 0) {
      printf("setup: gpioSetMode()outputs failed! Exiting.\n");
      return 1;
    }
  }
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (gpioSetMode(buttons[i], PI_INPUT) < 0) {
      printf("setup: gpioSetMode():buttons failed! Exiting.\n");
      return 1;
    }
  }
  if (gpioSetAlertFunc(BUTTON_PIN, commence)) {
    printf("gpioSetAlertFunc():button failed! Exiting.\n");
    return 1;
  }
  printf("button alert set!\n");
  if (gpioSetAlertFunc(ECHO, get_distance)) {
    printf("gpioSetAlertFunc():ECHO_LEFT failed! Exiting.\n");
    return 1;
  }
  return 0;
}

int cleanup() {
  stop_motors();  // Stop motors before exiting
  gpioTerminate();
  DEV_ModuleExit();
  handle_packs(NUM_SENSORS, DESTROY);
}

void follow_line(sensor_data *sensor_packs) {
  while (reading == 0) {
    if (sensor_packs[0].pin_state > 0 && sensor_packs[1].pin_state == 0 &&
        sensor_packs[2].pin_state == 0) {
      if (current_direction != STRAIGHT && consecutive_readings > 2) {
        accelerate_forward(1);
        consecutive_readings = 0;
      }
    }  // center and left see line
    else if (sensor_packs[0].pin_state > 0 && sensor_packs[1].pin_state > 0 &&
             sensor_packs[2].pin_state == 0) {
      if (current_direction != LEFT && consecutive_readings > 2) {
        printf("center and left see line\n");
        turn_left(90);
        consecutive_readings = 0;
      }
    }  // center and right see line
    else if (sensor_packs[0].pin_state > 0 && sensor_packs[1].pin_state == 0 &&
             sensor_packs[2].pin_state > 0) {
      if (current_direction != RIGHT && consecutive_readings > 2) {
        printf("center and right see line\n");
        turn_right(90);
        consecutive_readings = 0;
      }
    }  // only left sees line
    else if (sensor_packs[0].pin_state == 0 && sensor_packs[1].pin_state > 0 &&
             sensor_packs[2].pin_state == 0) {
      if (current_direction != LEFT && consecutive_readings > 2) {
        printf("left sees line\n");
        turn_left(90);
        consecutive_readings = 0;
      }
    }  // only right sees line
    else if (sensor_packs[0].pin_state == 0 && sensor_packs[1].pin_state == 0 &&
             sensor_packs[2].pin_state > 0) {
      if (current_direction != RIGHT && consecutive_readings > 2) {
        printf("right sees line\n");
        turn_right(90);
        consecutive_readings = 0;
      }
    }  // no line detected
    else if (sensor_packs[0].pin_state == 0 && sensor_packs[1].pin_state == 0 &&
             sensor_packs[2].pin_state == 0) {
      if (current_direction == STRAIGHT) {
        switch (prior_direction) {
          case LEFT:
            turn_left(90);
          case RIGHT:
            turn_right(90);
          default:
            turn_left(90);
        }
      }
    }
    // TODO: MAYBE :)
    // else if (sensor_packs[0].pin_state == 0 &&
    //            sensor_packs[1].pin_state > 0 && sensor_packs[2].pin_state >
    //            0) {
    // }
    else if (sensor_packs[0].pin_state > 0 && sensor_packs[1].pin_state > 0 &&
             sensor_packs[2].pin_state > 0) {
      // this is a cross, stretch goal
    }

    if (obstacle_detected) {
      // Perform obstacle avoidance
      avoid_obstacle();
      // Reset obstacle detection flag
      obstacle_detected = false;
    }
    usleep(100000);  // debounce
    consecutive_readings++;
  }
}

int main() {
  // Initialize and configure PCA9685, pigpio, and motor components as needed
  if (setup() > 0) {
    return 1;
  }
  signal(SIGINT, handle_sigint);
  // printf("Press the button to run the course.\n");
  // Sleep to debounce
  // while (!pressed) {
  //   usleep(500000);
  // }
  pthread_t threads[NUM_SENSORS];
  sensor_data *sensor_packs = handle_packs(NUM_SENSORS, CREATE);
  // thread line input reads
  for (int i = 0; i < NUM_LINES; i++) {
    printf("main: threading line on %d\n", i);
    if (pthread_create(&threads[i], NULL, &check_sensors,
                       (void *)&sensor_packs[i]) != 0) {
      printf("main: pthread_create %d failed, exiting.\n", i);
      return 1;
    }
  }
  for (int i = NUM_LINES; i < NUM_SENSORS; i++) {
    printf("main: threading echo on %d\n", i);
    if (pthread_create(&threads[i], NULL, &check_obstacle, NULL) != 0) {
      printf("main: pthread_create %d failed, exiting.\n", i);
      return 1;
    }
  }
  // DEBUGGING: constantly output pin states
  while (reading == 7) {
    time_sleep(1);
    for (int i = 0; i < NUM_INPUT; i++) {
      printf("%-10s%d%s%10s\n", "GPIO PIN: ", sensor_packs[i].GPIO, " Line? ",
             sensor_packs[i].pin_state ? "YES" : "NO");
    }
  }
  follow_line(sensor_packs);
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

#ifndef SONAR_H
#define SONAR_H

#include "main.h"


int local_sleep(int microseconds);
int local_write(int pin, int pin_state);
int pulse(int pin);
int median(int a, int b, int c);
void get_distance(int pin, int pin_state, uint32_t time);
int obstacleAvoidance(void);

#endif /*SONAR_H*/

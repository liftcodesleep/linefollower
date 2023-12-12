# Makefile for the motor control program

CC = gcc
<<<<<<< HEAD
CFLAGS = -Wall -Wextra -std=c99 -O2
LIBS = -lpigpio -lrt -lpthread

TARGET = main

SRCS = main.c PCA9685.c DEV_Config.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean
=======
CFLAGS = -Wall -D USE_DEV_LIB

SRCS = main.c lib/DEV_Config.c lib/PCA9685.c lib/dev_hardware_i2c.c lib/dev_hardware_SPI.c lib/sysfs_gpio.c
OBJS = $(SRCS:.c=.o)
EXEC = main
>>>>>>> c36d1943cc84da88fa467b49e60025b5eec88749

LIBS = -lpigpio -lbcm2835 -lm 

<<<<<<< HEAD
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
=======
INCLUDES = -I./lib

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $@

clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: clean
>>>>>>> c36d1943cc84da88fa467b49e60025b5eec88749

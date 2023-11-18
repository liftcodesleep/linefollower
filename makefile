# Makefile for the motor control program

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LIBS = -lpigpio -lrt -lpthread

TARGET = main

SRCS = main.c PCA9685.c DEV_Config.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

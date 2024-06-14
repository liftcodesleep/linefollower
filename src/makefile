CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -D USE_DEV_LIB
LIBS = -lpigpio -lbcm2835 -lm -lrt -lpthread
INCLUDES = -I./lib

TARGET = main

SRCS = main.c lib/DEV_Config.c lib/PCA9685.c lib/dev_hardware_i2c.c lib/dev_hardware_SPI.c lib/sysfs_gpio.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET)

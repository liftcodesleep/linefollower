CC = gcc
CFLAGS = -Wall -D USE_DEV_LIB

SRCS = main.c assignment2.c lib/DEV_Config.c lib/PCA9685.c lib/dev_hardware_i2c.c lib/dev_hardware_SPI.c lib/sysfs_gpio.c
OBJS = $(SRCS:.c=.o)
EXEC = main

LIBS = -lpigpio -lbcm2835 -lm 

INCLUDES = -I./lib

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $@

clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: clean

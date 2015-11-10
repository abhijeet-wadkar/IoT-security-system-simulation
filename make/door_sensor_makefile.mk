CC = gcc
CCFLAGS = -DDEBUG -g -Wall --std=gnu99
EXE_NAME = door_sensor
EXT_LIB = -lpthread

INCLUDES = src/common \
	src/door_sensor

SRCS = src/door_sensor/door_sensor.c \
	src/door_sensor/door_sensor_main.c \
	src/common/network_functions.c \
	src/common/network_read_thread.c \
	src/common/string_helper_functions.c \
	src/common/logger.c \
	src/common/logical_clock_utils.c
	
include base_make.mk

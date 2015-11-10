CC = gcc
CCFLAGS = -DDEBUG -g -Wall --std=c99
EXE_NAME = sensor
EXT_LIB = -lpthread

INCLUDES = src/common \
	src/sensor

SRCS = src/sensor/sensor.c \
	src/sensor/sensor_main.c \
	src/common/network_functions.c \
	src/common/network_read_thread.c \
	src/common/string_helper_functions.c \
	src/common/logger.c
	
include base_make.mk
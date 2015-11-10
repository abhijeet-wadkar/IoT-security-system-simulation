CC = gcc
CCFLAGS = -DDEBUG -g -Wall
EXE_NAME = key_chain_sensor
EXT_LIB = -lpthread

INCLUDES = src/common \
	src/key_chain_sensor

SRCS = src/key_chain_sensor/key_chain_sensor.c \
	src/key_chain_sensor/key_chain_sensor_main.c \
	src/common/network_functions.c \
	src/common/network_read_thread.c \
	src/common/string_helper_functions.c \
	src/common/logger.c
	
include base_make.mk

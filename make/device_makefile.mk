CC = gcc
CCFLAGS = -DDEBUG -g -Wall
EXE_NAME = device
EXT_LIB = -lpthread

INCLUDES = src/common \
	src/device

SRCS = src/device/device.c \
	src/device/device_main.c \
	src/common/network_functions.c \
	src/common/network_read_thread.c \
	src/common/string_helper_functions.c \
	src/common/logger.c
	
include base_make.mk
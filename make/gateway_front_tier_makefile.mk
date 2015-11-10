CC = gcc
CCFLAGS = -g -Wall -DDEBUG --std=gnu99
EXE_NAME = gateway_front_tier
EXT_LIB = -lpthread

INCLUDES = src/common \
	src/gateway

SRCS = src/gateway/front_tier/gateway_front_tier.c \
	src/gateway/front_tier/gateway_main.c \
	src/common/network_functions.c \
	src/common/network_read_thread.c \
	src/common/string_helper_functions.c \
	src/common/logger.c \
	src/common/logical_clock_utils.c
	
include base_make.mk
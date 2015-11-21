/*
 * logger.h
 *
 *  Created on: Sep 22, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>

#define LOG_FILENAME "log.txt"
#define GATEWAY_LOG_FILENAME "gateway_log.txt"

#ifdef DEBUG
#define LOG_DEBUG(x) log_to_file x;
#define LOG(x)
#else
#define LOG_DEBUG(x)
#define LOG(x)
#endif

#define LOG_SCREEN(x) printf x

#define LOG_INFO(x) log_to_output_file x;

#define LOG_ERROR(x) printf x
//#define LOG_INFO(x) printf x

#define LOG_GATEWAY(x) log_to_gateway_log_file x;

int log_open_output_file(char *filename);
void log_open_file(char *filename);
void log_to_file(char *msg, ...);
void log_to_output_file(char *msg, ...);
void log_to_gateway_log_file(char *msg, ...);
void logger_close();

#endif /* LOGGER_H_ */

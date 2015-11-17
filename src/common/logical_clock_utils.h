/*
 * logical_clock_utils.h
 *
 *  Created on: Nov 10, 2015
 *      Author: abhijeet
 */

#ifndef LOGICAL_CLOCK_UTILS_H_
#define LOGICAL_CLOCK_UTILS_H_

#include "message.h"

void adjust_clock(int local_clock[CLOCK_SIZE], int msg_clock[CLOCK_SIZE]);
void print_logical_clock(int logical_clock[CLOCK_SIZE]);
int check_devlivery(int local_clock[CLOCK_SIZE], int msg_clock[CLOCK_SIZE]);

#endif /* LOGICAL_CLOCK_UTILS_H_ */

/*
 * logical_clock_utils.c
 *
 *  Created on: Nov 10, 2015
 *      Author: abhijeet
 */

#include "logical_clock_utils.h"

static int max(int a, int b)
{
	if(a>b)
		return (a);
	return (b);
}

void adjust_clock(int local_clock[CLOCK_SIZE], int msg_clock[CLOCK_SIZE])
{
	for(int index=0; index<CLOCK_SIZE; index++)
	{
		local_clock[index] = max(local_clock[index], msg_clock[index]);
	}
}

void print_logical_clock(int logical_clock[CLOCK_SIZE])
{
	LOG_INFO(("INFO: CLOCK"));
	LOG_INFO(("<"));
	for(int index=0; index<CLOCK_SIZE; index++)
	{
		LOG_INFO(("%d", logical_clock[index]));
	}
	LOG_INFO((">"));
}

/*
 * logical_clock_utils.c
 *
 *  Created on: Nov 10, 2015
 *      Author: abhijeet
 */

#include "logical_clock_utils.h"
#include "logger.h"

static int max(int a, int b)
{
	if(a>b)
		return (a);
	return (b);
}

int check_devlivery(int local_clock[CLOCK_SIZE], int msg_clock[CLOCK_SIZE])
{
	int flag = 0;
	for(int index=0; index<CLOCK_SIZE; index++)
	{
		if(local_clock[index] != msg_clock[index])
		{
			if(local_clock[index]+1 != msg_clock[index])
			{
				flag++;
			}
		}
	}
	if(flag > 1)
		return 0;
	return 1;
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
	LOG_INFO(("CLOCK"));
	LOG_INFO(("<"));
	for(int index=0; index<CLOCK_SIZE; index++)
	{
		if(index+1==CLOCK_SIZE)
		{
			LOG_INFO(("%d", logical_clock[index]));
		}
		else
		{
			LOG_INFO(("%d,", logical_clock[index]));
		}
	}
	LOG_INFO((">"));
}

void print_logical_clock_to_screen(int logical_clock[CLOCK_SIZE])
{
	LOG_INFO(("CLOCK"));
	LOG_INFO(("<"));
	for(int index=0; index<CLOCK_SIZE; index++)
	{
		if(index+1==CLOCK_SIZE)
		{
			LOG_INFO(("%d", logical_clock[index]));
		}
		else
		{
			LOG_INFO(("%d,", logical_clock[index]));
		}
	}
	LOG_INFO((">"));
}


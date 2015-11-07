/*
 * logger.c
 *
 *  Created on: Oct 2, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#include <logger.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

FILE *fp = NULL;
FILE *fp1 = NULL;

void log_to_file(char *msg, ...)
{
	va_list	arglist;
	size_t size = 100;
	char buffer[100];

	if(fp == NULL)
	{
		fp = fopen(LOG_FILENAME, "w");
		if(fp==NULL)
			return;
	}
	fprintf(fp, "LOGGER:");

	va_start(arglist,msg);

	vsnprintf(buffer, size, msg, arglist);

	va_end(arglist);

	fwrite(buffer, strlen(buffer), 1, fp);

	fflush(fp);
}

void log_to_gateway_log_file(char *msg, ...)
{
	va_list	arglist;
	size_t size = 100;
	char buffer[100];

	if(fp1 == NULL)
	{
		fp1 = fopen(GATEWAY_LOG_FILENAME, "w");
		if(fp1==NULL)
				return;
	}

	va_start(arglist,msg);

	vsnprintf(buffer, size, msg, arglist);

	va_end(arglist);

	fwrite(buffer, strlen(buffer), 1, fp1);

	fflush(fp1);
}

void logger_close()
{
	if(fp)
		fclose(fp);
	if(fp1)
		fclose(fp1);
}




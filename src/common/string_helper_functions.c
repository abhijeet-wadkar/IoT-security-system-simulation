/*
 * string_helper_functions.c
 *
 *  Created on: Sep 27, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#include <string.h>
#include <malloc.h>

#include "string_helper_functions.h"

void str_copy(char** dest, char* src)
{
	int length = strlen(src);

	*dest = malloc(length + 1);
	strcpy(*dest, src);
}

void str_tokenize(char* str, char *delimiter, char *tokens[], int *count)
{
	int index = 0;
	tokens[index] = strtok(str, delimiter);
	while(tokens[index] != NULL)
	{
		index++;
		tokens[index] = strtok(NULL, delimiter);
	}
	*count = index;
	return;
}


/*
 * queue.c
 *
 *  Created on: Oct 4, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#include <stdio.h>
#include <malloc.h>

#include "queue.h"
#include "error_codes.h"

int add(queue **head, void* elem)
{
	queue *node = NULL;
	queue *temp = NULL;

	node = (queue*)malloc(sizeof(queue));
	if(!node)
	{
		return (E_FAILURE);
	}

	if(*head == NULL)
	{
		*head = node;
		node->next = NULL;
	}

	temp = *head;
	while(temp->next == NULL)
		temp = temp->next;

	temp->next = node;
	node->next = NULL;

	return (E_SUCCESS);
}

void* remove(queue **head)
{
	queue *node = NULL;

	if(*head == NULL)
		return NULL;

	node = *head;
	*head = *head->next;
	return NULL;
}



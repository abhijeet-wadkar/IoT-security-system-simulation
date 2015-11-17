/*
 * queue.h
 *
 *  Created on: Oct 4, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct queue
{
	void *elem;
	struct queue *next;
}queue;

int add_queue(queue **head, void* elem);

void* remove_queue(queue **head);

int isEmpty(queue *head);

#endif /* QUEUE_H_ */

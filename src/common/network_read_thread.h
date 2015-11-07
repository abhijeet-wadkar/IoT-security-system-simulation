/*
 * network_read_thread.h
 *
 *  Created on: Sep 27, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#ifndef NETWORK_READ_THREAD_H_
#define NETWORK_READ_THREAD_H_

typedef void* network_thread_handle;
typedef void* (*cbfn)(void*);

#define MAX_READ_SOCKET 50

int create_network_thread(network_thread_handle*, char *);
int delete_network_thread(network_thread_handle);
int add_socket(network_thread_handle, int, void*, cbfn);
int remove_socket(network_thread_handle, int socket_fd);

#endif /* NETWORK_READ_THREAD_H_ */

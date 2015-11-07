/*
 * network_read_thread.c
 *
 *  Created on: September 27, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>

#include "network_read_thread.h"
#include "error_codes.h"
#include "logger.h"

typedef struct read_socket
{
	int socket_fd;
	void* context;
	cbfn fn;
}read_socket;

typedef struct network_thread_context
{
	int control_socket;
	read_socket read_socket_list[MAX_READ_SOCKET];
	int read_socket_count;
	fd_set socket_list;
	pthread_t thread;
}network_thread_context;

typedef enum network_command_type
{
	ADD = 0,
	REMOVE = 1,
	EXIT = 2,
	UNKNOWN = 3
}network_command_type;

typedef struct network_command
{
	network_command_type type;
	int socket_fd;
}network_command;

void* network_read_thread(void*);

int create_network_thread(network_thread_handle* handle, char *ip_address)
{
	network_thread_context *network_thread = NULL;
	int return_value;
	struct sockaddr_in addr;
	socklen_t addr_size;

	network_thread = (network_thread_context*) malloc(
			sizeof(network_thread_context));
	if (NULL == network_thread) {
		LOG(("Out of memory\n"));
		return (E_OUT_OF_MEMORY);
	}
	memset(network_thread, 0, sizeof(network_thread_context));

	FD_ZERO(&network_thread->socket_list);

	network_thread->control_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (network_thread->control_socket < 0) {
		LOG(("Control socket create error\n"));
		delete_network_thread((network_thread_handle) network_thread);
		return (E_SOCKET_CREATE_ERROR);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(INADDR_ANY);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr_size = sizeof(addr);

	return_value = bind(network_thread->control_socket, (struct sockaddr *)&addr, sizeof(addr));
	if (return_value < 0) {
		LOG(("ERROR in BINDING\n"));
		return (E_SOCKET_CONNCTION_ERORR);
	}

	return_value = getsockname(network_thread->control_socket, (struct sockaddr *)&addr, &addr_size);
	if(return_value <0)
	{
		LOG(("ERROR in BINDING\n"));
		return (E_SOCKET_CONNCTION_ERORR);
	}

	return_value = connect(network_thread->control_socket, (struct sockaddr *)&addr, sizeof(addr));
	if (return_value < 0) {
		LOG(("ERROR in BINDING\n"));
		return (E_SOCKET_CONNCTION_ERORR);
	}

	/* Add control socket to FD set */
	FD_SET(network_thread->control_socket, &network_thread->socket_list);

	pthread_create(&network_thread->thread, NULL, network_read_thread,
			network_thread);

	*handle = network_thread;
	return (E_SUCCESS);
}

int delete_network_thread(network_thread_handle handle)
{
	network_command command;
	network_thread_context *network_thread =(network_thread_context*)handle;
	int send_count = 0;

	if(!handle)
		return (E_FAILURE);

	memset(&command, 0, sizeof(network_command));

	command.type = EXIT;
	if(network_thread->control_socket)
	{
		send_count = send(network_thread->control_socket, &command, sizeof(network_command), 0);
		if(send_count < 0)
		{
			LOG(("Error in control command in network thread"));
		}
		pthread_join(network_thread->thread, NULL);
		close(network_thread->control_socket);
	}

	free(handle);

	return (E_SUCCESS);
}

int add_socket(network_thread_handle handle, int socket_fd, void* context, cbfn fn)
{
	network_thread_context* network_thread = (network_thread_context*) handle;
	network_command command;
	int send_count;

	if(!handle)
	{
		return (E_INVALID_HANDLE);
	}

	if(network_thread->read_socket_count == MAX_READ_SOCKET)
	{
		LOG(("MAX Number of socket reached\n"));
		return (E_FAILURE);
	}

	network_thread->read_socket_list[network_thread->read_socket_count].socket_fd = socket_fd;
	network_thread->read_socket_list[network_thread->read_socket_count].fn = fn;
	network_thread->read_socket_list[network_thread->read_socket_count].context = context;
	network_thread->read_socket_count++;
	command.type = ADD;
	command.socket_fd = socket_fd;

	send_count = send(network_thread->control_socket, &command, sizeof(network_command), 0);
	if(send_count < 0)
	{
		LOG(("Error in control command in network thread"));
		return (E_FAILURE);
	}

	return (E_SUCCESS);
}

int remove_socket(network_thread_handle handle, int socket_fd)
{
	network_thread_context* network_thread = (network_thread_context*) handle;
	network_command command;
	int send_count, index, flag_found;

	if(!handle)
	{
		return (E_INVALID_HANDLE);
	}

	flag_found = 0;
	for(index=0; index<network_thread->read_socket_count; index++)
	{
		if(network_thread->read_socket_list[index].socket_fd == socket_fd)
		{
			flag_found = 1;
			break;
		}
	}

	if(flag_found == 1)
	{
		for(;index<network_thread->read_socket_count-1; index++)
		{
			network_thread->read_socket_list[index] = network_thread->read_socket_list[index+1];
		}
		network_thread->read_socket_count--;
	}

	command.type = REMOVE;
	command.type = socket_fd;
	send_count = write(network_thread->control_socket, &command, sizeof(network_command));
	if(send_count < 0)
	{
		LOG(("Error in control command in network thread"));
		return (E_FAILURE);
	}

	return (E_SUCCESS);
}

network_command_type handle_command(network_thread_context* network_thread)
{
	network_command command = {UNKNOWN, -1};
	int read_count = 0;
	read_count = read(network_thread->control_socket, &command, sizeof(network_command));
	if(read_count < 0)
	{
		return (UNKNOWN);
	}
	switch(command.type)
	{
	case ADD:
		FD_SET(command.socket_fd, &network_thread->socket_list);
		break;
	case REMOVE:
		FD_CLR(command.socket_fd, &network_thread->socket_list);
		break;
	default:
		break;
	}
	return (command.type);
}

void* network_read_thread(void* handle)
{
	network_thread_context* network_thread = (network_thread_context*)handle;
	fd_set socket_list;
	int index = 0;
	int return_value = 0;

	while(1)
	{
		socket_list = network_thread->socket_list;
		return_value = select(FD_SETSIZE, &socket_list, NULL, NULL, NULL);
		if(return_value < 0)
		{
			LOG(("Error in Select()\n"));
			return NULL;
		}

		if(FD_ISSET(network_thread->control_socket, &socket_list))
		{
			return_value = handle_command(network_thread);
			if(return_value == EXIT)
			{
				break;
			}
		}

		for(index=0; index<network_thread->read_socket_count; index++)
		{
			if(FD_ISSET(network_thread->read_socket_list[index].socket_fd, &socket_list))
			{
				network_thread->read_socket_list[index].fn(network_thread->read_socket_list[index].context);
			}
		}
	}
	return (NULL);
}

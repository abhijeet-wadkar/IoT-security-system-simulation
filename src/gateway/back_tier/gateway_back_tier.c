/*
 * gateway.c
 *
 *  Created on: Oct 1, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

#include "gateway.h"
#include "error_codes.h"
#include "logger.h"
#include "network_functions.h"
#include "network_read_thread.h"

char* device_string[] = {
		"door_sensor",
		"motion_sensor",
		"key_chain_sensor",
		"security_device",
		"gateway",
		"back_tier_gateway",
		"unknown"};

char* state_string[] = {
		"off",
		"on"
};

void* read_callback(void*);

void print_state(gateway_context *gateway)
{
	int index;
	for(index=0; index<gateway->client_count; index++)
	{
		gateway_client *client = gateway->clients[index];
		if(client->type == SECURITY_DEVICE)
		{
			fprintf(gateway->storage_file_pointer,
					"%d----%s:%s----%s----%s----%s\n",
					(int)(client&&0xFFFF),
					client->client_ip_address,
					client->client_port_number,
					device_string[client->type],
					client->area_id,
					state_string[client->state]);
		}
		else
		{
			fprintf(gateway->storage_file_pointer,
					"%d----%s:%s----%s----%s----%d\n",
					(int)(client&&0xFFFF),
					client->client_ip_address,
					client->client_port_number,
					device_string[client->type],
					client->area_id,
					client->value);
		}
	}
}

int create_gateway(gateway_handle* handle, gateway_create_params *params)
{
	gateway_context *gateway = NULL;
	int return_value = 0;

	gateway = (gateway_context*)malloc(sizeof(gateway_context));
	if(NULL == gateway)
	{
		LOG_ERROR(("ERROR: Out of memory\n"));
		return (E_OUT_OF_MEMORY);
	}
	memset(gateway, 0, sizeof(gateway_context));

	gateway->client_count = 0;

	gateway->storage_file_pointer = fopen(params->storage_file_name, "w");
	if(!gateway->storage_file_pointer)
	{
		LOG_ERROR(("ERROR: Unable to open persistent storage file\n"));
		delete_gateway((gateway_handle)gateway);
		return E_FAILURE;
	}

	/* create network read thread */
	return_value = create_network_thread(&gateway->network_thread, params->gateway_ip_address);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in creating n/w read thread\n"));
		delete_gateway((gateway_handle)gateway);
		return (return_value);
	}

	/* create connection to server */
	return_value = create_socket(&gateway->server_socket_fd, params->gateway_ip_address, params->gateway_port_no);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in creating the socket\n"));
		delete_gateway((gateway_handle)gateway);
		return (return_value);
	}

	/* add socket to network read thread */
	return_value = add_socket(gateway->network_thread, gateway->server_socket_fd,  (void*)gateway, &read_callback);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: add_socket() failed\n"));
		delete_gateway((gateway_handle)gateway);
		return (return_value);
	}
	message msg;

	/* register device with gateway */
	msg.type = REGISTER;
	msg.u.s.type = BACK_TIER_GATEWAY;
	msg.u.s.ip_address = params->gateway_ip_address;
	msg.u.s.port_no = params->gateway_port_no;
	msg.u.s.area_id = "0";

	return_value = write_message(gateway->server_socket_fd, gateway->logical_clock, &msg);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in registering device\n"));
		return (E_FAILURE);
	}
	*handle = gateway;
	return (E_SUCCESS);
}
void delete_gateway(gateway_handle handle)
{
	/* release all the resources */
	gateway_context* gateway = (gateway_context*)handle;
	int index;

	if(gateway)
	{
		for(index=0; index<gateway->client_count; index++)
		{
			remove_socket(gateway->network_thread, gateway->clients[index]->comm_socket_fd);
			if(gateway->clients[index]->client_ip_address)
				free(gateway->clients[index]->client_ip_address);
			if(gateway->clients[index]->client_port_number)
				free(gateway->clients[index]->client_port_number);
			if(gateway->clients[index]->area_id)
				free(gateway->clients[index]->area_id);
			free(gateway->clients[index]);
		}

		if(gateway->network_thread)
		{
			delete_network_thread(gateway->network_thread);
		}
		if(gateway->server_socket_fd)
		{
			close_socket(gateway->server_socket_fd);
		}

		free(gateway);
	}
}

void* read_callback(void *context)
{
	gateway_context *gateway = (gateway_context*)context;
	int return_value = 0;
	char *string = NULL;

	return_value = read_msg_from_frontend(gateway->server_socket_fd, &string);
	if(return_value != E_SUCCESS)
	{
		LOG_ERROR(("ERROR: Error in read message\n"));
		LOG_ERROR(("ERROR: Connection closed from peer\n"));
		exit(0);
		return (NULL);
	}

	if(gateway->storage_file_pointer)
	{
		fprintf(gateway->storage_file_pointer, "%s", string);
		fflush(gateway->storage_file_pointer);
		free(string);
	}

	return (NULL);
}

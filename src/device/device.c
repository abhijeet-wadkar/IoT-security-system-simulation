/*
 * device.c
 *
 *  Created on: Oct 1, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include "device.h"
#include "error_codes.h"
#include "logger.h"
#include "network_functions.h"
#include "message.h"
#include "logical_clock_utils.h"

static void* read_callback(void *context);

int create_device(device_handle *handle, device_create_params *params)
{
	device_context *device = NULL;
	int return_value = E_FAILURE;

	device = (device_context*)malloc(sizeof(device_context));
	if(NULL == device)
	{
		LOG_ERROR(("ERROR: Out of memory\n"));
		delete_device((device_handle)device);
		return (E_OUT_OF_MEMORY);
	}

	memset(device, 0, sizeof(device_context));
	device->device_params = params;
	device->state = 0;

	/* create network read thread */
	return_value = create_network_thread(&device->network_thread, params->device_ip_address);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in creating n/w read thread\n"));
		delete_device((device_handle)device);
		return (return_value);
	}

	/* create connection to server */
	return_value = create_socket(&device->socket_fd, params->gateway_ip_address, params->gateway_port_no);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Connection to Server failed\n"));
		delete_device((device_handle)device);
		return (return_value);
	}

	/* add socket to network read thread */
	return_value = add_socket(device->network_thread, device->socket_fd,  (void*)device, &read_callback);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Connection to Server failed\n"));
		delete_device((device_handle)device);
		return (return_value);
	}

	message msg;

	/* register device with gateway */
	msg.type = REGISTER;
	msg.u.s.type = SECURITY_DEVICE;
	msg.u.s.ip_address = device->device_params->device_ip_address;
	msg.u.s.port_no = device->device_params->device_port_no;
	msg.u.s.area_id = device->device_params->device_area_id;

	return_value = write_message(device->socket_fd, device->logical_clock, &msg);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in registering device\n"));
		return (E_FAILURE);
	}

	*handle = device;
	return (E_SUCCESS);
}

void delete_device(device_handle handle)
{
	/* release all the resources */
	device_context* device = (device_context*)handle;

	if(device)
	{
		if(device->network_thread)
		{
			delete_network_thread(device->network_thread);
		}
		if(device->socket_fd)
		{
			close_socket(device->socket_fd);
		}

		free(device);
	}
}

static void* read_callback(void *context)
{
	device_context *device = (device_context*)context;
	int return_value = 0;
	message msg;

	return_value = read_message(device->socket_fd, device->logical_clock, &msg);
	if(return_value != E_SUCCESS)
	{
		if(return_value == E_SOCKET_CONNECTION_CLOSED)
		{
			LOG_ERROR(("ERROR: Socket connection from server closed...\n"));
			exit(0);
		}
		LOG_ERROR(("ERROR: Error in read message\n"));
		return NULL;
	}

	switch(msg.type)
	{
	case SWITCH:
		LOG_INFO(("INFO: SWITCH "));
		if(msg.u.value==0)
		{
			LOG_INFO(("off"));
		}
		else
		{
			LOG_INFO(("on"));
		}
		LOG_INFO((" message received\n"));

		break;
	default:
		LOG_INFO(("INFO: Unknown/Unsupported message was received\n"));
		break;
	}

	return NULL;
}

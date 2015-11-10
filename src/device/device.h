/*
 * device.h
 *
 *  Created on: Sep 22, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <pthread.h>

#include "network_read_thread.h"
#include "message.h"

typedef void* device_handle;

typedef struct device_create_params
{
	char *device_ip_address;
	char *device_port_no;
	char *device_area_id;
	char *gateway_ip_address;
	char *gateway_port_no;
}device_create_params;

typedef struct device_context
{
	device_create_params *device_params;
	int socket_fd;
	network_thread_handle network_thread;
	int state;
	int logical_clock[CLOCK_SIZE];
}device_context;

int create_device(device_handle *handle, device_create_params *params);
void delete_device(device_handle);

#endif /* DEVICE_H_ */

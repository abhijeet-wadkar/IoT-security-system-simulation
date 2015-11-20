/*
 * sensor.h
 *
 *  Created on: Sep 22, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include <pthread.h>
#include "message.h"

#include "network_read_thread.h"

typedef void* sensor_handle;
typedef struct sensor_context sensor_context;

typedef struct sensor_create_params
{
	char *sensor_ip_address;
	char *sensor_port_no;
	char *sensor_area_id;
	char *gateway_ip_address;
	char *gateway_port_no;
	char *sensor_value_file_name;
}sensor_create_params;

typedef struct peer
{
	char *ip_address;
	char *port_no;
	int comm_socket_fd;
	sensor_context *sensor;
}peer;

struct sensor_context
{
	sensor_create_params *sensor_params;
	int interval;
	int socket_fd;
	network_thread_handle network_thread;
	pthread_t set_value_thread;
	int clock;
	int value;
	int run;
	FILE *sensor_value_file_pointer;
	peer *recv_peer[10];
	peer *send_peer[10];
	int server_socket_fd;
	int recv_peer_count;
	int send_peer_count;
	int logical_clock[CLOCK_SIZE];
	pthread_mutex_t mutex_lock;
};

int create_sensor(sensor_handle *handle, sensor_create_params *params);
void delete_sensor(sensor_handle);

#endif /* SENSOR_H_ */

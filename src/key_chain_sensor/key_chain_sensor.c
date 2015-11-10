/*
 * sensor.c
 *
 *  Created on: Sep 27, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "common.h"
#include "key_chain_sensor.h"
#include "error_codes.h"
#include "logger.h"
#include "network_functions.h"
#include "message.h"
#include "string_helper_functions.h"
#include "logical_clock_utils.h"

static void* accept_callback(void *context);
static void* read_callback(void *context);
static void* set_value_thread(void *context);
void sighand(int signo);

int create_sensor(sensor_handle *handle, sensor_create_params *params)
{
	sensor_context *sensor = NULL;
	int return_value = E_FAILURE;

	sensor = (sensor_context*)malloc(sizeof(sensor_context));
	if(NULL == sensor)
	{
		delete_sensor((sensor_handle)sensor);
		LOG_ERROR(("ERROR: Out of memory\n"));
		return (E_OUT_OF_MEMORY);
	}

	memset(sensor, 0, sizeof(sensor_context));
	sensor->interval = 5;
	sensor->sensor_params = params;
	sensor->clock = 0;
	sensor->value = 0;
	sensor->run = 1;
	sensor->recv_peer_count = 0;
	sensor->send_peer_count = 0;

	sensor->sensor_value_file_pointer = fopen(params->sensor_value_file_name, "r");
	if(!sensor->sensor_value_file_pointer)
	{
		LOG_ERROR(("Unable to open sensor value input file\n"));
		delete_sensor(sensor);
		return (E_FAILURE);
	}

	/* create network read thread */
	return_value = create_network_thread(&sensor->network_thread, params->sensor_ip_address);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in creating n/w read thread\n"));
		delete_sensor((sensor_handle)sensor);
		return (return_value);
	}

	/* create connection to server */
	return_value = create_server_socket(&sensor->server_socket_fd, params->sensor_ip_address, params->sensor_port_no);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in creating the socket\n"));
		delete_sensor((sensor_handle)sensor);
		return (return_value);
	}

	/* add socket to network read thread */
	return_value = add_socket(sensor->network_thread, sensor->server_socket_fd,  (void*)sensor, &accept_callback);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: add_socket() failed\n"));
		delete_sensor((sensor_handle)sensor);
		return (return_value);
	}

	/* create connection to server */
	return_value = create_socket(&sensor->socket_fd, params->gateway_ip_address, params->gateway_port_no);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Connection to Server failed\n"));
		delete_sensor((sensor_handle)sensor);
		return (return_value);
	}

	/* add socket to network read thread */
	return_value = add_socket(sensor->network_thread, sensor->socket_fd,  (void*)sensor, &read_callback);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: add_socket() filed\n"));
		delete_sensor((sensor_handle)sensor);
		return (return_value);
	}

	message msg;

	/* register sensor with gateway */
	msg.type = REGISTER;
	msg.u.s.type = MOTION_SENSOR;
	msg.u.s.ip_address = sensor->sensor_params->sensor_ip_address;
	msg.u.s.port_no = sensor->sensor_params->sensor_port_no;
	msg.u.s.area_id = sensor->sensor_params->sensor_area_id;

	return_value = write_message(sensor->socket_fd, sensor->logical_clock, &msg);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in registering sensor\n"));
		return (E_FAILURE);
	}

	struct sigaction        actions;
	memset(&actions, 0, sizeof(actions));
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0;
	actions.sa_handler = sighand;
	sigaction(SIGALRM,&actions,NULL);
	pthread_create(&sensor->set_value_thread, NULL, &set_value_thread, sensor);

	*handle = sensor;
	return (E_SUCCESS);
}

void delete_sensor(sensor_handle handle)
{
	/* release all the resources */
	sensor_context* sensor = (sensor_context*)handle;

	if(sensor)
	{
		if(sensor->network_thread)
		{
			delete_network_thread(sensor->network_thread);
		}
		if(sensor->socket_fd)
		{
			close_socket(sensor->socket_fd);
		}
		if(sensor->set_value_thread)
		{
			sensor->run = 0;
			pthread_kill(sensor->set_value_thread, SIGALRM);
			pthread_join(sensor->set_value_thread, NULL);
		}

		free(sensor);
	}
}

static void* accept_callback(void *context)
{
	int return_value = 0;
	sensor_context *sensor = NULL;
	peer *client = NULL;

	sensor = (sensor_context*)context;

	client = (peer*)malloc(sizeof(peer));
	if(!client)
	{
		LOG_DEBUG(("DEBUG: Out of memory\n"));
		return (NULL);
	}

	client->sensor = context;
	client->comm_socket_fd = accept(sensor->server_socket_fd, (struct sockaddr*)NULL, NULL);
	if(client->comm_socket_fd < 0)
	{
		LOG_ERROR(("ERROR: Accept call failed\n"));
		free(client);
		return NULL;
	}

	sensor->recv_peer[sensor->recv_peer_count] = client;
	sensor->recv_peer_count++;

	/* add socket to network read thread */
	return_value = add_socket(sensor->network_thread, client->comm_socket_fd,  (void*)client, &read_callback);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: add_socket() failed\n"));
		free(client);
		return (NULL);
	}
	//client->connection_state = 1;
	return (NULL);
}

static void* read_callback(void *context)
{
	sensor_context *sensor = (sensor_context*)context;
	int return_value = 0;
	message msg;
	peer *client = NULL;
	int msg_logical_clock[CLOCK_SIZE];

	return_value = read_message(sensor->socket_fd, msg_logical_clock, &msg);
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

	LOG_INFO(("INFO: msg clock"));
	print_logical_clock(msg_logical_clock);
	adjust_clock(sensor->logical_clock, msg_logical_clock);
	print_logical_clock(sensor->logical_clock);

	switch(msg.type)
	{
	case SET_INTERVAL:
		LOG_INFO(("INFO: SetInterval message received\n"));
		sensor->interval = msg.u.value;
		break;
	case REGISTER:
		LOG_INFO(("INFO: Register received from gateway\n"));

		client = (peer*)malloc(sizeof(peer));
		if(!client)
		{
			LOG_ERROR(("ERROR: Out of memory"));
			return (NULL);
		}

		client->ip_address = msg.u.s.ip_address;
		client->port_no = msg.u.s.port_no;
		/* create connection to server */
		return_value = create_socket(&client->comm_socket_fd,
				msg.u.s.ip_address,
				msg.u.s.port_no);
		if(E_SUCCESS != return_value)
		{
			LOG_ERROR(("ERROR: Connection to Server failed\n"));
		}

		/* add socket to network read thread */
		return_value = add_socket(sensor->network_thread,
				client->comm_socket_fd,
				(void*)client,
				&read_callback);
		if(E_SUCCESS != return_value)
		{
			LOG_ERROR(("ERROR: add_socket() filed\n"));
		}
		sensor->send_peer[sensor->send_peer_count] = client;
		sensor->send_peer_count++;
		break;
	default:
		LOG_INFO(("INFO: Unknown/Unhandled message was received\n"));
		break;
	}

	return NULL;
}

void sighand(int signo)
{
	LOG_DEBUG(("DEBUG: EXITING SET_VALUE_THREAD\n"));
}

void* set_value_thread(void *context)
{
	sensor_context *sensor = NULL;
	message msg;
	int return_value;
	char *tokens[10];
	char line[LINE_MAX];
	int count = 0;
	int start, end, value;

	sensor = (sensor_context*)context;

	msg.type = CURRENT_VALUE;
	while(sensor->run)
	{
		if(!(start <= sensor->clock && sensor->clock < end))
		{
			/* Figure out the value from file */
			if(fgets(line, LINE_MAX, sensor->sensor_value_file_pointer) == NULL)
			{
				LOG_DEBUG(("DEBUG: Seeking to beginning of file"));
				rewind(sensor->sensor_value_file_pointer);
				sensor->clock = 0;
				continue;
			}

			str_tokenize(line, ";\n\r", tokens, &count);
			if(count != 3)
			{
				LOG_ERROR(("ERROR: Wrong sensor temperature value file\n"));
				break;
			}

			start = atoi(tokens[0]);
			end = atoi(tokens[1]);
			if(strcmp (tokens[2], "true") == 0)
			{
				value = 1; 
			}
			else
			{
				value = 0;
			}
			sensor->value = value;
		}

		msg.u.value = sensor->value;

		sensor->logical_clock[2]++;
		print_logical_clock(sensor->logical_clock);
		LOG_INFO(("INFO: Sending temperature value %d to gateway\n", sensor->value));
		return_value = write_message(sensor->socket_fd, sensor->logical_clock, &msg);
		if(E_SUCCESS != return_value)
		{
			LOG_ERROR(("ERROR: Error in sending sensor temperature value to gateway\n"));
		}

		for(int index=0; index<sensor->send_peer_count; index++)
		{
			return_value = write_message(sensor->send_peer[index]->comm_socket_fd,
					sensor->logical_clock,
					&msg);
			if(E_SUCCESS != return_value)
			{
				LOG_ERROR(("ERROR: Error in sending sensor temperature value to peer\n"));
			}
		}

		LOG_INFO(("INFO: Sleeping for %d second(s)\n", sensor->interval));
		sleep(sensor->interval);
		sensor->clock += sensor->interval;
	}

	LOG_DEBUG(("Exiting SetValueThread...\n"));
	return (NULL);
}

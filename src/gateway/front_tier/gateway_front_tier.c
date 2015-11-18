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
#include <queue.h>

#include "gateway.h"
#include "error_codes.h"
#include "logger.h"
#include "network_functions.h"
#include "network_read_thread.h"
#include "logical_clock_utils.h"

char* device_string[] = {
		"door",
		"motion",
		"key_chain",
		"security_device",
		"gateway",
		"back_tier_gateway",
		"unknown"};

char* state_string[] = {
		"off",
		"on"
};

char *value_string[] = {
		"false",
		"true"
};

char *door_string[] = {
		"close",
		"open"
};

typedef struct message_context
{
	gateway_client *client;
	message *msg;
}message_context;

void* accept_callback(void *context);
void* read_callback(void*);
void* message_handler(void*);
void print_state(gateway_context *gateway);

void* message_handler(void *context)
{
	message_context *msg_context = NULL;
	gateway_client *client = NULL;
	gateway_context *gateway = (gateway_context*)context;
	message *msg = NULL;
	int return_value;
	char buffer[100] = {'\0'};

	while(1)
	{
		pthread_mutex_lock(&gateway->mutex_lock);
		if(isEmpty(gateway->msg_queue))
		{
			pthread_cond_wait(&gateway->cond_lock, &gateway->mutex_lock);
		}

		/* Process message */
		msg_context = remove_queue(&gateway->msg_queue);
		client = msg_context->client;
		msg = msg_context->msg;

		switch(msg->type)
		{
		case SET_INTERVAL:
			LOG_DEBUG(("DEBUG: SetInterval message received, Value: %d\n", msg->u.value));
			break;
		case REGISTER:
			LOG_DEBUG(("DEBUG: Register message received\n"));
			client->type = msg->u.s.type;
			client->client_ip_address = msg->u.s.ip_address;
			client->client_port_number = msg->u.s.port_no;
			client->area_id = msg->u.s.area_id;
			LOG_DEBUG(("DEBUG: DeviceType:%d\n", client->type));
			LOG_DEBUG(("DEBUG: IP Address: %s\n", client->client_ip_address));
			LOG_DEBUG(("DEBUG: Port Number: %s\n", client->client_port_number));
			LOG_DEBUG(("DEBUG: Area Id: %s\n", client->area_id));
			if(client->type == DOOR_SENSOR ||
				client->type == MOTION_SENSOR ||
				client->type == KEY_CHAIN_SENSOR)
			{
				client->state = 1;
			}
			if(client->type == SECURITY_DEVICE)
			{
				client->state = 0;
			}

			// Check if all the components of the system are connected to the gateway
			if (gateway->client_count == 4)
			{
				for(int index=0; index < gateway->client_count; index++)
				{
					if(gateway->clients[index]->type != BACK_TIER_GATEWAY && gateway->clients[index]->type != SECURITY_DEVICE)
					{
						for(int index1=0; index1<gateway->client_count; index1++)
						{
							if(0 != strcmp(gateway->clients[index]->client_port_number, gateway->clients[index1]->client_port_number)
									&& gateway->clients[index1]->type != BACK_TIER_GATEWAY
									&& gateway->clients[index1]->type != SECURITY_DEVICE)
							{
								message msg;
								msg.type = REGISTER;
								msg.u.s.type = gateway->clients[index1]->type;
								msg.u.s.ip_address = gateway->clients[index1]->client_ip_address;
								msg.u.s.port_no = gateway->clients[index1]->client_port_number;
								msg.u.s.area_id = gateway->clients[index1]->area_id;
								LOG_INFO(("INFO: Sending Clock\n"));
								print_logical_clock(gateway->logical_clock);
								return_value = write_message(gateway->clients[index]->comm_socket_fd, gateway->logical_clock, &msg);
								if (E_SUCCESS != return_value)
								{
									LOG_ERROR(("ERROR: unable to send the message\n"));
								}
							}
						}
					}
				}
			}
			break;
		case CURRENT_VALUE:
			LOG_DEBUG(("Current value message received\n"));
			LOG_DEBUG(("Value: %d\n", msg->u.value));

			client->value = msg->u.value;

			if(msg->u.s.type == MOTION_SENSOR)
			{
				sprintf(buffer, "%d----%s----%s----%lu----%s----%s\n",
					(int)(client&&0xFFFF),
					device_string[client->type],
					value_string[msg->u.value],
					msg->timestamp,
					client->client_ip_address,
					client->client_port_number);
				if((gateway->motion_state != msg->u.value) && (gateway->motion_state = msg->u.value))
				{
					if(gateway->key_state == 0)
					{
						LOG_INFO(("INFO: Security ALert - Raise the Alarm\n"));
					}
					else
					{
						if(gateway->door_state == 1)
						{
							LOG_INFO(("INFO: User Entered Home\n"));
						}
					}
				}
			}
			else if(msg->u.s.type == DOOR_SENSOR)
			{
				sprintf(buffer, "%d----%s----%s----%lu----%s----%s\n",
					(int)(client&&0xFFFF),
					device_string[client->type],
					door_string[msg->u.value],
					msg->timestamp,
					client->client_ip_address,
					client->client_port_number);
				if(gateway->door_state != msg->u.value)
				{
					gateway->door_state = msg->u.value;
					if(gateway->key_state == 0 )
					{
						LOG_INFO(("INFO: Security ALert - Raise the Alarm\n"));
					}
					else
					{
						if(gateway->door_state == 0 && gateway->motion_state == 1)
						{
							LOG_INFO(("INFO: User Exited Home\n"));
						}
					}
				}
			}
			else if(msg->u.s.type == KEY_CHAIN_SENSOR)
			{
				sprintf(buffer, "%d----%s----%s----%lu----%s----%s\n",
					(int)(client&&0xFFFF),
					device_string[client->type],
					value_string[msg->u.value],
					msg->timestamp,
					client->client_ip_address,
					client->client_port_number);
				gateway->key_state = msg->u.value;
			}

			LOG_INFO(("INFO: %s\n", buffer));
			print_state(client->gateway);

			for(int index=0; index<gateway->client_count; index++)
			{
				if(gateway->clients[index]->type == BACK_TIER_GATEWAY)
				{
					return_value = send_msg_to_backend(gateway->clients[index]->comm_socket_fd, buffer);
					if(E_SUCCESS != return_value)
					{
						LOG_ERROR(("ERROR: Unable to send message to back-end\n"));
					}
				}
			}
			break;
		case CURRENT_STATE:
			LOG_DEBUG(("DEBUG: Current state message is received\n"));
			//client->state = msg->u.value;
			//print_state(client->gateway);
			break;
		default:
			LOG_DEBUG(("Unknown/Unhandled message is received\n"));
			break;
		}

		pthread_mutex_unlock(&gateway->mutex_lock);
	}
	return (NULL);
}

void print_state(gateway_context *gateway)
{
	int index;

	LOG_GATEWAY(("-----------------------------------------------\n"));
	for(index=0; index<gateway->client_count; index++)
	{
		/* Need to open this code

		if(gateway->clients[index].type == BACK_TIER_GATEWAY)
		{
			create message and send it the back end for inserting into the file
		}*/

		gateway_client *client = gateway->clients[index];
		if(client->type == SECURITY_DEVICE)
		{
			LOG_GATEWAY(("%d----%s:%s----%s----%s----%s\n",
									(int)(client&&0xFFFF),
									client->client_ip_address,
									client->client_port_number,
									device_string[client->type],
									client->area_id,
									state_string[client->state]));
		}
		else
		{
			LOG_GATEWAY(("%d----%s:%s----%s----%s----%d\n",
							(int)(client&&0xFFFF),
							client->client_ip_address,
							client->client_port_number,
							device_string[client->type],
							client->area_id,
							client->value));
		}
	}
	LOG_GATEWAY(("-----------------------------------------------\n"));
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
	gateway->logical_clock[0] = 0;
	gateway->logical_clock[1] = 0;
	gateway->logical_clock[2] = 0;
	gateway->logical_clock[3] = 0;

	memset(gateway->buffered_messages, 0, 100*sizeof(void*));
	gateway->msg_queue = NULL;

	pthread_mutex_init(&gateway->mutex_lock, NULL);
	pthread_cond_init(&gateway->cond_lock, NULL);

	/* create network read thread */
	return_value = create_network_thread(&gateway->network_thread, params->gateway_ip_address);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in creating n/w read thread\n"));
		delete_gateway((gateway_handle)gateway);
		return (return_value);
	}

	/* create connection to server */
	return_value = create_server_socket(&gateway->server_socket_fd, params->gateway_ip_address, params->gateway_port_no);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Error in creating the socket\n"));
		delete_gateway((gateway_handle)gateway);
		return (return_value);
	}

	/* add socket to network read thread */
	return_value = add_socket(gateway->network_thread, gateway->server_socket_fd,  (void*)gateway, &accept_callback);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: add_socket() failed\n"));
		delete_gateway((gateway_handle)gateway);
		return (return_value);
	}

	pthread_create(&gateway->message_handler_thread, NULL, message_handler, gateway);
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

void* accept_callback(void *context)
{
	int return_value = 0;
	gateway_context *gateway = NULL;
	gateway_client *client = NULL;

	gateway = (gateway_context*)context;

	client = (gateway_client*)malloc(sizeof(gateway_client));
	if(!client)
	{
		LOG_DEBUG(("DEBUG: Out of memory\n"));
		return (NULL);
	}

	client->gateway = context;
	client->comm_socket_fd = accept(gateway->server_socket_fd, (struct sockaddr*)NULL, NULL);
	if(client->comm_socket_fd < 0)
	{
		LOG_ERROR(("ERROR: Accept call failed\n"));
		free(client);
		return NULL;
	}

	gateway->clients[gateway->client_count] = client;
	gateway->client_count++;

	/* add socket to network read thread */
	return_value = add_socket(gateway->network_thread, client->comm_socket_fd,  (void*)client, &read_callback);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: add_socket() failed\n"));
		free(client);
		return (NULL);
	}
	client->connection_state = 1;

	return (NULL);
}

void* read_callback(void *context)
{
	gateway_client *client = (gateway_client*)context;
	gateway_context *gateway = client->gateway;
	int return_value = 0;
	message *msg = NULL;
	message_context *msg_context = NULL;
	//message snd_msg;
	int index;
	int flag_found = 0;
	int msg_logical_clock[CLOCK_SIZE];
	int buffer_message;
	int signal_message_handler;

	msg_context = (message_context*)malloc(sizeof(message_context));
	if(!msg_context)
	{
		LOG_ERROR(("Out of Memory\n"));
		return (NULL);
	}
	msg = (message*)malloc(sizeof(message));
	if(!msg)
	{
		LOG_ERROR(("Out of Memory\n"));
		return (NULL);
	}

	msg_context->client = client;
	msg_context->msg = msg;

	return_value = read_message(client->comm_socket_fd, msg_logical_clock, msg);
	if(return_value != E_SUCCESS)
	{
		if(return_value == E_SOCKET_CONNECTION_CLOSED)
		{
			LOG_ERROR(("ERROR: Connection closed for client: %s-%s-%s...\n",
					client->client_ip_address,
					client->client_port_number,
					client->area_id));
			remove_socket(client->gateway->network_thread, client->comm_socket_fd);
			client->connection_state = 0;
			LOG_ERROR(("ERROR: Connection closed for client\n"));
			index = 0;
			flag_found = 0;
			for(index=0; index<gateway->client_count; index++)
			{
				if(gateway->clients[index]->comm_socket_fd == client->comm_socket_fd)
				{
					flag_found = 1;
					break;
				}
			}
			if(flag_found == 1)
			{
				for(;index<gateway->client_count-1; index++)
				{
					gateway->clients[index] = gateway->clients[index+1];
				}
				gateway->client_count--;
			}
			if(client->client_ip_address)
				free(client->client_ip_address);
			if(client->client_port_number)
				free(client->client_port_number);
			if(client->area_id)
				free(client->area_id);
			free(client);
			return NULL;
		}
		LOG_ERROR(("ERROR: Error in read message, error: %d\n", return_value));
		return NULL;
	}

	pthread_mutex_lock(&gateway->mutex_lock);

	buffer_message = 0;
	signal_message_handler = 0;
	//adjust_clock(gateway->logical_clock, msg_logical_clock);
	if(client->type == MOTION_SENSOR)
	{
		if(check_devlivery(gateway->logical_clock, msg_logical_clock))
		{
			/* correct order message */
			/* adjust clock */
			add_queue(&gateway->msg_queue, msg_context);
			adjust_clock(gateway->logical_clock, msg_logical_clock);
			signal_message_handler = 1;
		}
		else
		{
			/* buffer message */
			buffer_message = 1;
		}
	}
	else if(client->type == DOOR_SENSOR)
	{
		if(check_devlivery(gateway->logical_clock, msg_logical_clock))
		{
			/* correct order message */
			/* adjust clock */
			add_queue(&gateway->msg_queue, msg_context);
			adjust_clock(gateway->logical_clock, msg_logical_clock);
			signal_message_handler = 1;
		}
		else
		{
			/* buffer message */
			buffer_message = 1;
		}
	}
	else if(client->type == KEY_CHAIN_SENSOR)
	{
		if(check_devlivery(gateway->logical_clock, msg_logical_clock))
		{
			/* correct order message */
			/* adjust clock */
			add_queue(&gateway->msg_queue, msg_context);
			adjust_clock(gateway->logical_clock, msg_logical_clock);
			signal_message_handler = 1;
		}
		else
		{
			/* buffer message */
			buffer_message = 1;
		}
	}

	if(buffer_message)
	{
		for(int index=0; index<100; index++)
		{
			if(gateway->buffered_messages[index] == NULL)
			{
				gateway->buffered_messages[index] = msg_context;
				break;
			}
		}
	}

	/* Check other buffered message */
	for(int index=0; index<100; index++)
	{
		if(gateway->buffered_messages[index])
		{
			message_context *temp_message = (message_context*)gateway->buffered_messages[index];
			if(check_devlivery(gateway->logical_clock, temp_message->msg->logical_clock))
			{
				add_queue(&gateway->msg_queue, temp_message);
				adjust_clock(gateway->logical_clock, temp_message->msg->logical_clock);
				gateway->buffered_messages[index] = NULL;
				index = 0;
				signal_message_handler = 1;
			}
		}
	}

	if(signal_message_handler)
	{
		pthread_cond_signal(&gateway->cond_lock);
	}

	LOG_INFO(("INFO: Event Received: "));
	print_logical_clock(gateway->logical_clock);
	LOG_INFO((", timestamp: %lu, From %s:%s\n", msg->timestamp, client->client_ip_address, client->client_port_number));

	pthread_mutex_unlock(&gateway->mutex_lock);

	return (NULL);
}

int set_interval(gateway_handle handle, int index, int interval)
{
	gateway_context *gateway = handle;
	message snd_msg;

	gateway->logical_clock[3]++;

	if(0 <= index && index < gateway->client_count)
	{
		snd_msg.type = SET_INTERVAL;
		snd_msg.u.value = interval;
		LOG_INFO(("INFO: Sending Clock\n"));
		print_logical_clock(gateway->logical_clock);
		return (write_message(gateway->clients[index]->comm_socket_fd, gateway->logical_clock, &snd_msg));
	}
	else
	{
		LOG_ERROR(("ERROR: Such sensor don't exists\n"));
	}
	return (E_FAILURE);
}

void print_sensors(gateway_handle handle)
{
	int index;
	gateway_context *gateway = handle;

	printf("----------------List of sensor---------------\n");
	printf("<ID>-<IP_Address>-<Port_Number>-<AreaId>\n");
	for(index=0; index<gateway->client_count; index++)
	{
		if(gateway->clients[index]->type == DOOR_SENSOR)
		{
			printf("%d:%s-%s-%s\n",
					index,
					gateway->clients[index]->client_ip_address,
					gateway->clients[index]->client_port_number,
					gateway->clients[index]->area_id);
		}
	}
}

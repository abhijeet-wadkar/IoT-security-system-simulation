/*
 * network_functions.c
 *
 *  Created on: Sep 27, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include "network_functions.h"
#include "error_codes.h"
#include "logger.h"
#include "message.h"
#include "string_helper_functions.h"

int create_socket(int *socket_fd, char *ip_address, char* port_no)
{
	struct sockaddr_in serv_addr;

	*socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(*socket_fd < 0)
	{
		LOG(("Create socket error\n"));
		return (E_SOCKET_CREATE_ERROR);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port_no));

	if(inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0)
	{
		LOG(("inet_pton() failed\n"));
		return (E_FAILURE);
	}

	if(connect(*socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))	< 0)
	{
		LOG(("Connect Failed\n"));
		return (E_FAILURE);
	}
	return (E_SUCCESS);
}

int create_server_socket(int *socket_fd, char *ip_address, char* port_no)
{
	struct sockaddr_in serv_addr;

	*socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(*socket_fd < 0)
	{
		LOG(("Create socket error\n"));
		return (E_SOCKET_CREATE_ERROR);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port_no));

	if(inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0)
	{
		LOG(("inet_pton() failed\n"));
		return (E_FAILURE);
	}

	if(bind(*socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		LOG(("error in bind\n"));
		return (E_FAILURE);
	}

	if(listen(*socket_fd, 10) < 0)
	{
		LOG(("error in listen\n"));
		return (E_FAILURE);
	}

	return (E_SUCCESS);
}

void close_socket(int socket_fd)
{
	close(socket_fd);
	socket_fd = -1;
	return;
}

device_type get_device_type(char *str)
{
	if(strcmp(str, "door_sensor")==0)
	{
		return (DOOR_SENSOR);
	}
	else if(strcmp(str, "key_chain_sensor")==0)
	{
		return (KEY_CHAIN_SENSOR);
	}
	else if(strcmp(str, "motion_sensor")==0)
	{
		return (MOTION_SENSOR);
	}
	else if(strcmp(str, "security_device")==0)
	{
		return (SECURITY_DEVICE);
	}
	else if(strcmp(str, "gateway")==0)
	{
		return (GATEWAY);
	}
	return (UNKNOWN);
}

void send_socket(int socket_fd, char* data, int length)
{
	int send_count = 0;

	send_count = write(socket_fd, data, length);
	if(send_count < 0)
	{
		LOG(("Error in sending data\n"));
	}
}

int read_message(int socket_fd, message *msg)
{
	int read_count = 0;
	int msg_size = 0;
	char buffer[60] = {'\0'};
	char *tokens[10] = {NULL};
	int count = 0;
	char *register_tokens[10] = {NULL};

	/*read length */
	while(1)
	{
		read_count += read(socket_fd, &msg_size, sizeof(int) - read_count);
		if(read_count==0)
		{
			return (E_SOCKET_CONNECTION_CLOSED);
		}
		if(read_count == sizeof(int))
			break;
	}

	/*read the message */
	read_count = 0;
	while(1)
	{
		read_count += read(socket_fd, &buffer, msg_size - read_count);
		if(read_count==0)
		{
			return (E_SOCKET_CONNECTION_CLOSED);
		}
		if(read_count == msg_size)
			break;
	}

	str_tokenize(buffer, ":;", tokens, &count);
	if(count != 4)
	{
		return (E_INVALID_MESSAGE);
	}

	if(strcmp(tokens[0],"type")!=0 || strcmp(tokens[2], "action")!=0)
	{
		return (E_INVALID_MESSAGE);
	}

	if(strcmp(tokens[1], "switch")==0)
	{
		msg->type = SWITCH;
		if(strcmp(tokens[3], "on")==0)
		{
			msg->u.value = 1;
		}
		else if(strcmp(tokens[3], "off")==0)
		{
			msg->u.value = 0;
		}
		else
		{
			return (E_INVALID_MESSAGE);
		}
	}
	else if(strcmp(tokens[1], "currState")==0)
	{
		msg->type = CURRENT_STATE;
		if(strcmp(tokens[3], "on")==0)
		{
			msg->u.value = 1;
		}
		else if(strcmp(tokens[3], "off")==0)
		{
			msg->u.value = 0;
		}
		else
		{
			return (E_INVALID_MESSAGE);
		}
	}
	else if(strcmp(tokens[1], "currValue")==0)
	{
		msg->type = CURRENT_VALUE;
		msg->u.value = atoi(tokens[3]);
	}
	else if(strcmp(tokens[1], "setInterval")==0)
	{
		msg->type = SET_INTERVAL;
		msg->u.value = atoi(tokens[3]);
	}
	else if(strcmp(tokens[1], "register")==0)
	{
		msg->type = REGISTER;
		str_tokenize(tokens[3], "-", register_tokens, &count);
		if(count !=4 )
		{
			return (E_INVALID_MESSAGE);
		}
		msg->u.s.type = get_device_type(register_tokens[0]);
		str_copy(&msg->u.s.ip_address, register_tokens[1]);
		str_copy(&msg->u.s.port_no, register_tokens[2]);
		str_copy(&msg->u.s.area_id, register_tokens[3]);
	}
	else
	{
		return (E_INVALID_MESSAGE);
	}
	return (E_SUCCESS);
}

int write_message(int socket_fd, message *msg)
{
	char buffer[100] = {'\0'};
	char value_str[10] = {'\0'};
	int send_count = 0;
	int msg_length = 0;

	strcat(buffer, "type:");
	switch(msg->type)
	{
	case SWITCH:
		strcat(buffer, "switch");
		break;
	case CURRENT_STATE:
		strcat(buffer, "currState");
		break;
	case CURRENT_VALUE:
		strcat(buffer, "currValue");
		break;
	case SET_INTERVAL:
		strcat(buffer, "setInterval");
		break;
	case REGISTER:
		strcat(buffer, "register");
		break;
	}

	strcat(buffer, ";");
	strcat(buffer, "action:");

	switch(msg->type)
	{
	case SWITCH:
	case CURRENT_STATE:
		if(msg->u.value == 0)
		{
			strcat(buffer, "off");
		}
		else
		{
			strcat(buffer, "on");
		}
		break;
	case CURRENT_VALUE:
	case SET_INTERVAL:
		sprintf(value_str, "%d", msg->u.value);
		strcat(buffer, value_str);
		break;
	case REGISTER:
		switch(msg->u.s.type)
		{
		case DOOR_SENSOR:
			strcat(buffer, "door_sensor");
			break;
		case MOTION_SENSOR:
			strcat(buffer, "motion_sensor");
			break;
		case KEY_CHAIN_SENSOR:
			strcat(buffer, "key_chain_sensor");
			break;
		case SECURITY_DEVICE:
			strcat(buffer, "security_device");
			break;
		case GATEWAY:
			strcat(buffer, "gateway");
			break;
		default:
			break;
		}
		strcat(buffer, "-");
		strcat(buffer, msg->u.s.ip_address);
		strcat(buffer, "-");
		strcat(buffer, msg->u.s.port_no);
		strcat(buffer, "-");
		strcat(buffer, msg->u.s.area_id);
		break;
	}

	msg_length = strlen(buffer);
	send_count = send(socket_fd, &msg_length, sizeof(int), 0);
	if(send_count < 0)
	{
		return (E_SOCKET_CONNCTION_ERORR);
	}

	LOG(("Message sent: %s\n", buffer));
	send_count = send(socket_fd, buffer, strlen(buffer), 0);
	if(send_count < 0)
	{
		return (E_SOCKET_SEND_ERROR);
	}
	return (E_SUCCESS);
}

/*
 * network_functions.h
 *
 *  Created on: Sep 27, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#include "message.h"

int create_socket(int *socket_fd, char *ip_address, char* port_no);
int create_server_socket(int *socket_fd, char *ip_address, char* port_no);
void close_socket(int socket_fd);
void send_socket(int socket_fd, char* data, int length);
int write_message(int socket_fd, message *msg);
int read_message(int socket_fd, message *msg);



/*
 * device.c
 *
 *  Created on: Sep 22, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "common.h"
#include "device.h"
#include "logger.h"
#include "string_helper_functions.h"
#include "error_codes.h"

#define TOKEN_MAX 10

int main(int argc, char*argv[])
{
	char *conf_file_name = NULL;
	FILE *conf_file_pointer = NULL;
	char line[LINE_MAX] = {'\0'};
	char *tokens[TOKEN_MAX] = {NULL};
	int count = 0;
	device_create_params device_device = {NULL, NULL, NULL};
	device_handle device = NULL;
	int return_value = 0;

	LOG_DEBUG(("DEBUG: Number of arguments are: %d\n", argc));

	if(argc<3)
	{
		LOG_ERROR(("ERROR: Please provide configuration file name\n"));
		return (0);
	}

	conf_file_name = argv[1];

	LOG_DEBUG(("DEBUG: Configuration File Name is %s\n", conf_file_name));

	conf_file_pointer = fopen(conf_file_name, "r");
	if(!conf_file_pointer)
	{
		LOG_ERROR(("ERROR: Error in opening configuration file\n"));
		return (0);
	}

	return_value = log_open_output_file(argv[2]);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Unable to open the file for writing\n"));
		return (0);
	}

	/* Read line */
	if(fgets(line, LINE_MAX, conf_file_pointer) == NULL)
	{
		LOG_DEBUG(("DEBUG: Cleanup and return"));
		fclose(conf_file_pointer);
		LOG_ERROR(("ERROR: Wrong configuration file\n"));
		return (0);
	}
	str_tokenize(line, ":\n\r", tokens, &count);
	if(count<2)
	{
		LOG_ERROR(("ERROR: Wrong configuration file\n"));
		fclose(conf_file_pointer);
		return (0);
	}

	str_copy(&device_device.gateway_ip_address, tokens[0]);
	str_copy(&device_device.gateway_port_no, tokens[1]);
	LOG_DEBUG(("DEBUG: IP Address: %s\n", device_device.gateway_ip_address));
	LOG_DEBUG(("DEBUG: Port No: %s\n", device_device.gateway_port_no));

	/* Read line */
	if (fgets(line, LINE_MAX, conf_file_pointer) == NULL)
	{
		LOG_DEBUG(("DEBUG: Cleanup and return"));
		fclose(conf_file_pointer);
		LOG_ERROR(("ERROR: Wrong configuration file\n"));
		return (0);
	}
	str_tokenize(line, ":\r\n", tokens, &count);
	if (count < 4)
	{
		LOG_ERROR(("ERROR: Wrong configuration file\n"));
		fclose(conf_file_pointer);
		return (0);
	}
	if(strcmp("device", tokens[0])!=0)
	{
		LOG_ERROR(("ERROR: Wrong configuration file\n"));
		fclose(conf_file_pointer);
		return (0);
	}
	str_copy(&device_device.device_ip_address, tokens[1]);
	str_copy(&device_device.device_port_no, tokens[2]);
	str_copy(&device_device.device_area_id, tokens[3]);

	LOG_DEBUG(("DEBUG: ip_address: %s\n", device_device.device_ip_address));
	LOG_DEBUG(("DEBUG: port_no: %s\n", device_device.device_port_no));
	LOG_DEBUG(("DEBUG: area_id: %s\n", device_device.device_area_id));

	return_value = create_device(&device, &device_device);
	if(E_SUCCESS != return_value)
	{
		LOG_ERROR(("ERROR: Unable to create smart device\n"));
		free(device_device.gateway_ip_address);
		free(device_device.gateway_port_no);
		free(device_device.device_area_id);
		free(device_device.device_ip_address);
		free(device_device.device_port_no);
		fclose(conf_file_pointer);
		return (0);
	}

	LOG_SCREEN(("INFO: Security Device started successfully\n"));

	char choice;

	printf("Press enter to exit\n");
	scanf("%c", &choice);
	delete_device(device);

	free(device_device.gateway_ip_address);
	free(device_device.gateway_port_no);
	free(device_device.device_area_id);
	free(device_device.device_ip_address);
	free(device_device.device_port_no);
	fclose(conf_file_pointer);
	logger_close();
	return (0);
}

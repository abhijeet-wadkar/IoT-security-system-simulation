/*
 * sensor.c
 *
 *  Created on: Sep 22, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "common.h"
#include "door_sensor.h"
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
	sensor_create_params sensor_device = {NULL, NULL, NULL};
	sensor_handle sensor = NULL;

	LOG_DEBUG(("DEBUG: Number of arguments are: %d\n", argc));

	if(argc<3)
	{
		LOG_ERROR(("ERROR: Please provide configuration file name(s)\n"));
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

	/* Read line */
	if(fgets(line, LINE_MAX, conf_file_pointer) == NULL)
	{
		LOG_DEBUG(("DEBUG: Cleanup and return\n"));
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

	str_copy(&sensor_device.gateway_ip_address, tokens[0]);
	str_copy(&sensor_device.gateway_port_no, tokens[1]);
	LOG_DEBUG(("DEBUG: Gateway IP Address: %s\n", sensor_device.gateway_ip_address));
	LOG_DEBUG(("DEBUG: Gateway Port No: %s\n", sensor_device.gateway_port_no));

	/* Read line */
	if (fgets(line, LINE_MAX, conf_file_pointer) == NULL)
	{
		LOG_DEBUG(("DEBUG: Cleanup and return\n"));
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
	if(strcmp("sensor", tokens[0])!=0)
	{
		LOG_ERROR(("ERROR: Wrong configuration file\n"));
		fclose(conf_file_pointer);
		return (0);
	}
	str_copy(&sensor_device.sensor_ip_address, tokens[1]);
	str_copy(&sensor_device.sensor_port_no, tokens[2]);
	str_copy(&sensor_device.sensor_area_id, tokens[3]);

	sensor_device.sensor_value_file_name = argv[2];

	LOG_DEBUG(("DEBUG: sensor ip_address: %s\n", sensor_device.sensor_ip_address));
	LOG_DEBUG(("DEBUG: sensor port_no: %s\n", sensor_device.sensor_port_no));
	LOG_DEBUG(("DEBUG: sensor area_id: %s\n", sensor_device.sensor_area_id));

	int return_value = create_sensor(&sensor, &sensor_device);
	if(return_value != E_SUCCESS)
	{
		LOG_ERROR(("ERROR: Unable to create sensor\n"));
		free(sensor_device.gateway_ip_address);
		free(sensor_device.gateway_port_no);
		free(sensor_device.sensor_area_id);
		free(sensor_device.sensor_ip_address);
		free(sensor_device.sensor_port_no);
		fclose(conf_file_pointer);
		return (0);
	}

	LOG_ERROR(("Sensor started successfully\n"));

	char choice;

	printf("Press enter to exit\n");
	scanf("%c", &choice);
	delete_sensor(sensor);

	free(sensor_device.gateway_ip_address);
	free(sensor_device.gateway_port_no);
	free(sensor_device.sensor_area_id);
	free(sensor_device.sensor_ip_address);
	free(sensor_device.sensor_port_no);
	fclose(conf_file_pointer);
	logger_close();
	return (0);
}

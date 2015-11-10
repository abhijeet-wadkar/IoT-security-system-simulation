/*
 * message.h
 *
 *  Created on: Sep 30, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#define MAX_TYPE_LENGTH 10
#define MAX_ACTION_LENGTH 50

typedef enum message_type
{
	SWITCH,
	CURRENT_STATE,
	CURRENT_VALUE,
	SET_INTERVAL,
	REGISTER
}message_type;

typedef enum device_type
{
	DOOR_SENSOR,
	MOTION_SENSOR,
	KEY_CHAIN_SENSOR,
	SECURITY_DEVICE,
	GATEWAY,
	BACK_TIER_GATEWAY,
	UNKNOWN
}device_type;

typedef struct message
{
	message_type type;
	union
	{
		int value;
		struct
		{
			device_type type;
			char *ip_address;
			char *port_no;
			char *area_id;
		}s;
	}u;
}message;

#endif /* MESSAGE_H_ */

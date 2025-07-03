#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "system_types.h"
#include "communication_types.h"

void system_state_to_str(char* msg, SystemState state)
{
	switch (state)
	{
		case SYS_STATE_NONE:
			sprintf(msg, "none");
			break;
		case SYS_STATE_INIT:
			sprintf(msg, "init");
			break;
		case SYS_STATE_STANDBY:
			sprintf(msg, "standby");
			break;
		case SYS_STATE_LIFTOFF:
			sprintf(msg, "liftoff");
			break;
		case SYS_STATE_ASCENT:
			sprintf(msg, "ascent");
			break;
		case SYS_STATE_APOGY:
			sprintf(msg, "apogy");
			break;
		case SYS_STATE_DESCENT:
			sprintf(msg, "descent");
			break;
		case SYS_STATE_GROUND:
			sprintf(msg, "ground");
			break;
		default:
			sprintf(msg, "err (val: %i)", state);
			break;
	}
}

void system_area_to_str(char* msg, SystemArea area)
{
	switch (area)
	{
		case SYS_AREA_NONE:
			sprintf(msg, "none");
			break;
		case SYS_AREA_INIT:
			sprintf(msg, "init");
			break;
		case SYS_AREA_MAIN_ALGO:
			sprintf(msg, "main_algo");
			break;
		case SYS_AREA_READ_SENSORS:
			sprintf(msg, "read_sensors");
			break;
		case SYS_AREA_PERIPH_SDCARD:
			sprintf(msg, "periph_sdcard");
			break;
		case SYS_AREA_PERIPH_RADIO:
			sprintf(msg, "periph_radio");
			break;
		case SYS_AREA_PERIPH_BAROM:
			sprintf(msg, "periph_barom");
			break;
		case SYS_AREA_PERIPH_ACC:
			sprintf(msg, "periph_acc");
			break;
		default:
			sprintf(msg, "err (val: %i)", area);
			break;
	}
}


int main()
{
	const uint16_t PAYLOAD_SIZE = 39;
	uint8_t payload[PAYLOAD_SIZE];
	Telemetry decoded;

	while (true)
	{
		memset(payload, 0, PAYLOAD_SIZE);

		if(fread(payload, 1, PAYLOAD_SIZE, stdin) == PAYLOAD_SIZE)
		{
			set_default_telemetry(&decoded);
			uint32_t time_ms;
			memcpy(&time_ms,                payload,        4);
			memcpy(&decoded.sys_state,      payload + 3,    1);
			memcpy(&decoded.sys_area,       payload + 4,    1);
			memcpy(&decoded.temp,           payload + 5,    4);
			memcpy(&decoded.pressure,       payload + 9,    4);
			memcpy(&decoded.acc_x,          payload + 13,   8);
			memcpy(&decoded.acc_y,          payload + 21,   8);
			memcpy(&decoded.acc_z,          payload + 29,   8);
			memcpy(&decoded.sys_status,     payload + 37,   1);

			char str_state[32];
			system_state_to_str(str_state, decoded.sys_state);

			char str_area[32];
			system_area_to_str(str_area, decoded.sys_area);

			printf("ms: %lu / state: %s / area: %s / temp: %f / pressure: %f / acc: (%f, %f, %f) \n", time_ms, str_state, str_area, decoded.temp, decoded.pressure, decoded.acc_x, decoded.acc_y, decoded.acc_z);
		}
		else
		{
			printf("package dropped!");
		}
		
	}
}

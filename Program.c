#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "system_types.h"
#include "communication_types.h"
#include "errno.h"

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

#define PERIPH_STR_ON "\x1b[48;2;62;161;105mON\x1b[0m"
#define PERIPH_STR_OFF "\x1b[48;2;255;66;120mOFF\x1b[0m"

#define PERIPH_BIT_TO_STR(BIT_SEQUENCE, BIT_MASK) ( ((BIT_SEQUENCE) & (BIT_MASK)) != 0? PERIPH_STR_ON : PERIPH_STR_OFF )
#define RADIO_PACKAGE_SIZE TELEMETRY_BYTES_SIZE+1

typedef enum
{
	STATE_UKNOWN,
	STATE_ID_FIRST,
	STATE_ID_SECOND
} State;

bool is_prev_skip = false;
uint32_t bytes_skipped = 0;

int main(int argc, char *argv[])
{
	uint8_t payload[RADIO_PACKAGE_SIZE];
	
	uint8_t byte_num_to_read = RADIO_PACKAGE_SIZE;
	bool is_radio = true;
	if (argc > 1 && !strcmp(argv[1], "--no-radio"))
	{
		byte_num_to_read--;
		is_radio = false;
	}

	Telemetry decoded;
	State curr_state = STATE_UKNOWN;

	while (true)
	{
		uint8_t curr_char = 0;
		fflush(stdout);
		if (fread(&curr_char, 1, 1, stdin) != 1)
		{
			bytes_skipped++;
			if (!is_prev_skip)
			{
				printf("__skip:\n");
				is_prev_skip = true;
			}

			printf("\r%i bytes", bytes_skipped);
		}

		if (curr_state == STATE_UKNOWN)
		{
			if(curr_char == 'R')
			{
				curr_state = STATE_ID_FIRST;
			}
			else
			{
				curr_state = STATE_UKNOWN;
				bytes_skipped++;
				if (!is_prev_skip)
				{
					printf("__skip:\n");
					is_prev_skip = true;
				}

				printf("\r%i bytes", bytes_skipped);
			}
		}
		else if(curr_state == STATE_ID_FIRST)
		{
			if(curr_char == '6')
			{
				curr_state = STATE_ID_SECOND;
			}
			else
			{
				curr_state = STATE_UKNOWN;
				bytes_skipped++;
				if (!is_prev_skip)
				{
					printf("__skip:\n");
					is_prev_skip = true;
				}

				printf("\r%i bytes", bytes_skipped);
			}
		}
		else if(curr_state == STATE_ID_SECOND)
		{
			memset(payload, 0, RADIO_PACKAGE_SIZE);
			//from previous iterations
			payload[0] = 'R';
			payload[1] = '6';
			payload[2] = curr_char;
		
			if(fread(payload+3, 1, byte_num_to_read-3, stdin) == byte_num_to_read-3)
			{
				uint32_t time_ms;
				uint16_t received_id = 0;
				set_default_telemetry(&decoded);

				bytes_to_telemetry(&decoded, &time_ms, &received_id, payload);

				char str_state[32];
				system_state_to_str(str_state, decoded.sys_state);

				char str_area[32];
				system_area_to_str(str_area, decoded.sys_area);

				char str_id[3];
				memcpy(str_id, &received_id, 2);
				str_id[2] = '\0';

				is_prev_skip = false;
				if (bytes_skipped != 0)
				{
					printf("\n\n");
				}

				bytes_skipped = 0;

				printf("\x1b[48;2;89;123;202m");
				if(is_radio)
				{
					uint8_t rssi = payload[RADIO_PACKAGE_SIZE-1];
					printf("/ RSSI: %i ", rssi);
				}
				printf("/ id: %s / ms: %lu \x1b[48;2;184;55;177m/ state: %s /\x1b[0m\x1b[48;2;89;123;202m area: %s / temp: %f / pressure: %f / altitude: %f / acc: (%.4f, %.4f, %.4f) / gyro: (%.4f, %.4f, %.4f) / gps: (alt %.4f, lat %.4f, lon %.4f, fix: %.4f, sat: %.4f)\x1b[0m\n",
					 str_id, time_ms, str_state, str_area, decoded.temp, decoded.pressure, decoded.altitude, decoded.acc_x, decoded.acc_y, decoded.acc_z, decoded.acc_angular_x, decoded.acc_angular_y, decoded.acc_angular_z, decoded.gps.altitude, decoded.gps.latitude, decoded.gps.longitude, decoded.gps.fix_status, decoded.gps.satellites);
				printf("peripherals | radio: %s / sd: %s / barom: %s / acc: %s / gps: %s / jumper: %s / servo: %s\n\n",
					PERIPH_BIT_TO_STR(decoded.sys_status, PERIPH_RADIO),
					PERIPH_BIT_TO_STR(decoded.sys_status, PERIPH_SD),
					PERIPH_BIT_TO_STR(decoded.sys_status, PERIPH_BAROM),
					PERIPH_BIT_TO_STR(decoded.sys_status, PERIPH_ACC),
					PERIPH_BIT_TO_STR(decoded.sys_status, PERIPH_GPS),
					PERIPH_BIT_TO_STR(decoded.sys_status, PERIPH_JUMPER),
					PERIPH_BIT_TO_STR(decoded.sys_status, PERIPH_SERVO)
				);
			}
			else
			{
				printf("incomplete package dropped!\n\n");
			}

			curr_state = STATE_UKNOWN;
		}
	}
}

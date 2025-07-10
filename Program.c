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

int main()
{
	uint8_t payload[RADIO_PACKAGE_SIZE];
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
			if(curr_char == TELEMETRY_ID_UPPER)
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
			if(curr_char == TELEMETRY_ID_LOWER)
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
			payload[0] = TELEMETRY_ID_UPPER;
			payload[1] = TELEMETRY_ID_LOWER;
			payload[2] = curr_char;
		
			if(fread(payload+3, 1, RADIO_PACKAGE_SIZE-3, stdin) == RADIO_PACKAGE_SIZE-3)
			{
				uint32_t time_ms;
				uint16_t received_id = 0;
				set_default_telemetry(&decoded);

				bytes_to_telemetry(&decoded, &time_ms, &received_id, payload);

				char str_state[32];
				//system_state_to_str(str_state, decoded.sys_state);

				char str_area[32];
				//system_area_to_str(str_area, decoded.sys_area);

				char str_id[3];
				memcpy(str_id, &received_id, 2);
				str_id[2] = '\0';

				is_prev_skip = false;
				if (bytes_skipped != 0)
				{
					printf("\n\n");
				}

				uint8_t rssi = payload[RADIO_PACKAGE_SIZE-1];

				bytes_skipped = 0;
				printf("\x1b[48;2;89;123;202m / RSSI: %i / id:%s / num: %i / time: %i \x1b[48;2;184;55;177m/ status: %i /\x1b[0m\x1b[48;2;89;123;202m mag: (%.4f, %.4f, %.4f) / acc: (%.4f, %.4f, %.4f) / gyro: (%.4f, %.4f, %.4f) / pressure: %f / temp: %f / humidity: %f / altitude; %f / gps: (lat %.4f, lon %.4f)\x1b[0m\n\n",
					 rssi, str_id, decoded.num_pkg, time_ms, decoded.status, decoded.mag_x, decoded.mag_y, decoded.mag_z, decoded.acc_x, decoded.acc_y, decoded.acc_z, decoded.gyro_x, decoded.gyro_y, decoded.gyro_z, decoded.pressure, decoded.temp, decoded.humidity, decoded.altitude, decoded.gps_latitude, decoded.gps_longitude);
			}
			else
			{
				printf("incomplete package dropped!\n\n");
			}

			curr_state = STATE_UKNOWN;
		}
	}
}

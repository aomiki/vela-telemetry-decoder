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

#define PERIPH_STR_ON "\x1b[48;2;62;161;105mON\x1b[0m"
#define PERIPH_STR_OFF "\x1b[48;2;255;66;120mOFF\x1b[0m"

#define PERIPH_BIT_TO_STR(BIT_SEQUENCE, BIT_MASK) ( ((BIT_SEQUENCE) & (BIT_MASK)) != 0? PERIPH_STR_ON : PERIPH_STR_OFF )

int main()
{
	uint8_t payload[TELEMETRY_BYTES_SIZE];
	Telemetry decoded;

	while (true)
	{
		memset(payload, 0, TELEMETRY_BYTES_SIZE);

		if(fread(payload, 1, TELEMETRY_BYTES_SIZE, stdin) == TELEMETRY_BYTES_SIZE)
		{
			uint32_t time_ms;
			uint16_t received_id = 0;
			set_default_telemetry(&decoded);
			bytes_to_telemetry(&decoded, &time_ms, &received_id, payload);

			if (received_id != TELEMETRY_ID)
			{
				//Return back and skip 2 first bytes
				fseek(stdin, -TELEMETRY_BYTES_SIZE + 1, SEEK_CUR);
				printf("_skip_\n");
				continue;
			}

			char str_state[32];
			system_state_to_str(str_state, decoded.sys_state);

			char str_area[32];
			system_area_to_str(str_area, decoded.sys_area);

			char str_id[3];
			memcpy(str_id, &received_id, 2);
			str_id[2] = '\0';

			printf("\x1b[48;2;89;123;202m / id: %s / ms: %lu \x1b[48;2;184;55;177m/ state: %s /\x1b[0m\x1b[48;2;89;123;202m area: %s / temp: %f / pressure: %f / altitude: %f / acc: (%.4f, %.4f, %.4f) / gps: (alt %.4f, lat %.4f, lon %.4f)\x1b[0m\n", str_id, time_ms, str_state, str_area, decoded.temp, decoded.pressure, decoded.altitude, decoded.acc_x, decoded.acc_y, decoded.acc_z, decoded.gps.altitude, decoded.gps.latitude, decoded.gps.longitude);
			printf("periperals | radio: %s / sd: %s / barom: %s / acc: %s / gps: %s / jumper: %s / servo: %s\n\n",
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
			printf("package dropped!\n");
		}
		
	}
}

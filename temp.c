#include <avr/io.h>
#include "common.h"
#include "temp.h"
#include "lcd.h"
#include "timer.h"
#include "beep.h"
#include "ds18x20.h"
#include "onewire.h"
#include <stdio.h>

static uint8_t	g_ds18x20_sensors[MAX_DS18X20_COUNT][OW_ROMCODE_SIZE];
static uint8_t	g_ds18x20_count;
static float	g_temp_value[MAX_DS18X20_COUNT];

void stop(stop_mode_e);

uint16_t	g_temp_alarm[MAX_DS18X20_COUNT];
uint16_t	g_temp_stop[MAX_DS18X20_COUNT];

uint8_t search_sensors(void)
{
	uint8_t i;
	uint8_t id[OW_ROMCODE_SIZE];
	uint8_t diff, nSensors;
	
	nSensors = 0;
	
	for( diff = OW_SEARCH_FIRST; 
		diff != OW_LAST_DEVICE && nSensors < MAX_DS18X20_COUNT ; )
	{
		DS18X20_find_sensor( &diff, &id[0] );
		
		if( diff == OW_PRESENCE_ERR ) {
			break;
		}
		
		if( diff == OW_DATA_ERR ) {
			break;
		}
		
		for (i=0;i<OW_ROMCODE_SIZE;i++)
			g_ds18x20_sensors[nSensors][i]=id[i];
		
		nSensors++;
	}
	
	return nSensors;
}

void temp_init(void)
{
	int	id;
	
	g_ds18x20_count = search_sensors();
	
	for(id = 0; id < MAX_DS18X20_COUNT; id++)
		g_temp_value[id] = 0.;
}

float temp_value(int sensor_id)
{
	if (0 > sensor_id || sensor_id >= MAX_DS18X20_COUNT)
		sensor_id = 0;
	
	return g_temp_value[sensor_id];
} 

void do_temp(void)
{
	static uint8_t	timer_id = 0;
	static uint8_t	flag = 0;
	static uint8_t	sensor_id = 0;
	
	uint8_t	sign;
	uint8_t cel;
	uint8_t frac;
	
//	if (sensor_id >= g_ds18x20_count)
//		g_temp_value[sensor_id] = 0.;
		
	if ((0 == timer_id) || (0 == timer_value(timer_id)))
	{
		stop_timer(timer_id);
		timer_id = start_timer(DS18B20_TCONV_12BIT);

		if (0 == flag)
		{
			DS18X20_start_meas(DS18X20_POWER_EXTERN, g_ds18x20_sensors[sensor_id]);
			flag = 1;
			return;
		}
		
		flag = 0;
		
		if (DS18X20_OK != DS18X20_read_meas(g_ds18x20_sensors[sensor_id], &sign, &cel, &frac))
			g_temp_value[sensor_id] = 0.;
		else
			g_temp_value[sensor_id] = DS18X20_temp_to_decicel(sign, cel, frac) / 10.;
		
		
		if (IS_UZG_RUN)
		{				
			if (g_temp_stop[sensor_id] < temp_value(sensor_id))
			{
				stop(STOP_TEMPERATURE1 + sensor_id);
				beep_ms(1000);
			}	
			else if (g_temp_alarm[sensor_id] < temp_value(sensor_id))
			{
				sprintf(lcd_line0, "CRITICAL TEMP.%d      ", sensor_id + 1);
				do_lcd();
				beep_ms(100);
				_delay_ms(100);
				beep_ms(200);
				
			}
		}

		if (++sensor_id >= MAX_DS18X20_COUNT)
			sensor_id = 0;
	}
}

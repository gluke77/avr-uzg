#include <avr\io.h>
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
static float	g_temp_value;

void stop(stop_mode_e);

uint16_t	g_temp_alarm;
uint16_t	g_temp_stop;

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
	g_ds18x20_count = search_sensors();
	g_temp_value = 0.;
}

float temp_value(void)
{
	return g_temp_value;
} 

void do_temp(void)
{
	static uint8_t	timer_id = 0;
	static uint8_t	flag = 0;
	
	uint8_t	sign;
	uint8_t cel;
	uint8_t frac;
	
	if (0 == g_ds18x20_count)
		g_temp_value = 0.;
	else if ((0 == timer_id) || (0 == timer_value(timer_id)))
	{
		stop_timer(timer_id);
		timer_id = start_timer(DS18B20_TCONV_12BIT);

		if (0 == flag)
		{
			DS18X20_start_meas(DS18X20_POWER_EXTERN, g_ds18x20_sensors[0]);
			flag = 1;
			return;
		}
		
		flag = 0;
		
		if (DS18X20_OK != DS18X20_read_meas(g_ds18x20_sensors[0], &sign, &cel, &frac))
			g_temp_value = 0.;
		else
			g_temp_value = DS18X20_temp_to_decicel(sign, cel, frac) / 10.;
		
		if (!IS_UZG_RUN)
			return;
		
		if (g_temp_stop < temp_value())
		{
			stop(STOP_TEMPERATURE);
			beep_ms(1000);
		}	
		else if (g_temp_alarm < temp_value())
		{
			sprintf(lcd_line0, "ÒÅÌÏ. KÐÈÒÈ×ÅÑÊÀß   ");
			do_lcd();
			beep_ms(100);
			_delay_ms(100);
			beep_ms(200);
			
		}
	}
}
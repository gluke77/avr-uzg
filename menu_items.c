#include <avr\io.h>
#include <avr\interrupt.h>
#include <avr\eeprom.h>
#include "string.h"
#include "stdio.h"
#include "common.h"
#include "menu.h"
#include "menu_items.h"
#include "dds.h"
#include "timer.h"
#include "kbd.h"
#include "lcd.h"
#include "adc.h"
#include "current.h"
#include "power.h"
#include "temp.h"
#include "usart.h"
#include "beep.h"

uint32_t	g_freq_upper;
uint32_t	g_freq_lower;

uint16_t	g_int_timeout;

uint8_t		g_menu_search_auto_idx;
uint8_t		g_autosearch_running = 0;

extern uint8_t	g_bias_alarm;
extern uint8_t	g_pwm_alarm;

void keep_stop(void);
void keep_start(void);
void set_pfc_mode(pfc_mode_e);
void start(void);
void stop(stop_mode_e);
void loadFromEE(void);
void storeToEE(void);
void reset_settings(void);
void fault_interrupts_init(fault_interrupts_mode_e);

void menu_items_init(void)
{
	uint8_t		idx;
	
	idx = 0; 
//	menu_items[MENU_MODE_WORK][idx++] = menu_freq;
	menu_items[MENU_MODE_WORK][idx++] = menu_search;
	menu_items[MENU_MODE_WORK][idx++] = menu_power;
//	menu_items[MENU_MODE_WORK][idx++] = menu_mult;	
	menu_items[MENU_MODE_WORK][idx++] = menu_current;	
	menu_items[MENU_MODE_WORK][idx++] = menu_amp;
	menu_items[MENU_MODE_WORK][idx++] = menu_temp;
//	menu_items[MENU_MODE_WORK][idx++] = menu_monitor;
	menu_items[MENU_MODE_WORK][idx++] = menu_stop_mode;
	menu_items[MENU_MODE_WORK][idx++] = menu_adc0;
	menu_items[MENU_MODE_WORK][idx++] = menu_adc1;
	menu_items[MENU_MODE_WORK][idx++] = menu_adc2;
//	menu_items[MENU_MODE_WORK][idx++] = menu_adc3;
	menu_items[MENU_MODE_WORK][idx++] = menu_version;

	
	g_menu_search_auto_idx = idx;
	menu_items[MENU_MODE_WORK][idx++] = menu_search_auto;

	idx = 0;
	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_freq_upper;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_freq_lower;
	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_max_bias_pwm;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_min_bias_pwm;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_bias_pwm_base;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_bias_pwm_shift;
	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_max_power_pwm;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_min_power_pwm;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_power_pwm_base;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_power_pwm_shift;

	menu_items[MENU_MODE_SETTINGS][idx++] = menu_int_timeout;

	menu_items[MENU_MODE_SETTINGS][idx++] = menu_pfc_mode;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_autosearch_mode;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_keep_mode;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_keep_step;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_keep_delta;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_temp_alarm;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_temp_stop;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_fault_interrupts;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_modbus_id;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_baudrate;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_store_settings;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_reset_settings;

	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc0_count;
//	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc0_delay;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc1_count;
//	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc1_delay;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc2_count;
//	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc2_delay;
//	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc3_count;
//	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc3_delay;

}

void menu_common(void)
{
	if (KEY_PRESSED(KEY_UP))
	{
		menu_item_prev();
		CLEAR_KEY_PRESSED(KEY_UP);
	}
	
	if (KEY_PRESSED(KEY_DOWN))
	{
		menu_item_next();
		CLEAR_KEY_PRESSED(KEY_DOWN);
	}

	if (KEY_PRESSED(KEY_MENU))
	{
		menu_mode_next();
		CLEAR_KEY_PRESSED(KEY_MENU);
	}

	if (KEY_PRESSED(KEY_RUN))
	{
		start();
		CLEAR_KEY_PRESSED(KEY_RUN);
	}

	if (KEY_PRESSED(KEY_STOP))
	{
		if (IS_UZG_RUN)
			stop(STOP_BUTTON);
		else
		{
			g_bias_alarm = 0;
			g_pwm_alarm = 0;
			stop(STOP_NOT_CHANGE);
		}
		CLEAR_KEY_PRESSED(KEY_STOP);
	}
}


void menu_freq(void)
{
	sprintf(lcd_line1, "FREQUENCY= %-9ld  ", g_dds_freq);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_dds_freq++;
			
		if (g_dds_freq > g_freq_upper)
			g_dds_freq = g_freq_upper;
			
		dds_setfreq(g_dds_freq);
	
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		g_dds_freq--;
			
		if (g_dds_freq < g_freq_lower)
			g_dds_freq = g_freq_lower;
			
		dds_setfreq(g_dds_freq);

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_monitor(void)
{
	char	buf[50];
	uint16_t	i;

	sprintf(lcd_line1, "MONITOR                  ");
	
	if (KEY_PRESSED(KEY_ENTER))
	{
		CLEAR_KEY_PRESSED(KEY_ENTER);
	
		for (i = 0; i < 256; i++)
		{
			set_bias_pwm(i);
			delay_ms(g_int_timeout + g_int_timeout + adc_get_timeout(ADC_BIASCURRENT));

			sprintf(lcd_line1, "BIAS:%d ADC0%d", g_bias_pwm, adc_mean_value(ADC_BIASCURRENT));
			do_lcd();
	
			sprintf(buf, "BIAS_PWM\t%d\tADC0\t%d\n", g_bias_pwm, adc_mean_value(ADC_BIASCURRENT));
			usart1_cmd(buf, 0, 0, 0);
		}
	}
	
	menu_common();
}

void menu_amp(void)
{
	int16_t		amp;
	
	cli();
	amp = adc_value(ADC_AMP);
	sei();
	
	sprintf(lcd_line1, "AMPLITUDE:%-10d", amp);
	
	menu_common();
}

void menu_temp(void)
{
	sprintf(lcd_line1, "TEMPERATURE:%.1fC               ", temp_value());
	
	menu_common();
}

void menu_freq_step(void)
{
	uint32_t	freq_step = 100;

	sprintf(lcd_line1, "◊¿—“Œ“¿+=%-11ld  ", g_dds_freq);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_dds_freq += freq_step;
			
		if (g_dds_freq > g_freq_upper)
			g_dds_freq = g_freq_upper;
		
		dds_setfreq(g_dds_freq);
	
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		g_dds_freq -= freq_step;
			
		if (g_dds_freq < g_freq_lower)
			g_dds_freq = g_freq_lower;
			
		dds_setfreq(g_dds_freq);

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	menu_common();
}

void menu_current(void)
{
	sprintf(lcd_line1, "CURRENT= %.2fA            ", bias_pwm_to_current(g_bias_pwm));

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (0 == g_bias_pwm)
			g_bias_pwm = g_min_bias_pwm;
		else if (g_max_bias_pwm - g_bias_pwm < g_bias_pwm_step)
			g_bias_pwm = g_max_bias_pwm;
		else
			g_bias_pwm += g_bias_pwm_step;
		
		set_bias_pwm(g_bias_pwm);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_bias_pwm == g_min_bias_pwm)
			g_bias_pwm = 0;
		else if (g_bias_pwm < g_min_bias_pwm + g_bias_pwm_step)
			g_bias_pwm = g_min_bias_pwm;
		else
			g_bias_pwm -= g_bias_pwm_step;
			
		set_bias_pwm(g_bias_pwm);

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_power(void)
{
	sprintf(lcd_line1, "POWER= %2d%%               ", g_power_pwm + 1);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_max_power_pwm - g_power_pwm < g_power_pwm_step)
			g_power_pwm = g_max_power_pwm;
		else
			g_power_pwm += g_power_pwm_step;
		
		set_power_pwm(g_power_pwm);

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_power_pwm < g_min_power_pwm + g_power_pwm_step)
			g_power_pwm = g_min_power_pwm;
		else
			g_power_pwm -= g_power_pwm_step;
			
		set_power_pwm(g_power_pwm);

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_bias_pwm_base(void)
{
	sprintf(lcd_line1, "START CURRENT= %.2fA     ", bias_pwm_to_current(g_bias_pwm_base));

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_max_bias_pwm - g_bias_pwm_shift - g_bias_pwm_base < g_bias_pwm_step)
			g_bias_pwm_base = g_max_bias_pwm - g_bias_pwm_shift;
		else
			g_bias_pwm_base += g_bias_pwm_step;
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_bias_pwm_base < g_min_bias_pwm + g_bias_pwm_step)
			g_bias_pwm_base = g_min_bias_pwm;
		else
			g_bias_pwm_base -= g_bias_pwm_step;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}


void menu_bias_pwm_shift(void)
{
	sprintf(lcd_line1, "ADD. CURRENT= %.2fA       ", bias_pwm_to_current(g_bias_pwm_shift));

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_max_bias_pwm - g_bias_pwm_shift - g_bias_pwm_base < g_bias_pwm_step)
			g_bias_pwm_shift = g_max_bias_pwm - g_bias_pwm_base;
		else
			g_bias_pwm_shift += g_bias_pwm_step;
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_bias_pwm_shift < g_bias_pwm_step)
			g_bias_pwm_shift = 0;
		else
			g_bias_pwm_shift -= g_bias_pwm_step;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_power_pwm_base(void)
{
	sprintf(lcd_line1, "START POWER= %2d%%     ", g_power_pwm_base + 1);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_max_power_pwm - g_power_pwm_shift - g_power_pwm_base < g_power_pwm_step)
			g_power_pwm_base = g_max_power_pwm - g_power_pwm_shift;
		else
			g_power_pwm_base += g_power_pwm_step;
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_power_pwm_base < g_min_power_pwm + g_power_pwm_step)
			g_power_pwm_base = g_min_power_pwm;
		else
			g_power_pwm_base -= g_power_pwm_step;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}


void menu_power_pwm_shift(void)
{
	sprintf(lcd_line1, "ADD. POWER= %2d%%       ", g_power_pwm_shift);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_max_power_pwm - g_power_pwm_shift - g_power_pwm_base < g_power_pwm_step)
			g_power_pwm_shift = g_max_power_pwm - g_power_pwm_base;
		else
			g_power_pwm_shift += g_power_pwm_step;
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_power_pwm_shift < g_power_pwm_step)
			g_power_pwm_shift = 0;
		else
			g_power_pwm_shift -= g_power_pwm_step;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_max_power_pwm(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if ((g_power_pwm_base + g_power_pwm_shift < g_max_power_pwm) &&
			(g_min_power_pwm < g_max_power_pwm))
			g_max_power_pwm--;

		if ((IS_UZG_RUN) && (g_power_pwm > g_max_power_pwm))
			set_power_pwm(g_max_power_pwm);

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (99 > g_max_power_pwm)
			g_max_power_pwm++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "MAX POWER= %d%%       ", g_max_power_pwm + 1);
	
	menu_common();
}


void menu_min_power_pwm(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (69 < g_min_power_pwm)
			g_min_power_pwm--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if ((g_power_pwm_base > g_min_power_pwm) &&
			(g_max_power_pwm > g_min_power_pwm))
			g_min_power_pwm++;

		if ((IS_UZG_RUN) && (g_power_pwm < g_min_power_pwm))
			set_power_pwm(g_min_power_pwm);

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "MIN POWER= %d%%         ", g_min_power_pwm + 1);
	
	menu_common();
}



void menu_mult(void)
{
	sprintf(lcd_line1, "ÃÕŒ∆»“≈À‹:%-10ld    ", g_dds_mult);
/*	
	if (KEY_PRESSED(KEY_LEFT))
	{
		g_dds_mult--;
		
		if (0 == g_dds_mult)
			g_dds_mult = 1;
				
		dds_setmultiplier(g_dds_mult);
					
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_dds_mult++;
			
		if (0x0FFFFFFF + 1 == g_dds_mult)
			g_dds_mult = 0x0FFFFFFF;
				
		dds_setmultiplier(g_dds_mult);
				
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
*/	
	menu_common();
}

void menu_adc0(void)
{
	sprintf(lcd_line1, "ADC0:%-15d", adc_mean_value(0));
	
	menu_common();
}

void menu_adc1(void)
{
	sprintf(lcd_line1, "ADC1:%-15d", adc_mean_value(1));
	
	menu_common();
}

void menu_adc2(void)
{
//#define TEMP_GAIN	(0.6912)
//#define TEMP_OFFSET	(-140)

//	sprintf(lcd_line1, "ADC2:%-15f", adc_mean_value(2) * TEMP_GAIN + TEMP_OFFSET);
	sprintf(lcd_line1, "ADC2:%-15d", adc_mean_value(2));
	
	menu_common();
}

void menu_adc3(void)
{
	sprintf(lcd_line1, "ADC3:%-15d", adc_mean_value(3));
	
	menu_common();
}

void menu_search_auto(void)
{
	static uint32_t	left_freq_0;
	static uint32_t	right_freq_0;
	static uint32_t	freq_step_0 = 10;

	static uint16_t	max_current;

	static uint8_t		timer_id;
	static uint16_t	timeout;

	int16_t		current;
	
	if (KEY_PRESSED(KEY_ENTER))
	{
		if (!g_autosearch_running)
		{
			keep_stop();
		
			timeout = adc_get_timeout(ADC_CURRENT) + g_int_timeout; // + adc_get_timeout(0);
		
			dds_setfreq(g_freq_lower);
			left_freq_0 = g_freq_lower;
			right_freq_0 = g_freq_lower;
			max_current = 0;
			
			g_autosearch_running = 1;
		}
		
		CLEAR_KEY_PRESSED(KEY_ENTER);
	}
	
	if (g_autosearch_running)
	{
		if (g_dds_freq < g_freq_upper)
		{
			if (0 == timer_id)
				timer_id = start_timer(timeout);
			else
			{
				if (!timer_value(timer_id))
				{		
					stop_timer(timer_id);
					timer_id = 0;
			
					cli();
					current = adc_mean_value(ADC_CURRENT);
					sei();

					sprintf(lcd_line1, "SEARCH F:%-5ld C:%-4.2f", g_dds_freq, adc_to_current(current));

					if (current > max_current)
					{
						max_current = current;
						left_freq_0 = g_dds_freq;			
					}
					else if (current == max_current)
						right_freq_0 = g_dds_freq;
			
					g_dds_freq += freq_step_0;
					dds_setfreq(g_dds_freq);
				}
			}
		}
		else
		{
			if (right_freq_0 < left_freq_0)
				right_freq_0 = left_freq_0;
				
			dds_setfreq((left_freq_0 + right_freq_0)/2);
			set_bias_pwm(g_bias_pwm_base + g_bias_pwm_shift);
			set_power_pwm(g_power_pwm_base + g_power_pwm_shift);
			menu_item_next();
			keep_start();
			
			g_autosearch_running = 0;
		}
	}
	else
	{
		sprintf(lcd_line1, "START SEARCH        ");
		menu_common();
	}
}

void menu_search(void)
{
	int16_t		current_value = 0;
	int16_t		bias_value = 0;
	int16_t		amp_value = 0;
	
	float		curr = 0.;
	float		bias = 0.;
		
	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_dds_freq++;
			
		if (g_dds_freq > g_freq_upper)
			g_dds_freq = g_freq_upper;
			
		dds_setfreq(g_dds_freq);
	
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		g_dds_freq--;
			
		if (g_dds_freq < g_freq_lower)
			g_dds_freq = g_freq_lower;
			
		dds_setfreq(g_dds_freq);

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	current_value = adc_mean_value(ADC_CURRENT);
	bias_value = adc_mean_value(ADC_BIASCURRENT);
//	amp_value = adc_mean_value(ADC_AMP);
	
	curr = adc_to_current(current_value);
	if (curr < 0.)
		curr = 0.;
	
	bias = adc_to_current(bias_value);
	if (bias < 0.)
		bias = 0.;
	
	sprintf(lcd_line1, "F=%-5ld C:%-4.2f T:%-3.2f     ",
		g_dds_freq, curr, bias);
	
	menu_common();
}


void menu_adc0_count(void)
{
	if (KEY_PRESSED(KEY_RIGHT))
	{
		adc_set_count(0, adc_get_count(0) + 1);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		adc_set_count(0, adc_get_count(0) - 1);
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	sprintf(lcd_line1, "ADC_REPEAT0= %-7d", adc_get_count(0));

	menu_common();
}

void menu_adc0_delay(void)
{
	if (KEY_PRESSED(KEY_RIGHT))
	{
		adc_set_delay(0, adc_get_delay(0) + 1);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		adc_set_delay(0, adc_get_delay(0) - 1);
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	sprintf(lcd_line1, "ADC_DELAY0= %-8d", adc_get_delay(0));

	menu_common();
}

void menu_adc1_count(void)
{
	if (KEY_PRESSED(KEY_RIGHT))
	{
		adc_set_count(1, adc_get_count(1) + 1);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		adc_set_count(1, adc_get_count(1) - 1);
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	sprintf(lcd_line1, "ADC_REPEAT1= %-7d", adc_get_count(1));

	menu_common();
}

void menu_adc1_delay(void)
{
	if (KEY_PRESSED(KEY_RIGHT))
	{
		adc_set_delay(1, adc_get_delay(1) + 1);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		adc_set_delay(1, adc_get_delay(1) - 1);
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	sprintf(lcd_line1, "ADC_DELAY1= %-8d", adc_get_delay(1));

	menu_common();
}

void menu_adc2_count(void)
{
	if (KEY_PRESSED(KEY_RIGHT))
	{
		adc_set_count(2, adc_get_count(2) + 1);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		adc_set_count(2, adc_get_count(2) - 1);
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	sprintf(lcd_line1, "ADC_REPEAT2= %-7d", adc_get_count(2));

	menu_common();
}

void menu_adc2_delay(void)
{
	if (KEY_PRESSED(KEY_RIGHT))
	{
		adc_set_delay(2, adc_get_delay(2) + 1);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		adc_set_delay(2, adc_get_delay(2) - 1);
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	sprintf(lcd_line1, "ADC_DELAY2= %-8d", adc_get_delay(2));

	menu_common();
}

void menu_adc3_count(void)
{
	if (KEY_PRESSED(KEY_RIGHT))
	{
		adc_set_count(3, adc_get_count(3) + 1);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		adc_set_count(3, adc_get_count(3) - 1);
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	sprintf(lcd_line1, "ADC_REPEAT3= %-7d", adc_get_count(3));

	menu_common();
}

void menu_adc3_delay(void)
{
	if (KEY_PRESSED(KEY_RIGHT))
	{
		adc_set_delay(3, adc_get_delay(3) + 1);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		adc_set_delay(3, adc_get_delay(3) - 1);
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
		
	sprintf(lcd_line1, "ADC_DELAY3= %-8d", adc_get_delay(3));

	menu_common();
}

void menu_freq_lower(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		g_freq_lower--;

		if (DDS_MIN_FREQ > g_freq_lower)
			g_freq_lower = DDS_MIN_FREQ;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_freq_lower++;

		if (g_freq_upper < g_freq_lower)
			g_freq_lower = g_freq_upper;
	
		if (g_dds_freq < g_freq_lower)
			dds_setfreq(g_freq_lower);
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "MIN FREQ= %ldHz    ", g_freq_lower);

	menu_common();
}

void menu_freq_upper(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		g_freq_upper--;

		if (g_freq_upper < g_freq_lower)
			g_freq_upper = g_freq_lower;
	
		if (g_dds_freq > g_freq_upper)
			dds_setfreq(g_freq_upper);
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_freq_upper++;

		if (DDS_MAX_FREQ < g_freq_upper)
			g_freq_upper = DDS_MAX_FREQ;
						
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "MAX FREQ= %ldHz    ", g_freq_upper);

	menu_common();
}

char	pfc_mode_str[PFC_COUNT][5] = {"", "OFF", "ON", "AUTO"};

void menu_pfc_mode(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		
		if (PFC_OFF == g_pfc_mode)
			g_pfc_mode = PFC_COUNT;
		g_pfc_mode--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_pfc_mode++;
	
		if (PFC_AUTO < g_pfc_mode)
			g_pfc_mode = PFC_OFF;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "PC:%-17s", pfc_mode_str[g_pfc_mode]);
	
	set_pfc_mode(g_pfc_mode);
	
	menu_common();
}

char	keep_mode_str[KEEP_COUNT][5] = {"","OFF", "CURR.", "AMP."};

void menu_keep_mode(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		
		if (KEEP_OFF == g_keep_mode)
			g_keep_mode = KEEP_COUNT;
		g_keep_mode--;

		if (KEEP_OFF == g_keep_mode)
			keep_stop();
		else if (IS_UZG_RUN)
			keep_start();

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_keep_mode++;
	
		if (KEEP_COUNT <= g_keep_mode)
			g_keep_mode = KEEP_OFF;

		if (KEEP_OFF == g_keep_mode)
			keep_stop();
		else if (IS_UZG_RUN)
			keep_start();

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "AUTOADJUST:%-9s", keep_mode_str[g_keep_mode]);
	
	menu_common();
}

void menu_autosearch_mode(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (AUTOSEARCH_OFF == g_autosearch_mode)
			g_autosearch_mode = AUTOSEARCH_COUNT;
		g_autosearch_mode--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_autosearch_mode++;
	
		if (AUTOSEARCH_COUNT <= g_autosearch_mode)
			g_autosearch_mode = AUTOSEARCH_OFF;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "SEARCH AT START:%-4s",	(AUTOSEARCH_ON == g_autosearch_mode)?"ON":"OFF");
	
	menu_common();
}

void menu_fault_interrupts(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (FAULT_INTERRUPTS_OFF == g_fault_interrupts_mode)
			g_fault_interrupts_mode = FAULT_INTERRUPTS_COUNT;
		g_fault_interrupts_mode--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_fault_interrupts_mode++;
	
		if (FAULT_INTERRUPTS_COUNT <= g_fault_interrupts_mode)
			g_fault_interrupts_mode = FAULT_INTERRUPTS_OFF;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "PROTECTION:%-9s",	(FAULT_INTERRUPTS_ON == g_fault_interrupts_mode)?"ON":"OFF");

	fault_interrupts_init(g_fault_interrupts_mode);
	
	menu_common();
}


void menu_keep_step(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (0 < g_keep_freq_step)
			g_keep_freq_step--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (10 > g_keep_freq_step)
			g_keep_freq_step++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "SEARCH STEP= %-7d", g_keep_freq_step);
	
	menu_common();
}

void menu_keep_delta(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (0 < g_keep_freq_max_delta)
			g_keep_freq_max_delta--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (10 > g_keep_freq_max_delta)
			g_keep_freq_max_delta++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "SEARCH RANGE= %-6d", g_keep_freq_max_delta);
	
	menu_common();
}

void menu_max_bias_pwm(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if ((g_bias_pwm_base + g_bias_pwm_shift < g_max_bias_pwm) && (g_min_bias_pwm < g_max_bias_pwm))
			g_max_bias_pwm--;

		if ((IS_UZG_RUN) && (g_bias_pwm > g_max_bias_pwm))
			set_bias_pwm(g_max_bias_pwm);

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (255 > g_max_bias_pwm)
			g_max_bias_pwm++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "MAX CURRENT= %-4.2fA   ",
		bias_pwm_to_current(g_max_bias_pwm));
	
	menu_common();
}


void menu_min_bias_pwm(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (10 < g_min_bias_pwm)
			g_min_bias_pwm--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if ((g_bias_pwm_base > g_min_bias_pwm) && (g_max_bias_pwm > g_min_bias_pwm))
			g_min_bias_pwm++;

		if ((IS_UZG_RUN) && (g_bias_pwm < g_min_bias_pwm))
			set_bias_pwm(g_min_bias_pwm);

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "MIN CURRENT= %-4.2fA  ",
		bias_pwm_to_current(g_min_bias_pwm));
	
	menu_common();
}


void menu_store_settings(void)
{
	if (KEY_PRESSED(KEY_ENTER))
	{
		storeToEE();
		sprintf(lcd_line1, "SETTINGS SAVED      ");
		do_lcd();
		beep_ms(500);
	
		CLEAR_KEY_PRESSED(KEY_ENTER);
	}
	
	sprintf(lcd_line1, "SAVE SETTINGS       ");
	
	menu_common();
}

void menu_reset_settings(void)
{
	if (KEY_PRESSED(KEY_ENTER))
	{
		reset_settings();
		storeToEE();
		sprintf(lcd_line1, "DEFAULT SETTINGS    ");
		do_lcd();
		beep_ms(500);
	
		CLEAR_KEY_PRESSED(KEY_ENTER);
	}
	
	sprintf(lcd_line1, "RESET SETTINGS      ");
	
	menu_common();
}

#define FREQ_UPPER_ADDR	(0)
#define FREQ_LOWER_ADDR	(2)
#define FREQ_ADDR		(4)
#define PWM_SHIFT_ADDR	(6)
#define	PWM_BASE_ADDR	(8)

#define ADC0_DELAY_ADDR	(9)
#define ADC0_COUNT_ADDR	(10)
#define ADC1_DELAY_ADDR	(12)
#define ADC1_COUNT_ADDR	(13)
#define ADC2_DELAY_ADDR	(15)
#define ADC2_COUNT_ADDR	(16)
#define ADC3_DELAY_ADDR	(18)
#define ADC3_COUNT_ADDR	(19)

#define PFC_MODE_ADDR	(21)
#define INT_TIMEOUT_ADDR	(22)
#define KEEP_MODE_ADDR	(24)
#define KEEP_STEP_ADDR	(25)
#define KEEP_DELTA_ADDR	(26)
#define MAX_PWM_ADDR	(27)
#define MIN_PWM_ADDR	(28)
#define BAUD_LO_ADDR	(29)
#define BAUD_HI_ADDR	(31)
#define MODBUS_ID_ADDR	(33)
#define TEMP_ALARM_ADDR	(34)
#define TEMP_STOP_ADDR	(36)

#define POWER_PWM_SHIFT_ADDR		(38)
#define POWER_PWM_BASE_ADDR			(39)
#define MAX_POWER_PWM_ADDR			(40)
#define MIN_POWER_PWM_ADDR			(41)
#define AUTOSEARCH_MODE_ADDR		(42)
#define FAULT_INTERRUPTS_MODE_ADDR	(43)

void loadFromEE(void)
{
	g_freq_upper = (uint32_t)eeprom_read_word(FREQ_UPPER_ADDR);
	g_freq_lower = (uint32_t)eeprom_read_word(FREQ_LOWER_ADDR);
	dds_setfreq((uint32_t)eeprom_read_word(FREQ_ADDR));
	g_bias_pwm_base = eeprom_read_byte(PWM_BASE_ADDR);
	g_bias_pwm_shift = eeprom_read_byte(PWM_SHIFT_ADDR);
	
	adc_set_delay(0, eeprom_read_byte(ADC0_DELAY_ADDR));
	adc_set_count(0, eeprom_read_word(ADC0_COUNT_ADDR));
	adc_set_delay(1, eeprom_read_byte(ADC1_DELAY_ADDR));
	adc_set_count(1, eeprom_read_word(ADC1_COUNT_ADDR));	
	adc_set_delay(2, eeprom_read_byte(ADC2_DELAY_ADDR));
	adc_set_count(2, eeprom_read_word(ADC2_COUNT_ADDR));
//	adc_set_delay(3, eeprom_read_byte(ADC3_DELAY_ADDR));
//	adc_set_count(3, eeprom_read_word(ADC3_COUNT_ADDR));
	
	set_pfc_mode(eeprom_read_byte(PFC_MODE_ADDR));
	
	g_int_timeout = eeprom_read_word(INT_TIMEOUT_ADDR);
	g_keep_mode = eeprom_read_byte(KEEP_MODE_ADDR);
	g_keep_freq_step = eeprom_read_byte(KEEP_STEP_ADDR);
	g_keep_freq_max_delta = eeprom_read_byte(KEEP_DELTA_ADDR);
	g_max_bias_pwm = eeprom_read_byte(MAX_PWM_ADDR);
	g_min_bias_pwm = eeprom_read_byte(MIN_PWM_ADDR);
	
	g_baudrate = (uint32_t)eeprom_read_word(BAUD_LO_ADDR);
	g_baudrate |= ((uint32_t)(eeprom_read_word(BAUD_HI_ADDR))) << 16;
	
	g_modbus_id = eeprom_read_byte(MODBUS_ID_ADDR);
	
	g_temp_alarm = eeprom_read_word(TEMP_ALARM_ADDR);
	g_temp_stop = eeprom_read_word(TEMP_STOP_ADDR);
	
	g_power_pwm_base = eeprom_read_byte(POWER_PWM_BASE_ADDR);
	g_power_pwm_shift = eeprom_read_byte(POWER_PWM_SHIFT_ADDR);
	g_max_power_pwm = eeprom_read_byte(MAX_POWER_PWM_ADDR);
	g_min_power_pwm = eeprom_read_byte(MIN_POWER_PWM_ADDR);
	g_autosearch_mode = eeprom_read_byte(AUTOSEARCH_MODE_ADDR);
	fault_interrupts_init(eeprom_read_byte(FAULT_INTERRUPTS_MODE_ADDR));
}

void storeToEE(void)
{
	eeprom_write_word(FREQ_UPPER_ADDR, (uint16_t)g_freq_upper);
	eeprom_write_word(FREQ_LOWER_ADDR, (uint16_t)g_freq_lower);
	eeprom_write_word(FREQ_ADDR, (uint16_t)g_dds_freq);
	eeprom_write_byte(PWM_BASE_ADDR, g_bias_pwm_base);
	eeprom_write_byte(PWM_SHIFT_ADDR, g_bias_pwm_shift);
	
	eeprom_write_byte(ADC0_DELAY_ADDR, adc_get_delay(0));
	eeprom_write_word(ADC0_COUNT_ADDR, adc_get_count(0));
	eeprom_write_byte(ADC1_DELAY_ADDR, adc_get_delay(1));
	eeprom_write_word(ADC1_COUNT_ADDR, adc_get_count(1));
	eeprom_write_byte(ADC2_DELAY_ADDR, adc_get_delay(2));
	eeprom_write_word(ADC2_COUNT_ADDR, adc_get_count(2));
//	eeprom_write_byte(ADC3_DELAY_ADDR, adc_get_delay(3));
//	eeprom_write_word(ADC3_COUNT_ADDR, adc_get_count(3));
	
	eeprom_write_byte(PFC_MODE_ADDR, g_pfc_mode);
	eeprom_write_word(INT_TIMEOUT_ADDR, g_int_timeout);
	eeprom_write_byte(KEEP_MODE_ADDR, g_keep_mode);
	eeprom_write_byte(KEEP_STEP_ADDR, g_keep_freq_step);
	eeprom_write_byte(KEEP_DELTA_ADDR, g_keep_freq_max_delta);
	eeprom_write_byte(MAX_PWM_ADDR, g_max_bias_pwm);
	eeprom_write_byte(MIN_PWM_ADDR, g_min_bias_pwm);
	eeprom_write_word(BAUD_LO_ADDR, (uint16_t)(g_baudrate & 0x0000FFFF));
	eeprom_write_word(BAUD_HI_ADDR, (uint16_t)(g_baudrate >> 16));
	
	eeprom_write_byte(MODBUS_ID_ADDR, g_modbus_id);
	eeprom_write_word(TEMP_ALARM_ADDR, g_temp_alarm);
	eeprom_write_word(TEMP_STOP_ADDR, g_temp_stop);

	eeprom_write_byte(POWER_PWM_BASE_ADDR, g_power_pwm_base);
	eeprom_write_byte(POWER_PWM_SHIFT_ADDR, g_power_pwm_shift);
	eeprom_write_byte(MAX_POWER_PWM_ADDR, g_max_power_pwm);
	eeprom_write_byte(MIN_POWER_PWM_ADDR, g_min_power_pwm);
	eeprom_write_byte(AUTOSEARCH_MODE_ADDR, g_autosearch_mode);
	eeprom_write_byte(FAULT_INTERRUPTS_MODE_ADDR, g_fault_interrupts_mode);
}

void reset_settings(void)
{
	g_freq_upper = 21400;
	g_freq_lower = 21250;
	dds_setfreq(21350);

	g_bias_pwm_base = 100;
	g_bias_pwm_shift = 0;
	
	g_int_timeout = 200;
	g_keep_mode = KEEP_OFF;
	
	g_autosearch_mode = AUTOSEARCH_OFF;
	g_fault_interrupts_mode = FAULT_INTERRUPTS_OFF;
	
	g_keep_freq_step = 1;
	g_keep_freq_max_delta = 5;
	
	g_max_bias_pwm = 255;
	g_min_bias_pwm = 10;
	
	g_baudrate = 115200;
	g_modbus_id = 1;
	
	g_temp_alarm = 75;
	g_temp_stop = 80;
	
	adc_set_delay(0, 1);
	adc_set_count(0, 200);

	adc_set_delay(1, 1);
	adc_set_count(1, 200);

	adc_set_delay(2, 1);
	adc_set_count(2, 200);

//	adc_set_delay(3, 1);
//	adc_set_count(3, 400);
	
	set_pfc_mode(PFC_AUTO);
	
	g_power_pwm_base = 95;
	g_power_pwm_shift = 0;

	g_max_power_pwm = 99;
	g_min_power_pwm = 69;
}

void menu_int_timeout(void)
{
	sprintf(lcd_line1, "INTEGR. TIMEOUT= %-3d", g_int_timeout);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_int_timeout++;
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		g_int_timeout--;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_modbus_id(void)
{
	sprintf(lcd_line1, "DEVICE ADDRESS= %-4d", g_modbus_id);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_modbus_id++;
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		g_modbus_id--;
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

uint32_t	baud[] = {2400, 4800, 9600, 19200, 38400, 57600, 115200};

void menu_baudrate(void)
{
	uint8_t		idx;
	
	for (idx = 0; idx < 7; idx++)
		if (g_baudrate == baud[idx])
			break;
	
	sprintf(lcd_line1, "BAUDRATE= %-10ld", g_baudrate);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		idx++;
		
		if (7 == idx)
			idx = 0;
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (0 == idx)
			idx = 7;
			
		idx--;
		
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}
	
	g_baudrate = baud[idx];

	menu_common();
}

void menu_temp_alarm(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (0 < g_temp_alarm)
			g_temp_alarm--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_temp_stop > g_temp_alarm)
			g_temp_alarm++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "CRITICAL TEMP.= %-d—   ", g_temp_alarm);
	
	menu_common();
}

void menu_temp_stop(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_temp_alarm < g_temp_stop)
			g_temp_stop--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (95 > g_temp_stop)
			g_temp_stop++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "STOP TEMP.= %-d—      ", g_temp_stop);
	
	menu_common();
}

char	stop_mode_str[5][15] = {"BUTTON", "485", "OVERHEAT", "FREQUENCY O/L", "CURRENT O/L"};

void menu_stop_mode(void)
{
	sprintf(lcd_line1, "STOP:%-15s", stop_mode_str[(uint8_t)g_stop_mode]);
	menu_common();
}

void menu_version(void)
{
	sprintf(lcd_line1, "FW VERSION:%-10s", FW_VERSION);
	menu_common();
}


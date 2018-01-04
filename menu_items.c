#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
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

#define FAST_MODE_COUNT		(75)
#define FAST_MODE_DELAY		(500)
#define FAST_MODE_STEP		(20)

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
void check_settings(void);
void fault_interrupts_init(fault_interrupts_mode_e);

void menu_items_init(void)
{
	uint8_t		idx;
	
	idx = 0; 
//	menu_items[MENU_MODE_WORK][idx++] = menu_freq;
	menu_items[MENU_MODE_WORK][idx++] = menu_search;
#ifdef _POWER_CHANGEABLE
	menu_items[MENU_MODE_WORK][idx++] = menu_power;
#endif // _POWER_CHANGEABLE
//	menu_items[MENU_MODE_WORK][idx++] = menu_mult;	
#ifdef _BIAS_CHANGEABLE
	menu_items[MENU_MODE_WORK][idx++] = menu_current;	
#endif // _BIAS_CHANGEABLE
	menu_items[MENU_MODE_WORK][idx++] = menu_amp;
	menu_items[MENU_MODE_WORK][idx++] = menu_temp;
	menu_items[MENU_MODE_WORK][idx++] = menu_temp2;
//	menu_items[MENU_MODE_WORK][idx++] = menu_monitor;
	menu_items[MENU_MODE_WORK][idx++] = menu_stop_mode;

#ifdef _ADC_SHOW
	menu_items[MENU_MODE_WORK][idx++] = menu_adc0;
	menu_items[MENU_MODE_WORK][idx++] = menu_adc1;
	menu_items[MENU_MODE_WORK][idx++] = menu_adc2;
//	menu_items[MENU_MODE_WORK][idx++] = menu_adc3;
#endif // _ADC_SHOW

	menu_items[MENU_MODE_WORK][idx++] = menu_version;
	menu_items[MENU_MODE_WORK][idx++] = menu_din;

	
	g_menu_search_auto_idx = idx;
	menu_items[MENU_MODE_WORK][idx++] = menu_search_auto;

	idx = 0;
	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_freq_upper;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_freq_lower;

#ifdef _BIAS_CHANGEABLE
#ifdef _MAX_BIAS_CHANGEABLE
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_max_bias_pwm;
#endif // _MAX_BIAS_CHANGEABLE
#ifdef _MIN_BIAS_CHANGEABLE
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_min_bias_pwm;	
#endif // _MIN_BIAS_CHANGEABLE	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_bias_pwm_base;
#endif // _BIAS_CHANGEABLE
#ifdef _BIAS_SHIFT_CHANGEABLE
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_bias_pwm_shift;
#endif // _BIAS_SHIFT_CHANGEABLE
	//menu_items[MENU_MODE_SETTINGS][idx++] = menu_bias_pwm_multiplier;
	//menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc_multiplier;

#ifdef _POWER_CHANGEABLE	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_max_power_pwm;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_min_power_pwm;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_power_pwm_base;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_power_pwm_shift;
#endif // _POWER_CHANGEABLE

#ifdef _INT_TIMEOUT_CHANGEABLE
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_int_timeout; //-
#endif // _INT_TIMEOUT_CHANGEABLE
	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_pfc_mode;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_autosearch_mode;

#ifdef _STARTBUTTON_ENABLED
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_startbutton;
#endif // _STARTBUTTON_ENABLED

#ifdef _KEEP_CHANGEABLE
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_keep_mode;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_keep_step;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_keep_delta;	
#endif // _KEEP_CHANGEABLE
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_temp_alarm;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_temp_stop;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_temp2_alarm;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_temp2_stop;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_fault_interrupts;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_modbus_id;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_baudrate;	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_store_settings;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_reset_settings;

//-	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc0_count;
//	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc0_delay;
//-	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc1_count;
//	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc1_delay;
//-	menu_items[MENU_MODE_SETTINGS][idx++] = menu_adc2_count;
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
		if (STARTBUTTON_OFF == g_startbutton_mode)
			start();
		CLEAR_KEY_PRESSED(KEY_RUN);
	}

	if (KEY_PRESSED(KEY_STOP))
	{
		if (STARTBUTTON_OFF == g_startbutton_mode)
		{
			if (IS_UZG_RUN)
				stop(STOP_BUTTON);
			else
			{
				g_bias_alarm = 0;
				g_pwm_alarm = 0;
				stop(STOP_NOT_CHANGE);
			}
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
			delay_ms(g_int_timeout + g_int_timeout + adc_get_timeout(ADC_BIAS_CURRENT));

			sprintf(lcd_line1, "BIAS:%d ADC0%d", g_bias_pwm, adc_mean_value(ADC_BIAS_CURRENT));
			do_lcd();
	
			sprintf(buf, "BIAS_PWM\t%d\tADC0\t%d\n", g_bias_pwm, adc_mean_value(ADC_BIAS_CURRENT));
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
	sprintf(lcd_line1, "TEMP.1:%.1fC               ", temp_value(0));
	
	menu_common();
}

void menu_temp2(void)
{
	sprintf(lcd_line1, "TEMP.2:%.1fC               ", temp_value(1));
	
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
	sprintf(lcd_line1, "POWER= %2d%%               ", (g_power_pwm + 1) * 10);

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
		if (g_max_bias_pwm - g_bias_pwm_shift < g_bias_pwm_base)
			g_bias_pwm_base = g_max_bias_pwm - g_bias_pwm_shift;
		else
			g_bias_pwm_base ++;
		
		normalize_bias_pwm_base();
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_bias_pwm_base < g_min_bias_pwm)
			g_bias_pwm_base = g_min_bias_pwm;
		else
			g_bias_pwm_base --;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}


void menu_bias_pwm_shift(void)
{
	sprintf(lcd_line1, "ADD. CURRENT= %.2fA       ", bias_pwm_to_current(g_bias_pwm_shift));

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_max_bias_pwm - g_bias_pwm_shift < g_bias_pwm_base)
			g_bias_pwm_shift = g_max_bias_pwm - g_bias_pwm_base;
		else
			g_bias_pwm_shift++;
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_bias_pwm_shift < g_bias_pwm_step)
			g_bias_pwm_shift = 0;
		else
			g_bias_pwm_shift--;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_power_pwm_base(void)
{
	sprintf(lcd_line1, "START POWER= %2d%%     ", (g_power_pwm_base + 1) * 10);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_max_power_pwm - g_power_pwm_shift <= g_power_pwm_base)
			g_power_pwm_base = g_max_power_pwm - g_power_pwm_shift;
		else
			g_power_pwm_base++;
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_power_pwm_base <= g_min_power_pwm)
			g_power_pwm_base = g_min_power_pwm;
		else
			g_power_pwm_base--;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}


void menu_power_pwm_shift(void)
{
	sprintf(lcd_line1, "ADD. POWER= %2d%%       ", g_power_pwm_shift * 10);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_max_power_pwm - g_power_pwm_shift <= g_power_pwm_base)
			g_power_pwm_shift = g_max_power_pwm - g_power_pwm_base;
		else
			g_power_pwm_shift++;
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_power_pwm_shift <= 0 || g_max_power_pwm - g_power_pwm_shift < g_power_pwm_base)
			g_power_pwm_shift = 0;
		else
			g_power_pwm_shift--;

			
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
		if (POWER_PWM_MAX > g_max_power_pwm)
			g_max_power_pwm++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "MAX POWER= %d%%       ", (g_max_power_pwm + 1) * 10);
	
	menu_common();
}


void menu_min_power_pwm(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (POWER_PWM_MIN < g_min_power_pwm)
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
		
	sprintf(lcd_line1, "MIN POWER= %d%%         ", (g_min_power_pwm + 1) * 10);
	
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
		
			timeout = adc_get_timeout(ADC_FEEDBACK_CURRENT) + g_int_timeout; // + adc_get_timeout(0);
		
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

		if (!IS_UZG_RUN)
		{
			g_autosearch_running = 0;
			
			if (0 != timer_id)
				stop_timer(timer_id);
				timer_id = 0;
				
			menu_item_next();
			return;
		}
		
		
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
					current = adc_mean_value(ADC_FEEDBACK_CURRENT);
					sei();

					sprintf(lcd_line1, "SEARCH F:%-5ld C:%-4.2f", g_dds_freq, adc_feedback_to_current(current));

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

	static uint32_t	last_mseconds = 0;
	static int		fast_mode_count = 0;
	
	if (KEY_PRESSED(KEY_LEFT) || KEY_PRESSED(KEY_RIGHT))
	{
		if (timer_mseconds_total - last_mseconds < FAST_MODE_DELAY)
		{
			
			if (fast_mode_count < FAST_MODE_COUNT)
				fast_mode_count++;
		}
		else
		{
			fast_mode_count = 0;
		}
		last_mseconds = timer_mseconds_total;
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (fast_mode_count >= FAST_MODE_COUNT)
			g_dds_freq+=FAST_MODE_STEP;
		else
			g_dds_freq++;
			
		if (g_dds_freq > g_freq_upper)
			g_dds_freq = g_freq_upper;
			
		dds_setfreq(g_dds_freq);
	
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (fast_mode_count >= FAST_MODE_COUNT)
			g_dds_freq-=FAST_MODE_STEP;
		else
			g_dds_freq--;
			
		if (g_dds_freq < g_freq_lower)
			g_dds_freq = g_freq_lower;
			
		dds_setfreq(g_dds_freq);

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	current_value = adc_mean_value(ADC_FEEDBACK_CURRENT);
	bias_value = adc_mean_value(ADC_BIAS_CURRENT);
//	amp_value = adc_mean_value(ADC_AMP);
	
	curr = adc_feedback_to_current(current_value);
	if (curr < 0.)
		curr = 0.;
	
	bias = adc_bias_to_current(bias_value);
	if (bias < 0.)
		bias = 0.;
	
#ifdef _BIAS_CHANGEABLE
	sprintf(lcd_line1, "F=%-5ld C:%-4.2f T:%-3.2f     ",
		g_dds_freq, curr, bias);
#else
	sprintf(lcd_line1, "   F=%-5ld C:%-4.2f           ",
		g_dds_freq, curr);
#endif // _BIAS_CHANGEABLE
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
	static uint32_t	last_mseconds = 0;
	static int		fast_mode_count = 0;
	
	if (KEY_PRESSED(KEY_LEFT) || KEY_PRESSED(KEY_RIGHT))
	{
		if (timer_mseconds_total - last_mseconds < FAST_MODE_DELAY)
		{
			
			if (fast_mode_count < FAST_MODE_COUNT)
				fast_mode_count++;
		}
		else
		{
			fast_mode_count = 0;
		}
		last_mseconds = timer_mseconds_total;
	}
	
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (fast_mode_count >= FAST_MODE_COUNT)
			g_freq_lower -= FAST_MODE_STEP;
		else
			g_freq_lower--;

		if (g_freq_supermin > g_freq_lower)
			g_freq_lower = g_freq_supermin;
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (fast_mode_count >= FAST_MODE_COUNT)
			g_freq_lower += FAST_MODE_STEP;
		else
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
	static uint32_t	last_mseconds = 0;
	static int		fast_mode_count = 0;
	
	if (KEY_PRESSED(KEY_LEFT) || KEY_PRESSED(KEY_RIGHT))
	{
		if (timer_mseconds_total - last_mseconds < FAST_MODE_DELAY)
		{
			
			if (fast_mode_count < FAST_MODE_COUNT)
				fast_mode_count++;
		}
		else
		{
			fast_mode_count = 0;
		}
		last_mseconds = timer_mseconds_total;
	}
	

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (fast_mode_count >= FAST_MODE_COUNT)
			g_freq_upper -= FAST_MODE_STEP;
		else
			g_freq_upper--;

		if (g_freq_upper < g_freq_lower)
			g_freq_upper = g_freq_lower;
	
		if (g_dds_freq > g_freq_upper)
			dds_setfreq(g_freq_upper);
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (fast_mode_count >= FAST_MODE_COUNT)
			g_freq_upper += FAST_MODE_STEP;
		else
			g_freq_upper++;

		if (g_freq_supermax < g_freq_upper)
			g_freq_upper = g_freq_supermax;
						
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
		
	sprintf(lcd_line1, "PFC:%-16s", pfc_mode_str[g_pfc_mode]);
	
	set_pfc_mode(g_pfc_mode);
	
	menu_common();
}

char	keep_mode_str[KEEP_COUNT][6] = {"", "OFF", "CURR.", "AMP."};

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
/*
void menu_adc_multiplier(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_adc_multiplier > 20)
			g_adc_multiplier--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (200 > g_adc_multiplier)
			g_adc_multiplier++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "ADC MULT= %-10d", g_adc_multiplier);
	
	menu_common();
}
*/
void menu_bias_pwm_multiplier(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_bias_pwm_multiplier > MIN_BIAS_PWM_MULTIPLIER)
			g_bias_pwm_multiplier--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (MAX_BIAS_PWM_MULTIPLIER > g_bias_pwm_multiplier)
			g_bias_pwm_multiplier++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "BIAS PWM MULT= %-5d", g_bias_pwm_multiplier);
	
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
		if ((bias_pwm_to_current(g_max_bias_pwm + 1) < g_supermax_bias_pwm / 10.) && (g_max_bias_pwm < 255))
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
		check_settings();
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
		check_settings();
		storeToEE();
		sprintf(lcd_line1, "DEFAULT SETTINGS    ");
		do_lcd();
		beep_ms(500);
	
		CLEAR_KEY_PRESSED(KEY_ENTER);
	}
	
	sprintf(lcd_line1, "RESET SETTINGS      ");
	
	menu_common();
}

void loadFromEE(void)
{
	int i;

	g_freq_supermax = eeprom_read_word(SUPERMAX_FREQ_ADDR);
	g_freq_supermin = eeprom_read_word(SUPERMIN_FREQ_ADDR);
	
	g_freq_upper = (uint32_t)eeprom_read_word(FREQ_UPPER_ADDR);
	g_freq_lower = (uint32_t)eeprom_read_word(FREQ_LOWER_ADDR);
	g_dds_freq = (uint32_t)eeprom_read_word(FREQ_ADDR);
	dds_setfreq(g_dds_freq);
		
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
	
	g_temp_alarm[0] = eeprom_read_word(TEMP_ALARM_ADDR);
	g_temp_stop[0] = eeprom_read_word(TEMP_STOP_ADDR);
	g_temp_alarm[1] = eeprom_read_word(TEMP2_ALARM_ADDR);
	g_temp_stop[1] = eeprom_read_word(TEMP2_STOP_ADDR);

	
	g_power_pwm_base = eeprom_read_byte(POWER_PWM_BASE_ADDR);
	g_power_pwm_shift = eeprom_read_byte(POWER_PWM_SHIFT_ADDR);
	g_max_power_pwm = eeprom_read_byte(MAX_POWER_PWM_ADDR);
	g_min_power_pwm = eeprom_read_byte(MIN_POWER_PWM_ADDR);
	g_autosearch_mode = eeprom_read_byte(AUTOSEARCH_MODE_ADDR);
	fault_interrupts_init(eeprom_read_byte(FAULT_INTERRUPTS_MODE_ADDR));

	g_bias_pwm_multiplier = eeprom_read_word(BIAS_PWM_MULTIPLIER_ADDR);
	//g_adc_multiplier = eeprom_read_byte(ADC_MULTIPLIER_ADDR);
	
	for (i = 0; i < DIN_SIZE; i++)
		g_din[i] = eeprom_read_byte(DIN_ADDR + i);
	g_din[DIN_SIZE - 1] = 0;
	
	g_supermax_bias_pwm = eeprom_read_byte(SUPERMAX_BIAS_PWM_ADDR);
	
	adc[0].bias = eeprom_read_word(ADC0_BIAS_ADDR);
	adc[1].bias = eeprom_read_word(ADC1_BIAS_ADDR);
	adc[2].bias = eeprom_read_word(ADC2_BIAS_ADDR);
//	adc[3].bias = eeprom_read_word(ADC3_BIAS_ADDR);
	
	g_adc_bias_multiplier = eeprom_read_byte(ADC_BIAS_MULTIPLIER_ADDR);
	g_adc_feedback_multiplier = eeprom_read_byte(ADC_FEEDBACK_MULTIPLIER_ADDR);
	
	g_startbutton_mode = eeprom_read_byte(STARTBUTTON_MODE_ADDR);
	
	g_fault_interrupts_mode = eeprom_read_byte(FAULT_INTERRUPTS_MODE_ADDR);
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
	eeprom_write_word(TEMP_ALARM_ADDR, g_temp_alarm[0]);
	eeprom_write_word(TEMP_STOP_ADDR, g_temp_stop[0]);

	eeprom_write_word(TEMP2_ALARM_ADDR, g_temp_alarm[1]);
	eeprom_write_word(TEMP2_STOP_ADDR, g_temp_stop[1]);

	eeprom_write_byte(POWER_PWM_BASE_ADDR, g_power_pwm_base);
	eeprom_write_byte(POWER_PWM_SHIFT_ADDR, g_power_pwm_shift);
	eeprom_write_byte(MAX_POWER_PWM_ADDR, g_max_power_pwm);
	eeprom_write_byte(MIN_POWER_PWM_ADDR, g_min_power_pwm);
	eeprom_write_byte(AUTOSEARCH_MODE_ADDR, g_autosearch_mode);
	eeprom_write_byte(FAULT_INTERRUPTS_MODE_ADDR, g_fault_interrupts_mode);
	
	eeprom_write_byte(STARTBUTTON_MODE_ADDR, g_startbutton_mode);
	
//	eeprom_write_word(BIAS_PWM_MULTIPLIER_ADDR, g_bias_pwm_multiplier);
//	eeprom_write_byte(ADC_MULTIPLIER_ADDR, g_adc_multiplier);
}

void reset_settings(void)
{

	g_freq_upper = g_freq_supermax;

	if (g_freq_upper > g_freq_supermax)
		g_freq_upper = g_freq_supermax;
		
	if (g_freq_upper < g_freq_supermin)
		g_freq_upper = g_freq_supermin;
		
	g_freq_lower = g_freq_supermin;

	if (g_freq_lower < g_freq_supermin)
		g_freq_lower = g_freq_supermin;
		
	if (g_freq_lower > g_freq_upper)
		g_freq_lower = g_freq_supermin;

	dds_setfreq((g_freq_upper + g_freq_lower) / 2);

	g_max_bias_pwm = 255;

	while (bias_pwm_to_current(g_max_bias_pwm) > g_supermax_bias_pwm / 10.)
			g_max_bias_pwm--;

	g_min_bias_pwm = MIN_BIAS_PWM;

	g_bias_pwm_base = DEFAULT_BIAS_PWM_BASE;
	g_bias_pwm_shift = DEFAULT_BIAS_PWM_SHIFT;
	
	g_int_timeout = DEFAULT_INT_TIMEOUT;
	g_keep_mode = KEEP_CURRENT;

#ifdef _STARTBUTTON_ENABLED
	g_autosearch_mode = AUTOSEARCH_OFF;
#else 
	g_autosearch_mode = AUTOSEARCH_ON;
#endif //_STARTBUTTON_ENABLED

	g_fault_interrupts_mode = FAULT_INTERRUPTS_ON;
	
	g_keep_freq_step = 5;
	g_keep_freq_max_delta = 1;
	
	g_baudrate = 9600;
	g_modbus_id = 1;
	
	g_temp_alarm[0] = 75;
	g_temp_stop[0] = 80;
	
	g_temp_alarm[1] = 60;
	g_temp_stop[1] = 65;

	adc_set_delay(0, 1);
	adc_set_count(0, 200);

	adc_set_delay(1, 1);
	adc_set_count(1, 200);

	adc_set_delay(2, 1);
	adc_set_count(2, 200);

//	adc_set_delay(3, 1);
//	adc_set_count(3, 400);
	
	set_pfc_mode(DEFAULT_PFC_MODE);
	
	g_power_pwm_base = 8;
	g_power_pwm_shift = 0;

	g_max_power_pwm = POWER_PWM_MAX;
	g_min_power_pwm = POWER_PWM_MIN;
	
#ifdef _STARTBUTTON_ENABLED
	g_startbutton_mode = STARTBUTTON_ON;
#else 
	g_startbutton_mode = STARTBUTTON_OFF;
#endif //_STARTBUTTON_ENABLED
}

void check_settings(void)
{
	if (MIN_BIAS_PWM_MULTIPLIER > g_bias_pwm_multiplier || g_bias_pwm_multiplier > MAX_BIAS_PWM_MULTIPLIER)
	{	
		g_bias_pwm_multiplier = DEFAULT_BIAS_PWM_MULTIPLIER;
		eeprom_write_word(BIAS_PWM_MULTIPLIER_ADDR, g_bias_pwm_multiplier);
	}
	
	if (10 > g_adc_bias_multiplier || g_adc_bias_multiplier > 60)
	{
		g_adc_bias_multiplier = 55;
		eeprom_write_byte(ADC_BIAS_MULTIPLIER_ADDR, g_adc_bias_multiplier);
	}
		
	if (20 > g_adc_feedback_multiplier || g_adc_feedback_multiplier > 60)
	{
		g_adc_feedback_multiplier = 30;
		eeprom_write_byte(ADC_FEEDBACK_MULTIPLIER_ADDR, g_adc_feedback_multiplier);
	}

	if (adc[0].bias > 0 || adc[0].bias < 1000)
	{
		adc[0].bias = 511;
		eeprom_write_word(ADC0_BIAS_ADDR, adc[0].bias);
	}

	if (adc[1].bias > 520 || adc[1].bias < 500)
	{
		adc[1].bias = 511;
		eeprom_write_word(ADC1_BIAS_ADDR, adc[1].bias);
	}

	if (adc[2].bias > 520 || adc[2].bias < 500)
	{
		adc[2].bias = 511;
		eeprom_write_word(ADC2_BIAS_ADDR, adc[2].bias);
	}
	
	if (DDS_MAX_FREQ < g_freq_supermax || DDS_MIN_FREQ > g_freq_supermax)
	{
		g_freq_supermax = DDS_MAX_FREQ;
		eeprom_write_word(SUPERMAX_FREQ_ADDR, (uint16_t)g_freq_supermax);
	}

	if (g_freq_supermin > g_freq_supermax || DDS_MIN_FREQ > g_freq_supermin)
	{
		g_freq_supermin = DDS_MIN_FREQ;
		eeprom_write_word(SUPERMIN_FREQ_ADDR, (uint16_t)g_freq_supermin);
	}
#ifdef _NARROW_FREQ
	g_freq_supermax = DDS_MAX_FREQ;
	g_freq_supermin = DDS_MIN_FREQ; 
#endif // _NARROW_FREQ
	
	if (g_freq_upper > g_freq_supermax)
		g_freq_upper = g_freq_supermax;
		
	if (g_freq_upper < g_freq_supermin)
		g_freq_upper = g_freq_supermin;
		
	if (g_freq_lower < g_freq_supermin)
		g_freq_lower = g_freq_supermin;
		
	if (g_freq_lower > g_freq_upper)
		g_freq_lower = g_freq_supermin;

	if (g_dds_freq > g_freq_upper || g_dds_freq < g_freq_lower)
		dds_setfreq((g_freq_upper + g_freq_lower) / 2);

#ifdef _SUPERMAX_BIAS_CHANGEABLE		
	if (g_supermax_bias_pwm > SUPERMAX_BIAS_PWM)
#endif // _SUPERMAX_BIAS_CHANGEABLE
	{
		g_supermax_bias_pwm = SUPERMAX_BIAS_PWM;
		eeprom_write_byte(SUPERMAX_BIAS_PWM_ADDR, g_supermax_bias_pwm);
	}

	while (bias_pwm_to_current(g_max_bias_pwm) > g_supermax_bias_pwm / 10.)
			g_max_bias_pwm--;

#ifdef _MIN_BIAS_CHANGEABLE		
	if (g_min_bias_pwm > g_max_bias_pwm)
#endif // _MIN_BIAS_CHANGEABLE		
		g_min_bias_pwm = MIN_BIAS_PWM;

	if (g_min_bias_pwm > g_max_bias_pwm)
		g_min_bias_pwm = g_max_bias_pwm;
		
	if (g_bias_pwm_base > g_max_bias_pwm)
		g_bias_pwm_base = g_max_bias_pwm;

	if (g_bias_pwm_base < g_min_bias_pwm)
		g_bias_pwm_base = g_min_bias_pwm;

#ifdef _BIAS_SHIFT_CHANGEABLE		
	if (g_bias_pwm_shift > g_max_bias_pwm - g_bias_pwm_base)
		g_bias_pwm_shift = g_max_bias_pwm - g_bias_pwm_base;
#else
	g_bias_pwm_shift = DEFAULT_BIAS_PWM_SHIFT;	
#endif // _BIAS_SHIFT_CHANGEABLE
	
	normalize_bias_pwm_base();
		
#ifdef _POWER_CHANGEABLE		
	if (POWER_PWM_MAX < g_max_power_pwm || POWER_PWM_MIN > g_max_power_pwm)
		g_max_power_pwm = POWER_PWM_MAX;

	if (POWER_PWM_MIN > g_min_power_pwm || g_max_power_pwm < g_min_power_pwm)
		g_min_power_pwm = POWER_PWM_MIN;

	if (g_min_power_pwm > g_max_power_pwm)
		g_min_power_pwm = g_max_power_pwm;
		
	if (g_power_pwm_base > g_max_power_pwm)
		g_power_pwm_base = g_max_power_pwm;

	if (g_power_pwm_base < g_min_power_pwm)
		g_power_pwm_base = g_min_power_pwm;

	if (g_power_pwm_shift > g_max_power_pwm - g_power_pwm_base)
		g_power_pwm_shift = g_max_power_pwm - g_power_pwm_base;
#else // _POWER_CHANGEABLE
	g_max_power_pwm = POWER_PWM_MAX;
	g_min_power_pwm = POWER_PWM_MIN;
	g_power_pwm_base = 8;
	g_power_pwm_shift = 0;
#endif // _POWER_CHANGEABLE
		
#ifdef _INT_TIMEOUT_CHANGEABLE
	if (g_int_timeout > 1000)
#endif // _INT_TIMEOUT_CHANGEABLE
		g_int_timeout = DEFAULT_INT_TIMEOUT;
	
#ifdef _KEEP_CHANGEABLE
	if (g_keep_mode >= KEEP_COUNT)
#endif // _KEEP_CHANGEABLE
		g_keep_mode = KEEP_CURRENT;
	
	if (g_autosearch_mode >= AUTOSEARCH_COUNT)
#ifdef _STARTBUTTON_ENABLED
	g_autosearch_mode = AUTOSEARCH_OFF;
#else 
	g_autosearch_mode = AUTOSEARCH_ON;
#endif //_STARTBUTTON_ENABLED
		
	if (g_fault_interrupts_mode < FAULT_INTERRUPTS_OFF || g_fault_interrupts_mode >= FAULT_INTERRUPTS_COUNT)
        g_fault_interrupts_mode = FAULT_INTERRUPTS_ON;
	
	if (g_keep_freq_step > 10 || 0 > g_keep_freq_step)
		g_keep_freq_step = 5;
	
	if (g_keep_freq_max_delta > 10 || 0 > g_keep_freq_max_delta)
		g_keep_freq_max_delta = 1;
		
	if (g_temp_stop[0] > 80 || g_temp_stop[0] < 0)
		g_temp_stop[0] = 80;
		
	if (g_temp_alarm[0] > 75 || g_temp_alarm[0] < 0)
		g_temp_alarm[0] = 75;

	if (g_temp_alarm[0] > g_temp_stop[0])
		g_temp_alarm[0] = g_temp_stop[0];

	if (g_temp_stop[1] > 65 || g_temp_stop[1] < 0)
		g_temp_stop[1] = 65;
		
	if (g_temp_alarm[1] > 60 || g_temp_alarm[1] < 0)
		g_temp_alarm[1] = 60;

	if (g_temp_alarm[1] > g_temp_stop[1])
		g_temp_alarm[1] = g_temp_stop[1];

	adc_set_delay(0, 1);
	adc_set_count(0, 200);

	adc_set_delay(1, 1);
	adc_set_count(1, 200);

	adc_set_delay(2, 1);
	adc_set_count(2, 200);

//	adc_set_delay(3, 1);
//	adc_set_count(3, 400);
	
	if (PFC_OFF > g_pfc_mode || g_pfc_mode >= PFC_COUNT)
		set_pfc_mode(DEFAULT_PFC_MODE);
			
	g_din[DIN_SIZE - 1] = 0;
	
	if (STARTBUTTON_OFF > g_startbutton_mode || g_startbutton_mode >= STARTBUTTON_COUNT)
		g_startbutton_mode = STARTBUTTON_OFF;

#ifndef _STARTBUTTON_ENABLED
	g_startbutton_mode = STARTBUTTON_OFF;
#endif // ! _STARTBUTTON_ENABLED
		
}

void menu_int_timeout(void)
{
	sprintf(lcd_line1, "INTEGR. TIMEOUT= %-3d", g_int_timeout);

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_int_timeout < 1000)
			g_int_timeout++;
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_int_timeout > 0)
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
		if (0 < g_temp_alarm[0])
			g_temp_alarm[0]--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_temp_stop[0] > g_temp_alarm[0])
			g_temp_alarm[0]++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "CRIT. TEMP.1= %-d—   ", g_temp_alarm[0]);
	
	menu_common();
}

void menu_temp_stop(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_temp_alarm[0] < g_temp_stop[0])
			g_temp_stop[0]--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (95 > g_temp_stop[0])
			g_temp_stop[0]++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "STOP TEMP.1= %-d—      ", g_temp_stop[0]);
	
	menu_common();
}

void menu_temp2_alarm(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (0 < g_temp_alarm[1])
			g_temp_alarm[1]--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (g_temp_stop[1] > g_temp_alarm[1])
			g_temp_alarm[1]++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "CRIT. TEMP.2= %-d—   ", g_temp_alarm[1]);
	
	menu_common();
}

void menu_temp2_stop(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (g_temp_alarm[1] < g_temp_stop[1])
			g_temp_stop[1]--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		if (95 > g_temp_stop[1])
			g_temp_stop[1]++;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "STOP TEMP.2= %-d—      ", g_temp_stop[1]);
	
	menu_common();
}

char	stop_mode_str[6][15] = {"BUTTON", "485", "OVERHEAT 1", "OVERHEAT 2", "FREQUENCY O/L", "CURRENT O/L"};

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

void menu_din(void)
{
	sprintf(lcd_line1, "DIN:%-16s", g_din);
	menu_common();
}

char	startbutton_mode_str[STARTBUTTON_COUNT][5] = {"OFF", "ON"};

void menu_startbutton(void)
{
	if (KEY_PRESSED(KEY_LEFT))
	{
		if (STARTBUTTON_OFF == g_startbutton_mode)
			g_startbutton_mode = STARTBUTTON_COUNT;
		g_startbutton_mode--;

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	if (KEY_PRESSED(KEY_RIGHT))
	{
		g_startbutton_mode++;
	
		if (STARTBUTTON_COUNT <= g_startbutton_mode)
			g_startbutton_mode = STARTBUTTON_OFF;

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}
		
	sprintf(lcd_line1, "REMOTE BUTTON:%-6s", startbutton_mode_str[g_startbutton_mode]);
	
	menu_common();
}

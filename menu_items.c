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
#include "voltage.h"

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
	menu_items[MENU_MODE_WORK][idx++] = menu_search;
#ifdef _POWER_CHANGEABLE
	menu_items[MENU_MODE_WORK][idx++] = menu_power;
#endif // _POWER_CHANGEABLE
	menu_items[MENU_MODE_WORK][idx++] = menu_current;	
#ifdef _VOLTAGE_CHANGEABLE
	menu_items[MENU_MODE_WORK][idx++] = menu_voltage;
#endif // _VOLTAGE_CHANGEABLE
//	menu_items[MENU_MODE_WORK][idx++] = menu_amp;
	menu_items[MENU_MODE_WORK][idx++] = menu_temp;
	menu_items[MENU_MODE_WORK][idx++] = menu_temp2;
	menu_items[MENU_MODE_WORK][idx++] = menu_stop_mode;

#ifdef _ADC_SHOW
	menu_items[MENU_MODE_WORK][idx++] = menu_adc0;
	menu_items[MENU_MODE_WORK][idx++] = menu_adc1;
//  menu_items[MENU_MODE_WORK][idx++] = menu_adc2;
//	menu_items[MENU_MODE_WORK][idx++] = menu_adc3;
#endif // _ADC_SHOW

	menu_items[MENU_MODE_WORK][idx++] = menu_version;
	menu_items[MENU_MODE_WORK][idx++] = menu_din;

	
	g_menu_search_auto_idx = idx;
	menu_items[MENU_MODE_WORK][idx++] = menu_search_auto;

	idx = 0;
	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_freq_upper;
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_freq_lower;

	menu_items[MENU_MODE_SETTINGS][idx++] = menu_start_bias;

#ifdef _VOLTAGE_CHANGEABLE
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_start_voltage;
#endif // _VOLTAGE_CHANGEABLE


#ifdef _POWER_CHANGEABLE	
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_start_power;
#endif // _POWER_CHANGEABLE

#ifdef _INT_TIMEOUT_CHANGEABLE
	menu_items[MENU_MODE_SETTINGS][idx++] = menu_int_timeout; //-
#endif // _INT_TIMEOUT_CHANGEABLE
	
//	menu_items[MENU_MODE_SETTINGS][idx++] = menu_pfc_mode;
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
    sprintf(lcd_line1, "CURRENT= %-3.1fA %d%%        ",
        get_bias_adc(),
        get_bias_pwm());

	if (KEY_PRESSED(KEY_RIGHT))
	{
        inc_bias_pwm();
			
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
        dec_bias_pwm();

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_power(void)
{
	sprintf(lcd_line1, "POWER= %2d%%               ", get_power_pwm());

	if (KEY_PRESSED(KEY_RIGHT))
	{
        inc_power_pwm();

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
        dec_power_pwm();

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_voltage(void)
{
	sprintf(lcd_line1, "VOLTAGE: %d%%               ", get_voltage_pwm());

	menu_common();
}

void menu_start_bias(void)
{
	sprintf(lcd_line1, "START CURRENT= %d%%    ", get_start_bias());

	if (KEY_PRESSED(KEY_RIGHT))
	{
		inc_start_bias();
		
		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
		dec_start_bias();
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}


void menu_start_power(void)
{
	sprintf(lcd_line1, "START POWER= %2d%%     ", get_start_power());

	if (KEY_PRESSED(KEY_RIGHT))
	{
	    inc_start_power();

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
        dec_start_power();
			
		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

	menu_common();
}

void menu_start_voltage(void)
{
	sprintf(lcd_line1, "START VOLTAGE= %d%%     ", get_start_voltage());

	if (KEY_PRESSED(KEY_RIGHT))
	{
	    inc_start_voltage();

		CLEAR_KEY_PRESSED(KEY_RIGHT);
	}

	if (KEY_PRESSED(KEY_LEFT))
	{
	    dec_start_voltage();

		CLEAR_KEY_PRESSED(KEY_LEFT);
	}

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
	if (KEY_PRESSED(KEY_ENTER))
	{
		if (!g_autosearch_running)
		{
			keep_stop();
		
			dds_setfreq(g_freq_lower);
			
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
			menu_item_next();
			return;
		}
		
		
		if (g_dds_freq < g_freq_upper)
		{
            if (((g_dds_freq - g_freq_lower) & 0x00000FFF) == 0)
            {
                sprintf(lcd_line1, "SEARCH F:%-5ld                       ", g_dds_freq);
                do_lcd();
            }

            g_dds_freq += g_keep_freq_step;
            dds_setfreq(g_dds_freq);
		}
		else
		{
            g_bias_alarm = 0;
            g_pwm_alarm = 0;
            stop(STOP_NOT_CHANGE);
			menu_item_next();
			g_autosearch_running = 0;
		}
	}
	else
	{
		sprintf(lcd_line1, "START SEARCH        ");
		menu_common();
	}
}

void menu_search_auto_ori(void)
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
	
	float		curr = 0.;

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
	
	curr = adc_feedback_to_current(current_value);
	if (curr < 0.)
		curr = 0.;
	
	sprintf(lcd_line1, "F=%-5ld C:%-4.2f V:%d%%     ",
		g_dds_freq, curr, get_voltage_pwm());
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

		if (DDS_MIN_FREQ > g_freq_lower)
			g_freq_lower = DDS_MIN_FREQ;
			
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
		
	sprintf(lcd_line1, "PFC:%-16s", pfc_mode_str[g_pfc_mode]);
	
	set_pfc_mode(g_pfc_mode);
	
	menu_common();
}

char	keep_mode_str[KEEP_COUNT][6] = {"", "OFF", "ON" /*"CURR."*/, "AMP."};

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

	g_freq_upper = (uint32_t)eeprom_read_word(FREQ_UPPER_ADDR);
	g_freq_lower = (uint32_t)eeprom_read_word(FREQ_LOWER_ADDR);
	g_dds_freq = (uint32_t)eeprom_read_word(FREQ_ADDR);
	dds_setfreq(g_dds_freq);
		
	set_start_bias(eeprom_read_byte(PWM_BASE_ADDR));
	
	set_pfc_mode(eeprom_read_byte(PFC_MODE_ADDR));
	
	g_int_timeout = eeprom_read_word(INT_TIMEOUT_ADDR);
	g_keep_mode = eeprom_read_byte(KEEP_MODE_ADDR);
	g_keep_freq_step = eeprom_read_byte(KEEP_STEP_ADDR);
	g_keep_freq_max_delta = eeprom_read_byte(KEEP_DELTA_ADDR);
	
	g_baudrate = (uint32_t)eeprom_read_word(BAUD_LO_ADDR);
	g_baudrate |= ((uint32_t)(eeprom_read_word(BAUD_HI_ADDR))) << 16;
	
	g_modbus_id = eeprom_read_byte(MODBUS_ID_ADDR);
	
	g_temp_alarm[0] = eeprom_read_word(TEMP_ALARM_ADDR);
	g_temp_stop[0] = eeprom_read_word(TEMP_STOP_ADDR);
	g_temp_alarm[1] = eeprom_read_word(TEMP2_ALARM_ADDR);
	g_temp_stop[1] = eeprom_read_word(TEMP2_STOP_ADDR);

	
	set_start_power(eeprom_read_byte(POWER_PWM_BASE_ADDR));

	g_autosearch_mode = eeprom_read_byte(AUTOSEARCH_MODE_ADDR);
	fault_interrupts_init(eeprom_read_byte(FAULT_INTERRUPTS_MODE_ADDR));

	for (i = 0; i < DIN_SIZE; i++)
		g_din[i] = eeprom_read_byte(DIN_ADDR + i);
	g_din[DIN_SIZE - 1] = 0;
	
	adc[ADC_BIAS_CURRENT].bias = eeprom_read_word(ADC0_BIAS_ADDR);
	adc[ADC_FEEDBACK_CURRENT].bias = eeprom_read_word(ADC1_BIAS_ADDR);
	adc[ADC_AMP].bias = eeprom_read_word(ADC2_BIAS_ADDR);
//	adc[3].bias = eeprom_read_word(ADC3_BIAS_ADDR);
	
	g_adc_bias_multiplier = eeprom_read_byte(ADC_BIAS_MULTIPLIER_ADDR);
	g_adc_feedback_multiplier = eeprom_read_byte(ADC_FEEDBACK_MULTIPLIER_ADDR);
	
	g_startbutton_mode = eeprom_read_byte(STARTBUTTON_MODE_ADDR);
	
	g_fault_interrupts_mode = eeprom_read_byte(FAULT_INTERRUPTS_MODE_ADDR);

    set_start_voltage(eeprom_read_byte(VOLTAGE_PWM_BASE_ADDR));
    set_default_real_voltage(eeprom_read_byte(DEFAULT_VOLTAGE_ADDR));
}

void storeToEE(void)
{
	eeprom_write_word(FREQ_UPPER_ADDR, (uint16_t)g_freq_upper);
	eeprom_write_word(FREQ_LOWER_ADDR, (uint16_t)g_freq_lower);
	eeprom_write_word(FREQ_ADDR, (uint16_t)g_dds_freq);
	eeprom_write_byte(PWM_BASE_ADDR, get_start_bias());
	
	eeprom_write_byte(PFC_MODE_ADDR, g_pfc_mode);
	eeprom_write_word(INT_TIMEOUT_ADDR, g_int_timeout);
	eeprom_write_byte(KEEP_MODE_ADDR, g_keep_mode);
	eeprom_write_byte(KEEP_STEP_ADDR, g_keep_freq_step);
	eeprom_write_byte(KEEP_DELTA_ADDR, g_keep_freq_max_delta);
	eeprom_write_word(BAUD_LO_ADDR, (uint16_t)(g_baudrate & 0x0000FFFF));
	eeprom_write_word(BAUD_HI_ADDR, (uint16_t)(g_baudrate >> 16));
	
	eeprom_write_byte(MODBUS_ID_ADDR, g_modbus_id);
	eeprom_write_word(TEMP_ALARM_ADDR, g_temp_alarm[0]);
	eeprom_write_word(TEMP_STOP_ADDR, g_temp_stop[0]);

	eeprom_write_word(TEMP2_ALARM_ADDR, g_temp_alarm[1]);
	eeprom_write_word(TEMP2_STOP_ADDR, g_temp_stop[1]);

	eeprom_write_byte(POWER_PWM_BASE_ADDR, get_start_power());
	eeprom_write_byte(AUTOSEARCH_MODE_ADDR, g_autosearch_mode);
	eeprom_write_byte(FAULT_INTERRUPTS_MODE_ADDR, g_fault_interrupts_mode);
	
	eeprom_write_byte(STARTBUTTON_MODE_ADDR, g_startbutton_mode);
	
    eeprom_write_byte(VOLTAGE_PWM_BASE_ADDR, get_start_voltage());
    eeprom_write_byte(DEFAULT_VOLTAGE_ADDR, get_default_real_voltage());
}

void reset_settings(void)
{

	g_freq_upper = DDS_MAX_FREQ;
	g_freq_lower = DDS_MIN_FREQ;

	dds_setfreq((g_freq_upper + g_freq_lower) / 2);

	reset_start_bias();
	
	g_int_timeout = DEFAULT_INT_TIMEOUT;
	g_keep_mode = KEEP_CURRENT;

#ifdef _STARTBUTTON_ENABLED
	g_autosearch_mode = AUTOSEARCH_OFF;
#else 
	g_autosearch_mode = AUTOSEARCH_ON;
#endif //_STARTBUTTON_ENABLED

	g_fault_interrupts_mode = FAULT_INTERRUPTS_OFF;
	
	g_keep_freq_step = 1;
	g_keep_freq_max_delta = 1;
	
	g_baudrate = 9600;
	g_modbus_id = 1;
	
	g_temp_alarm[0] = 75;
	g_temp_stop[0] = 80;
	
	g_temp_alarm[1] = 60;
	g_temp_stop[1] = 65;

	set_pfc_mode(DEFAULT_PFC_MODE);
	
    reset_start_power();

#ifdef _STARTBUTTON_ENABLED
	g_startbutton_mode = STARTBUTTON_ON;
#else 
	g_startbutton_mode = STARTBUTTON_OFF;
#endif //_STARTBUTTON_ENABLED
    reset_start_voltage();
    reset_default_real_voltage();
}

void check_settings(void)
{
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

	if (adc[ADC_BIAS_CURRENT].bias > 1000 || adc[ADC_BIAS_CURRENT].bias < 0)
	{
		adc[ADC_BIAS_CURRENT].bias = 511;
		eeprom_write_word(ADC0_BIAS_ADDR, adc[ADC_BIAS_CURRENT].bias);
	}

	if (adc[ADC_FEEDBACK_CURRENT].bias > 520 || adc[ADC_FEEDBACK_CURRENT].bias < 500)
	{
		adc[ADC_FEEDBACK_CURRENT].bias = 511;
		eeprom_write_word(ADC1_BIAS_ADDR, adc[ADC_FEEDBACK_CURRENT].bias);
	}

	if (adc[ADC_AMP].bias > 520 || adc[ADC_AMP].bias < 500)
	{
		adc[ADC_AMP].bias = 511;
		eeprom_write_word(ADC2_BIAS_ADDR, adc[ADC_AMP].bias);
	}
	
	if (g_freq_upper > DDS_MAX_FREQ)
		g_freq_upper = DDS_MAX_FREQ;
		
	if (g_freq_upper < DDS_MIN_FREQ)
		g_freq_upper = DDS_MIN_FREQ;
		
	if (g_freq_lower < DDS_MIN_FREQ)
		g_freq_lower = DDS_MIN_FREQ;
		
	if (g_freq_lower > g_freq_upper)
		g_freq_lower = DDS_MIN_FREQ;

	if (g_dds_freq > g_freq_upper || g_dds_freq < g_freq_lower)
		dds_setfreq((g_freq_upper + g_freq_lower) / 2);

	validate_start_bias();
		
	validate_start_power();
		
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

	if (PFC_OFF > g_pfc_mode || g_pfc_mode >= PFC_COUNT)
		set_pfc_mode(DEFAULT_PFC_MODE);
			
	g_din[DIN_SIZE - 1] = 0;
	
	if (STARTBUTTON_OFF > g_startbutton_mode || g_startbutton_mode >= STARTBUTTON_COUNT)
		g_startbutton_mode = STARTBUTTON_OFF;

#ifndef _STARTBUTTON_ENABLED
	g_startbutton_mode = STARTBUTTON_OFF;
#endif // ! _STARTBUTTON_ENABLED
		
    validate_start_voltage();
    validate_default_real_voltage();
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

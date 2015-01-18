#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "usart.h"
#include "lcd.h"
#include "beep.h"
#include "modbus.h"
#include "timer.h"
#include "kbd.h"
#include "adc.h"
#include "dds.h"
#include "menu.h"
#include "menu_items.h"
#include "current.h"
#include "power.h"
#include "temp.h"
#include "startbutton.h"

fault_interrupts_mode_e	g_fault_interrupts_mode = FAULT_INTERRUPTS_OFF; 
stop_mode_e				g_stop_mode = STOP_BUTTON;
pfc_mode_e				g_pfc_mode;
keep_mode_e				g_keep_mode;
autosearch_mode_e		g_autosearch_mode;
keep_mode_e				g_current_keep_mode = KEEP_OFF;

int16_t		g_keep_current;
int16_t		g_keep_amp;
uint8_t		g_keep_bias_pwm;
int8_t		g_keep_freq_step;
int8_t		g_keep_freq_max_delta;

uint32_t	g_baudrate;
uint8_t		g_modbus_id;

char		g_din[DIN_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

extern uint8_t		g_autosearch_running;
extern uint8_t		g_menu_search_auto_idx;
extern uint16_t	g_int_timeout;

#define DELTA_ZERO_COUNT	(1)

char	buf[50];

void keep_start(void);
void keep_stop(void);
void uzg_run(void);
void uzg_stop(void);
void start(void);
void stop(stop_mode_e);
void loadFromEE(void);
void storeToEE(void);
void do_keep_resonance(void);
void do_keep_amp(void);
void do_usart(void);
void set_pfc_mode(pfc_mode_e mode);
void fault_interrupts_init(fault_interrupts_mode_e mode);

uint8_t		g_bias_alarm = 0;
uint8_t		g_pwm_alarm = 0;

result_e		res;
modbus_cmd_s	cmd;
uint8_t			msg[MODBUS_MAX_MSG_LENGTH];
uint16_t		value;

volatile uint8_t	g_uzg_run;

uint8_t	sendByte;
uint8_t	tmp;

void reset_settings(void);
void check_settings(void);

int main(void)
{
	timer_init();
	beep_init();
	bias_pwm_init();
	power_pwm_init();
	kbd_init();
	startbutton_init();
	lcd_init();
	temp_init();
	dds_init();
	adc_init(ADC_USE_INTERRUPT);

	SETBIT(RUN_PFC_DDR, RUN_PFC_BIT);

	menu_init();
	menu_items_init();

	loadFromEE();
	check_settings();
	
	usart1_init(USART_RS485_SLAVE, g_baudrate);
	usart1_setprotocol_modbus();

	set_power_pwm(g_power_pwm_base);

	stop(STOP_BUTTON);
	sei();

	fault_interrupts_init(g_fault_interrupts_mode);

	beep_ms(500);
	_delay_ms(500);
	beep_ms(200);
	_delay_ms(200);
	beep_ms(200);
	

	for (;;)
	{
		menu_doitem();
		do_keep_resonance();

		if (g_bias_alarm || TEST_FAULT_BIAS)
		{
			sprintf(lcd_line0, "CURRENT OVERLOAD     ");

			beep_ms(200);
			_delay_ms(200);
		}
		else if (g_pwm_alarm || TEST_FAULT_PWM)
		{
			sprintf(lcd_line0, "FREQUENCY OVERLOAD  ");

			beep_ms(200);
			_delay_ms(200);
		}
		else if (g_temp_stop[0] < temp_value(0))
			sprintf(lcd_line0, "OVERHEAT 1          ");
		else if (g_temp_stop[1] < temp_value(1))
			sprintf(lcd_line0, "OVERHEAT 2          ");
		else
			sprintf(lcd_line0, "  UZG-%s PFC-%s  ", 
				(IS_UZG_RUN)?"ON  ":"OFF ", (IS_PFC_RUN)?"ON  ":"OFF ");

		do_startbutton();
		do_lcd();
		do_temp();
		do_usart();
	}
	return 0;
}

void do_usart(void)
{
	int			i;
	uint16_t	value;

	if (!usart1_msg_ready)
		return;
		
	usart1_msg_ready = 0;
	res = modbus_msg2cmd(usart1_inbuf, &cmd);
			
	if (RESULT_OK == res)
		if (g_modbus_id == cmd.device_id)
		{
			if (MODBUS_WRITE == cmd.cmd_code)
			{
				if (!g_autosearch_running)
				{
					value = cmd.value[0];
					switch (cmd.addr)
					{
					case 0x0000:
						switch (value & 0x0003)
						{
						case 0x0001:
							stop(STOP_485);
							break;
						case 0x0002:
							start();
							break;
						default:
							break;
						}
							
						if (value & 0x000C)
							set_pfc_mode((value & 0x000C) >> 2);
						
						switch (value & 0x0030)
						{
						case 0x0010:
							keep_stop();
							break;
						case 0x0020:
							g_keep_mode = KEEP_CURRENT;
							if (IS_UZG_RUN)
								keep_start();
							break;
						case 0x0030:
							g_keep_mode = KEEP_AMP;
							if (IS_UZG_RUN)
								keep_start();
							break;
						default:
							break;
						}
								
						break;
					case 0x0001:	// set freq
						if (value < g_freq_lower)
							value = g_freq_lower;
						if (value > g_freq_upper)
							value = g_freq_upper;
						dds_setfreq((uint32_t)value);
						break;
					case 0x0002:	// set current pwm
						if (value < g_min_bias_pwm)
							value = g_min_bias_pwm;
						if (value > g_max_bias_pwm)
							value = g_max_bias_pwm;
						set_bias_pwm((uint8_t)value);
						break;
					case 0x0003:	// set upper freq
						g_freq_upper = (uint32_t)value;
						if (g_freq_lower > g_freq_upper)
							g_freq_upper = g_freq_lower;
						if (g_freq_supermax < g_freq_upper)
							g_freq_upper = g_freq_supermax;
						if (DDS_MAX_FREQ < g_freq_upper)
							g_freq_upper = DDS_MAX_FREQ;
						if (g_dds_freq > g_freq_upper)
							dds_setfreq(g_freq_upper);
						break;
					case 0x0004:	// set lower freq
						g_freq_lower = (uint32_t)value;
						if (g_freq_lower > g_freq_upper)
							g_freq_lower = g_freq_upper;
						if (g_freq_supermin > g_freq_lower)
							g_freq_lower = g_freq_supermin;
						if (DDS_MIN_FREQ > g_freq_lower)
							g_freq_lower = DDS_MIN_FREQ;
						if (g_freq_lower > g_dds_freq)
							dds_setfreq(g_freq_lower);
						break;
					case 0x0005:	// set current pwm base
						g_bias_pwm_base = (uint8_t)value;
						if (g_max_bias_pwm - g_bias_pwm_shift < g_bias_pwm_base)
							g_bias_pwm_base = g_max_bias_pwm - g_bias_pwm_shift;
						if (g_bias_pwm_base < g_min_bias_pwm)
							g_bias_pwm_base = g_min_bias_pwm;

						normalize_bias_pwm_base();

						break;
#ifdef _BIAS_SHIFT_CHANGEABLE
					case 0x0006:	// set current pwm shift
						g_bias_pwm_shift = (uint8_t)value;
						if (g_max_bias_pwm < g_bias_pwm_shift + g_bias_pwm_base)
							g_bias_pwm_shift = g_max_bias_pwm - g_bias_pwm_base;
						if (g_bias_pwm_shift < 0)
							g_bias_pwm_shift = 0;
						break;
#endif // _BIAS_SHIFT_CHANGEABLE						
#ifdef _MAX_BIAS_CHANGEABLE
					case 0x0007:	// set max current pwm
						g_max_bias_pwm = (uint8_t)value;
						if (g_max_bias_pwm < g_bias_pwm_base + g_bias_pwm_shift)
							g_max_bias_pwm = g_bias_pwm_base + g_bias_pwm_shift;
	
						if (g_max_bias_pwm > 255)
							g_max_bias_pwm = 255;
	
						while (bias_pwm_to_current(g_max_bias_pwm) > g_supermax_bias_pwm / 10.)
							g_max_bias_pwm--;
						
						if ((IS_UZG_RUN) && (g_max_bias_pwm < g_bias_pwm))
							set_bias_pwm(g_max_bias_pwm);
						break;
#endif // _MAX_BIAS_CHANGEABLE
#ifdef _MIN_BIAS_CHANGEABLE
					case 0x0008:	// set min current pwm
						g_min_bias_pwm = (uint8_t)value;
						if (g_bias_pwm_base < g_min_bias_pwm)
							g_min_bias_pwm = g_bias_pwm_base;
						if ((IS_UZG_RUN) && (g_bias_pwm < g_min_bias_pwm))
							set_bias_pwm(g_min_bias_pwm);
						break;
#endif // _MIN_BIAS_CHANGEABLE
					case 0x0009:	// set freq step
						g_keep_freq_step = value;
						if (10 < g_keep_freq_step)
							g_keep_freq_step = 10;
						if (0 > g_keep_freq_step)
							g_keep_freq_step = 0;
						break;
					case 0x000A:	// set freq delta
						g_keep_freq_max_delta = value;
						if (10 < g_keep_freq_max_delta)
							g_keep_freq_max_delta = 10;
						if (0 > g_keep_freq_max_delta)
							g_keep_freq_max_delta = 0;
						break;
					case 0x000B:	// set stop temp
						g_temp_stop[0] = value;
						if (g_temp_stop[0] < g_temp_alarm[0])
							g_temp_alarm[0] = g_temp_stop[0];
						break;
					case 0x000C:	// set alarm temp
						g_temp_alarm[0] = value;
						if (g_temp_stop[0] < g_temp_alarm[0])
							g_temp_alarm[0] = g_temp_stop[0];
						break;

					case 0x100B:	// set stop temp
						g_temp_stop[1] = value;
						if (g_temp_stop[1] < g_temp_alarm[1])
							g_temp_alarm[1] = g_temp_stop[1];
						break;
					case 0x100C:	// set alarm temp
						g_temp_alarm[1] = value;
						if (g_temp_stop[1] < g_temp_alarm[1])
							g_temp_alarm[1] = g_temp_stop[1];
						break;

#ifdef _POWER_CHANGEABLE
					case 0x000D:	// set power pwm
						if (value < g_min_power_pwm)
							value = g_min_power_pwm;
						if (value > g_max_power_pwm)
							value = g_max_power_pwm;
						set_power_pwm((uint8_t)value);
						break;
					case 0x000E:	// set power pwm base
						g_power_pwm_base = (uint8_t)value;
						if (g_max_power_pwm - g_power_pwm_shift < g_power_pwm_base)
							g_power_pwm_base = g_max_power_pwm - g_power_pwm_shift;
						if (g_power_pwm_base < g_min_power_pwm)
							g_power_pwm_base = g_min_power_pwm;
						break;
					case 0x000F:	// set power pwm shift
						g_power_pwm_shift = (uint8_t)value;
						if (g_max_power_pwm - g_power_pwm_shift < g_power_pwm_base)
							g_power_pwm_shift = g_max_power_pwm - g_power_pwm_base;
						if (g_power_pwm_shift < 0)
							g_power_pwm_shift = 0;
						break;
#endif // _POWER_CHANGEABLE
						case 0x0010:
						g_bias_pwm_multiplier = (uint16_t)value;
						if (MIN_BIAS_PWM_MULTIPLIER > g_bias_pwm_multiplier || g_bias_pwm_multiplier > MAX_BIAS_PWM_MULTIPLIER)
							g_bias_pwm_multiplier = DEFAULT_BIAS_PWM_MULTIPLIER;
						eeprom_write_word(BIAS_PWM_MULTIPLIER_ADDR, g_bias_pwm_multiplier);
						break;
#ifdef _SUPERMAX_BIAS_CHANGEABLE
					case 0x0011:
						g_supermax_bias_pwm = (uint8_t)value;
						if (g_supermax_bias_pwm > SUPERMAX_BIAS_PWM)
							g_supermax_bias_pwm = SUPERMAX_BIAS_PWM;
						eeprom_write_byte(SUPERMAX_BIAS_PWM_ADDR, g_supermax_bias_pwm);
						break;
#endif // _SUPERMAX_BIAS_CHANGEABLE
					case 0x0012:
						g_adc_bias_multiplier = (uint8_t)value;
						if (10 > g_adc_bias_multiplier || g_adc_bias_multiplier > 60)
							g_adc_bias_multiplier = 55;
						eeprom_write_byte(ADC_BIAS_MULTIPLIER_ADDR, g_adc_bias_multiplier);
						break;
					case 0x0013:
						g_adc_feedback_multiplier = (uint8_t)value;
						if (20 > g_adc_feedback_multiplier || g_adc_feedback_multiplier > 60)
							g_adc_feedback_multiplier = 30;
						eeprom_write_byte(ADC_FEEDBACK_MULTIPLIER_ADDR, g_adc_feedback_multiplier);
						break;

					case 0x0014:
						g_freq_supermax = (uint32_t)value;
						if (DDS_MAX_FREQ < g_freq_supermax)
							g_freq_supermax = DDS_MAX_FREQ;
						if (g_freq_supermax < g_freq_upper)
							g_freq_supermax = g_freq_upper;
						eeprom_write_word(SUPERMAX_FREQ_ADDR, (uint16_t)g_freq_supermax);
						break;
					case 0x0015:
						g_freq_supermin = (uint32_t)value;
						if (DDS_MIN_FREQ > g_freq_supermin)
							g_freq_supermin = DDS_MIN_FREQ;
						if (g_freq_supermin > g_freq_lower)
							g_freq_supermin = g_freq_lower;
						eeprom_write_word(SUPERMIN_FREQ_ADDR, (uint16_t)g_freq_supermin);
						break;
						
						
					case 0x001C:
						adc[0].bias = (int16_t)value;
						if (adc[0].bias > 0 || adc[0].bias < 1000)
							adc[0].bias = 511;
						eeprom_write_word(ADC0_BIAS_ADDR, adc[0].bias);
						break;
					case 0x001D:
						adc[1].bias = (int16_t)value;
						if (adc[1].bias > 520 || adc[1].bias < 500)
							adc[1].bias = 511;
						eeprom_write_word(ADC1_BIAS_ADDR, adc[1].bias);
						break;
					case 0x001E:
						adc[2].bias = (int16_t)value;
						if (adc[2].bias > 520 || adc[2].bias < 500)
							adc[2].bias = 511;
						eeprom_write_word(ADC2_BIAS_ADDR, adc[2].bias);
						break;
/*					case 0x001F:
						adc[3].bias = (int16_t)value;
						eeprom_write_word(ADC3_BIAS_ADDR, adc[3].bias);
						break;
*/				
					case 0x0020:
						g_din[0] = (char)value;
						eeprom_write_byte(DIN_ADDR + 0, g_din[0]);
						break;
					case 0x0021:
						g_din[1] = (char)value;
						eeprom_write_byte(DIN_ADDR + 1, g_din[1]);
						break;
					case 0x0022:
						g_din[2] = (char)value;
						eeprom_write_byte(DIN_ADDR + 2, g_din[2]);
						break;
					case 0x0023:
						g_din[3] = (char)value;
						eeprom_write_byte(DIN_ADDR + 3, g_din[3]);
						break;
					case 0x0024:
						g_din[4] = (char)value;
						eeprom_write_byte(DIN_ADDR + 4, g_din[4]);
						break;
					case 0x0025:
						g_din[5] = (char)value;
						eeprom_write_byte(DIN_ADDR + 5, g_din[5]);
						break;
					case 0x0026:
						g_din[6] = (char)value;
						eeprom_write_byte(DIN_ADDR + 6, g_din[6]);
						break;
					case 0x0027:
						g_din[7] = (char)value;
						eeprom_write_byte(DIN_ADDR + 7, g_din[7]);
						break;
					case 0x0028:
						g_din[8] = (char)value;
						eeprom_write_byte(DIN_ADDR + 8, g_din[8]);
						break;
					case 0x0029:
						g_din[9] = (char)value;
						eeprom_write_byte(DIN_ADDR + 9, g_din[9]);
						break;
					case 0x002A:
						g_din[10] = (char)value;
						eeprom_write_byte(DIN_ADDR + 10, g_din[10]);
						break;
					case 0x002B:
						g_din[11] = (char)value;
						eeprom_write_byte(DIN_ADDR + 11, g_din[11]);
						break;
					case 0x002C:
						g_din[12] = (char)value;
						eeprom_write_byte(DIN_ADDR + 12, g_din[12]);
						break;
					case 0x002D:
						g_din[13] = (char)value;
						eeprom_write_byte(DIN_ADDR + 13, g_din[13]);
						break;
					case 0x002E:
						g_din[14] = (char)value;
						eeprom_write_byte(DIN_ADDR + 14, g_din[14]);
						break;
					case 0x002F:
						g_din[15] = (char)value;
						eeprom_write_byte(DIN_ADDR + 15, g_din[15]);
						break;

					case 0xFFFF:
						check_settings();
						storeToEE();
						break;
					default:
						break;
					}
				}
				else
				{
					cmd.device_id = g_modbus_id;
					cmd.cmd_code = MODBUS_WRITE;
					cmd.cmd_type = MODBUS_ACK;
					cmd.addr = 0xFFFF;
					cmd.value[0] = 0xFFFF;
				}
				
				modbus_cmd2msg(&cmd, msg, MODBUS_MAX_MSG_LENGTH);
				usart1_cmd(msg, 0, 0, 300);
			}
			else if (MODBUS_READ == cmd.cmd_code)
			{
				cmd.device_id = g_modbus_id;
				cmd.cmd_code = MODBUS_READ;
				cmd.cmd_type = MODBUS_ACK;

				if (0x0000 == cmd.addr)
				{
					cmd.addr = 9;
				
					cmd.value[0] = (IS_UZG_RUN)?2:1;
					cmd.value[0] |= (uint16_t)g_pfc_mode << 2;
					cmd.value[0] |= (uint16_t)g_current_keep_mode << 4;
					cmd.value[0] |= (uint16_t)g_autosearch_running << 6;

					cmd.value[1] = (uint16_t)g_dds_freq;
					cmd.value[2] = (uint16_t)g_bias_pwm;
					cmd.value[3] = (uint16_t)g_power_pwm;
					cmd.value[4] = (int16_t)(((temp_value(0)>temp_value(1))?temp_value(0):temp_value(1)) * 10);
					cmd.value[5] = (uint16_t)adc_mean_value(ADC_BIAS_CURRENT);
					cmd.value[6] = (uint16_t)adc_mean_value(ADC_FEEDBACK_CURRENT);
					cmd.value[7] = (uint16_t)adc_mean_value(ADC_AMP); // feedback coil
					cmd.value[8] = (uint16_t)g_stop_mode;
				}
				else if (0x0001 == cmd.addr)
				{
					cmd.addr = 12;
					
					cmd.value[0] = (uint16_t)g_freq_upper;
					cmd.value[1] = (uint16_t)g_freq_lower;
					cmd.value[2] = (uint16_t)g_bias_pwm_base;
					cmd.value[3] = (uint16_t)g_bias_pwm_shift;
					cmd.value[4] = (uint16_t)g_max_bias_pwm;
					cmd.value[5] = (uint16_t)g_min_bias_pwm;
					cmd.value[6] = (uint16_t)g_keep_freq_step;
					cmd.value[7] = (uint16_t)g_keep_freq_max_delta;
					cmd.value[8] = (uint16_t)g_temp_stop;
					cmd.value[9] = (uint16_t)g_temp_alarm;
					cmd.value[10] = (uint16_t)g_power_pwm_base;
					cmd.value[11] = (uint16_t)g_power_pwm_shift;
				}
				else if (0x0002 == cmd.addr)
				{
					cmd.addr = 6;
					
					cmd.value[0] = temp_value(0) * 10;
					cmd.value[1] = temp_value(1) * 10;
					cmd.value[2] = (uint16_t)g_temp_stop[0];
					cmd.value[3] = (uint16_t)g_temp_alarm[0];
					cmd.value[4] = (uint16_t)g_temp_stop[1];
					cmd.value[5] = (uint16_t)g_temp_alarm[1];
				}
				else if (0x0010 == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)g_bias_pwm_multiplier;
				}
				else if (0x0011 == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)g_supermax_bias_pwm;
				}
				else if (0x0012 == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)g_adc_bias_multiplier;
				}
				else if (0x0013 == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)g_adc_feedback_multiplier;
				}
				else if (0x0014 == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)g_freq_supermax;
				}
				else if (0x0015 == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)g_freq_supermin;
				}
				else if (0x001C == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)(adc[0].bias);
				}
				else if (0x001D == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)(adc[1].bias);
				}
				else if (0x001E == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)(adc[2].bias);
				}
/*				else if (0x001F == cmd.addr)
				{
					cmd.addr = 1;
					
					cmd.value[0] = (uint16_t)(adc[3].bias);
				}
*/				else if (0x0020 == cmd.addr)
				{
					cmd.addr = DIN_SIZE;
					
					for (i = 0; i < DIN_SIZE; i++)
						cmd.value[i] = (uint16_t)g_din[i];
					
				}
				
				modbus_cmd2msg(&cmd, msg, MODBUS_MAX_MSG_LENGTH);
				usart1_cmd(msg, 0, 0, 300);
			}
		}
}

void do_timer(void)
{
	do_kbd();
	//do_startbutton();
}

void uzg_run(void)
{
	dds_setfreq(g_dds_freq);
	set_power_on();
	set_power_pwm(g_power_pwm_base);
	
#ifdef _BIAS_CHANGEABLE
	set_bias_pwm(g_bias_pwm_base);
#endif // _BIAS_CHANGEABLE
}

void uzg_stop(void)
{
	set_bias_off();
	set_power_off();
}

void start(void)
{
    fault_interrupts_init(g_fault_interrupts_mode);

	if (TEST_FAULT_PWM || g_pwm_alarm)
	{
		sprintf(lcd_line0, "FREQUENCY OVERLOAD  ");
		do_lcd();
		beep_ms(200);
		_delay_ms(1000);
		return;
	}
		
	if (TEST_FAULT_BIAS || g_bias_alarm)
	{
		sprintf(lcd_line0, "CURRENT OVERLOAD    ");
		do_lcd();
		beep_ms(200);
		_delay_ms(1000);
		return;
	}
	
	if (g_temp_alarm[0] < temp_value(0))
	{
		sprintf(lcd_line0, "OVERHEAT 1          ");
		do_lcd();
		beep_ms(200);
		_delay_ms(1000);
		return;
	}
	
	if (g_temp_alarm[1] < temp_value(1))
	{
		sprintf(lcd_line0, "OVERHEAT 2          ");
		do_lcd();
		beep_ms(200);
		_delay_ms(1000);
		return;
	}

	if (PFC_OFF != g_pfc_mode)
	{
		PFC_RUN;
		_delay_ms(10);
	}
	
	uzg_run();

	if (AUTOSEARCH_ON == g_autosearch_mode)
	{
		g_menu_mode = MENU_MODE_WORK;
		g_menu_item[MENU_MODE_WORK] = g_menu_search_auto_idx;
		SET_KEY_PRESSED(KEY_ENTER);
	}
}

void stop(stop_mode_e mode)
{
	keep_stop();
	uzg_stop();
	
	if (PFC_ON != g_pfc_mode)
	{
		_delay_ms(10);
		PFC_STOP;
	}
	
	if (STOP_NOT_CHANGE != mode)
		g_stop_mode = mode;

    fault_interrupts_init(g_fault_interrupts_mode);
    
    if (!TEST_FAULT_BIAS)
        g_bias_alarm = 0;

    if (!TEST_FAULT_PWM)
        g_pwm_alarm = 0;
}


void set_pfc_mode(pfc_mode_e mode)
{
	g_pfc_mode = mode;
	
	switch (g_pfc_mode)
	{
	case PFC_OFF:
		PFC_STOP;
		break;
	case PFC_ON:
		PFC_RUN;
		break;
	case PFC_AUTO:
		if (IS_UZG_RUN)
			PFC_RUN;
		else
			PFC_STOP;
	}
}

void do_keep_resonance(void)
{
	static int8_t		delta;
	static int8_t		max_delta;
	static int8_t		step;
	static uint8_t		timer_id;
	static uint32_t	old_freq;
	static int32_t		integral;
	static int32_t		sum_value;
	static	uint8_t		looking_up_freq;
	static	uint8_t		looking_up_bias_pwm;
	static uint8_t		delta_zero_count;
	
	int16_t				amp;
	int16_t				current;
	int16_t				new_value;
	int16_t				keep_value;
	
	if (g_current_keep_mode == KEEP_OFF)
	{
		if (0 != timer_id)
		{
			stop_timer(timer_id);
			timer_id = 0;
		}

		looking_up_freq = 0;
		looking_up_bias_pwm = 0;
		return;
	}
	
	cli();
	current = adc_mean_value(ADC_FEEDBACK_CURRENT);
	amp = adc_mean_value(ADC_AMP);
	sei();
	
	if ((!looking_up_freq) && (!looking_up_bias_pwm))
	{
		if (0 != timer_id)
		{
			stop_timer(timer_id);
			timer_id = 0;
		}

		if ((current == g_keep_current) && ((amp == g_keep_amp) || (KEEP_CURRENT == g_current_keep_mode)))
			return;
			
		if (current < g_keep_current)
		{
			looking_up_freq = 1;
			integral = 0;
			sum_value = 0;
			old_freq = g_dds_freq;
			max_delta = g_keep_freq_max_delta;
			step = g_keep_freq_step;
			delta = -max_delta;
	
			dds_setfreq(old_freq + delta * step);
		}
		else
		{
			looking_up_bias_pwm = 1;
		}
		
		timer_id = start_timer(g_int_timeout + adc_get_timeout(ADC_FEEDBACK_CURRENT));
		beep_ms(100);
	}
//	else if (0 == timer_value(timer_id))
	else if ((0 == timer_value(timer_id)) || (0 == timer_id))
	{
		stop_timer(timer_id);
		timer_id = 0;
		
		if (looking_up_freq)
		{
			integral += current * delta * step;
			sum_value += current;
		
			delta++;
		
			if (delta > max_delta)
			{
				if (integral > step * max_delta)
					integral = step * max_delta;
				else if (integral < - step * max_delta)
					integral = - step * max_delta;
					
				delta = integral;

/*
				if ((0 < integral) && (integral < sum_value))
					delta = 1;
				else if ((-sum_value < integral) && (integral < 0))
					delta = -1;
				else if (0 == sum_value)
					delta = 0;
				else
					delta = integral / sum_value;
*/			
				dds_setfreq(old_freq + delta);
				looking_up_freq = 0;
				
				if (0 != delta)
					delta_zero_count = 0;
				else
					delta_zero_count++;
					
				if (DELTA_ZERO_COUNT < delta_zero_count)
				{
					looking_up_bias_pwm = 1;
					delta_zero_count = 0;
				}
			}
			else
			{
				dds_setfreq(old_freq + delta * step);
				timer_id = start_timer(g_int_timeout + adc_get_timeout(ADC_FEEDBACK_CURRENT));
			}
		}
		
		if (looking_up_bias_pwm)
		{
			if (KEEP_CURRENT == g_current_keep_mode)
			{
				new_value = current;
				keep_value = g_keep_current;
			}
			else
			{
				new_value = amp;
				keep_value = g_keep_amp;
			}
		
			if ((new_value < keep_value) && (g_bias_pwm < g_max_bias_pwm))
			{
				set_bias_pwm(g_bias_pwm + 1);
				timer_id = start_timer(g_int_timeout + adc_get_timeout(ADC_FEEDBACK_CURRENT) + adc_get_timeout(ADC_AMP));
			}
			else if ((new_value > keep_value) && (g_bias_pwm > g_min_bias_pwm))
			{
				set_bias_pwm(g_bias_pwm - 1);
				timer_id = start_timer(g_int_timeout + adc_get_timeout(ADC_FEEDBACK_CURRENT) + adc_get_timeout(ADC_AMP));
			}
			else
			{
				looking_up_bias_pwm = 0;
				g_keep_current = current;
			}
		}
	}
}


void keep_stop(void)
{
	g_current_keep_mode = KEEP_OFF;
}

void keep_start(void)
{
	_delay_ms(1000);
	delay_ms(g_int_timeout + adc_get_timeout(ADC_FEEDBACK_CURRENT) + adc_get_timeout(ADC_AMP));
	g_current_keep_mode = g_keep_mode;
	cli();
	g_keep_current = adc_mean_value(ADC_FEEDBACK_CURRENT);
	g_keep_amp = adc_mean_value(ADC_AMP);
	sei();
	g_keep_bias_pwm = g_bias_pwm;
	sprintf(lcd_line1,"C:%-8.1fA:%-10d", adc_feedback_to_current(g_keep_current), g_keep_amp);
	do_lcd();
	_delay_ms(500);
}

void fault_interrupts_init(fault_interrupts_mode_e mode)
{
	CLEARBIT(DDRE, PE5);
	CLEARBIT(EICRB, ISC50);
	CLEARBIT(EICRB, ISC51);
	
	CLEARBIT(DDRE, PE7);
	CLEARBIT(EICRB, ISC70);
	CLEARBIT(EICRB, ISC71);

	if (FAULT_INTERRUPTS_ON == mode)
	{
		SETBIT(EIMSK, INT5);
		SETBIT(EIMSK, INT7);
	}
	else
	{
		CLEARBIT(EIMSK, INT5);
		CLEARBIT(EIMSK, INT7);
	}
	
	g_fault_interrupts_mode = mode;
}

//FLTPWM
ISR(INT5_vect)
{
	if (IS_UZG_RUN)
		stop(STOP_FAULT_PWM);

	g_pwm_alarm = 1;
	CLEARBIT(EIMSK, INT5);
}

//FLTBIAS
ISR(INT7_vect)
{
	if (IS_UZG_RUN)
		stop(STOP_FAULT_BIAS);

	g_bias_alarm = 1;
	CLEARBIT(EIMSK, INT7);
}

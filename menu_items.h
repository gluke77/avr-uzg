#if !defined _MENU_ITEMS_INCLUDED
#define _MENU_ITEMS_INCLUDED

#include "common.h"
#include "menu.h"

void menu_items_init(void);

void menu_common(void);
void menu_freq(void);
void menu_freq_step(void);
void menu_current(void);
void menu_power(void);
void menu_voltage(void);

void menu_adc0(void);
void menu_adc1(void);
void menu_adc2(void);
void menu_adc3(void);

void menu_search(void);
void menu_search_auto(void);
void menu_amp(void);
void menu_temp(void);
void menu_temp2(void);

void menu_freq_upper(void);
void menu_freq_lower(void);

void menu_adc0_count(void);
void menu_adc0_delay(void);
void menu_adc1_count(void);
void menu_adc1_delay(void);
void menu_adc2_count(void);
void menu_adc2_delay(void);
void menu_adc3_count(void);
void menu_adc3_delay(void);
//void menu_adc_multiplier(void);

void menu_pfc_mode(void);

void menu_store_settings(void);
void menu_reset_settings(void);

void menu_start_bias(void);
void menu_start_power(void);
void menu_start_voltage(void);

void menu_int_timeout(void);

void menu_keep_mode(void);
void menu_keep_step(void);
void menu_keep_delta(void);

void menu_autosearch_mode(void);

void menu_modbus_id(void);
void menu_baudrate(void);

void menu_temp_alarm(void);
void menu_temp_stop(void);
void menu_temp2_alarm(void);
void menu_temp2_stop(void);
void menu_fault_interrupts(void);
void menu_stop_mode(void);

void menu_version(void);
void menu_din(void);

void menu_startbutton(void);

#endif /* _MENU_ITEMS_INCLUDED */

#if !defined _VOLTAGE_INCLUDED
#define _VOLTAGE_INCLUDED

#include "common.h"

#define MAX_VOLTAGE_PWM	    (150)
#define MIN_VOLTAGE_PWM	    (110)   
#define DEFAULT_VOLTAGE_PWM (120)
#define VOLTAGE_BIAS        (20)

extern uint8_t  g_voltage_pwm;

void voltage_pwm_init(void);

void set_voltage_on(void);
void set_voltage_off(void);
uint8_t is_voltage_on(void);

uint8_t get_voltage_pwm(void);
void set_voltage_pwm(uint8_t);
void inc_voltage_pwm(void);
void dec_voltage_pwm(void);

uint8_t get_start_voltage(void);
void set_start_voltage(uint8_t);
void inc_start_voltage(void);
void dec_start_voltage(void);
void validate_start_voltage(void);
void reset_start_voltage(void);

#endif /* _VOLTAGE_INCLUDED */

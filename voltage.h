#if !defined _VOLTAGE_INCLUDED
#define _VOLTAGE_INCLUDED

#include "common.h"

extern uint8_t		g_voltage_pwm;
extern uint8_t		g_voltage_pwm_base;
extern uint8_t		g_voltage_pwm_step;
extern uint8_t		g_max_voltage_pwm;
extern uint8_t		g_min_voltage_pwm;

void voltage_pwm_init(void);
void set_voltage_pwm(uint8_t);

void set_voltage_on(void);
void set_voltage_off(void);
uint8_t is_voltage_on(void);


#endif /* _VOLTAGE_INCLUDED */

#if !defined _POWER_INCLUDED
#define _POWER_INCLUDED

#include "common.h"

extern uint8_t		g_power_pwm;
extern uint8_t		g_power_pwm_step;
extern uint8_t		g_power_pwm_base;
extern uint8_t		g_power_pwm_shift;
extern uint8_t		g_max_power_pwm;
extern uint8_t		g_min_power_pwm;

void power_pwm_init(void);
void set_power_pwm(uint8_t);

void set_power_on(void);
void set_power_off(void);
uint8_t is_power_on(void);

#define POWER_PWM_MAX	(99)
#define POWER_PWM_MIN	(29)

#endif /* _POWER_INCLUDED */

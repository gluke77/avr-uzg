#if !defined _POWER_INCLUDED
#define _POWER_INCLUDED

#include "common.h"

#define MAX_POWER_PWM	    (9)
#define MIN_POWER_PWM	    (6)   
#define DEFAULT_POWER_PWM   (8)
#define POWER_BIAS          (-1)

void power_pwm_init(void);

void set_power_on(void);
void set_power_off(void);
uint8_t is_power_on(void);

uint8_t get_power_pwm(void);
void set_power_pwm(uint8_t);
void inc_power_pwm(void);
void dec_power_pwm(void);

uint8_t get_start_power(void);
void set_start_power(uint8_t);
void inc_start_power(void);
void dec_start_power(void);
void validate_start_power(void);
void reset_start_power(void);

#endif /* _POWER_INCLUDED */

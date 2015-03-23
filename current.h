#if !defined _CURRENT_INCLUDED
#define _CURRENT_INCLUDED

#include "common.h"

#define MAX_BIAS_PWM    (110)
#define MIN_BIAS_PWM    (50)

#define MAX_START_BIAS (MAX_BIAS_PWM)
#define MIN_START_BIAS (MIN_BIAS_PWM)
#define DEFAULT_BIAS (100)

extern uint8_t		g_bias_pwm;
extern uint16_t		g_bias_pwm_multiplier;
extern uint8_t		g_adc_bias_multiplier;

extern uint8_t      g_start_bias;

void bias_pwm_init(void);
void setup_bias();
void stop_bias(void);
double bias_pwm_to_current(uint8_t);

void inc_bias_pwm(void);
void dec_bias_pwm(void);

uint8_t get_start_bias(void);
void inc_start_bias(void);
void dec_start_bias(void);
void set_start_bias(uint8_t);
void validate_start_bias(void);

double get_bias_adc(void);

#endif /* _CURRENT_INCLUDED */

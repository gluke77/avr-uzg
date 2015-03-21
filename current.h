#if !defined _CURRENT_INCLUDED
#define _CURRENT_INCLUDED

#include "common.h"

#define MAX_BIAS_PWM    (200)
#define MIN_BIAS_PWM    (0)

#define MAX_WANTED_BIAS (50)
#define MIN_WANTED_BIAS (0)
#define DEFAULT_BIAS ((MAX_WANTED_BIAS + MIN_WANTED_BIAS) / 2)

extern uint8_t		g_bias_pwm;
extern uint16_t		g_bias_pwm_multiplier;

extern uint8_t      g_wanted_bias;

extern uint8_t      g_adc_bias_multiplier;

void bias_pwm_init(void);
void setup_bias();
void stop_bias(void);
double bias_pwm_to_current(uint8_t);

void inc_bias_pwm(void);
void dec_bias_pwm(void);

double get_wanted_bias(void);
void inc_wanted_bias(void);
void dec_wanted_bias(void);
void set_wanted_bias(uint8_t);
void validate_wanted_bias(void);

double get_bias_adc(void);

#endif /* _CURRENT_INCLUDED */

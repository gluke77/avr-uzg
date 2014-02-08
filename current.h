#if !defined _CURRENT_INCLUDED
#define _CURRENT_INCLUDED

#include "common.h"

// moved to common.h #define SUPERMAX_BIAS_PWM	(70)
// moved to common.h #define MAX_BIAS_PWM_BASE	(4.)

extern uint8_t		g_bias_pwm;
extern uint8_t		g_bias_pwm_step;
extern uint8_t		g_bias_pwm_base;
extern uint8_t		g_bias_pwm_shift;
extern uint8_t		g_max_bias_pwm;
extern uint8_t		g_min_bias_pwm;
extern uint16_t		g_bias_pwm_multiplier;
extern uint8_t		g_supermax_bias_pwm;

void bias_pwm_init(void);
void set_bias_pwm(uint8_t);
void set_bias_off(void);
void normalize_bias_pwm_base(void);
float bias_pwm_to_current(uint8_t);

#endif /* _CURRENT_INCLUDED */
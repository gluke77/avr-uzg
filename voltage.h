#if !defined _VOLTAGE_INCLUDED
#define _VOLTAGE_INCLUDED

#include "common.h"

#define MAX_VOLTAGE	    (130)
#define MIN_VOLTAGE	    (90)   
#define DEFAULT_VOLTAGE (100)

#define MAX_REAL_VOLTAGE	    (150)
#define MIN_REAL_VOLTAGE        (50)   
#define DEFAULT_REAL_VOLTAGE    (130)

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

void set_default_real_voltage(uint8_t);
uint8_t get_default_real_voltage(void);
void validate_default_real_voltage(void);
void reset_default_real_voltage(void);

#endif /* _VOLTAGE_INCLUDED */

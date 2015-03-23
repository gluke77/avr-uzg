#include <avr/io.h>
#include <avr/interrupt.h>
#include "voltage.h"
#include "common.h"

uint8_t		g_voltage_pwm = VOLTAGE_BIAS;
uint8_t		g_start_voltage = DEFAULT_VOLTAGE_PWM;

void set_voltage_on(void)
{
    set_voltage_pwm(get_start_voltage());
	SETBIT(TCCR2, CS21);
}

void set_voltage_off(void)
{
    // Disable clock source
	CLEARBIT(TCCR2, CS21);
    OCR2 = 0;
    g_voltage_pwm = VOLTAGE_BIAS;
}

uint8_t is_voltage_on(void)
{
	return TESTBIT(TCCR2, CS21);
}

void voltage_pwm_init(void)
{
	TCCR2 = 0x00;
	
	// Operation mode: 3 Fast PWM top=OCR2
	SETBIT(TCCR2, WGM20);
	SETBIT(TCCR2, WGM21);

	// OC2 output mode 2: Clear on Compare Match, set on BOTTOM
	SETBIT(TCCR2, COM21);
	CLEARBIT(TCCR2, COM20);
	
	// Clock source: CLK_IO, no prescaling
	SETBIT(TCCR2, CS20);
	CLEARBIT(TCCR2, CS21);
	CLEARBIT(TCCR2, CS22);

    set_voltage_off();
	
	// PB7/OC2 output
    SETBIT(DDRB, PB7);
}

void set_voltage_pwm(uint8_t byte)
{
	if (byte > MAX_VOLTAGE_PWM)
        byte = MAX_VOLTAGE_PWM;

    if (byte < MIN_VOLTAGE_PWM)
        byte = MIN_VOLTAGE_PWM;
	
    byte += VOLTAGE_BIAS;
    
    OCR2 = byte;
	g_voltage_pwm = byte;
}

uint8_t get_voltage_pwm()
{
    return g_voltage_pwm - VOLTAGE_BIAS;
}

void inc_voltage_pwm()
{
    uint8_t pwm = get_voltage_pwm();
    if (pwm < MAX_VOLTAGE_PWM)
    {
        pwm++;
        set_voltage_pwm(pwm);
    }
}

void dec_voltage_pwm()
{
    uint8_t pwm = get_voltage_pwm();
    if (pwm > MIN_VOLTAGE_PWM)
    {
        pwm--;
        set_voltage_pwm(pwm);
    }
}

uint8_t get_start_voltage()
{
    return g_start_voltage;
}

void set_start_voltage(uint8_t byte)
{
	if (byte > MAX_VOLTAGE_PWM)
        byte = MAX_VOLTAGE_PWM;

    if (byte < MIN_VOLTAGE_PWM)
        byte = MIN_VOLTAGE_PWM;
	
	g_start_voltage = byte;
}

void inc_start_voltage()
{
    if (g_start_voltage < MAX_VOLTAGE_PWM)
        g_start_voltage++;
}

void dec_start_voltage()
{
    if (g_start_voltage > MIN_VOLTAGE_PWM)
        g_start_voltage--;
}

void validate_start_voltage()
{
	if (g_start_voltage > MAX_VOLTAGE_PWM)
        g_start_voltage = MAX_VOLTAGE_PWM;

    if (g_start_voltage < MIN_VOLTAGE_PWM)
        g_start_voltage = MIN_VOLTAGE_PWM;
}

void reset_start_voltage()
{
    g_start_voltage = DEFAULT_VOLTAGE_PWM;
}

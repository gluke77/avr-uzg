#include <avr/io.h>
#include <avr/interrupt.h>
#include "voltage.h"
#include "common.h"

uint8_t		g_voltage_pwm = DEFAULT_VOLTAGE_PWM;
uint8_t		g_voltage_pwm_base = DEFAULT_VOLTAGE_PWM;
uint8_t		g_voltage_pwm_step = VOLTAGE_PWM_STEP;
uint8_t		g_max_voltage_pwm = VOLTAGE_PWM_MAX;
uint8_t		g_min_voltage_pwm = VOLTAGE_PWM_MIN;

void set_voltage_on(void)
{
    set_voltage_pwm(g_voltage_pwm_base);
	SETBIT(TCCR2, CS20);
}

void set_voltage_off(void)
{
    // Disable clock source
	CLEARBIT(TCCR2, CS20);
    set_voltage_pwm(0);
}

uint8_t is_voltage_on(void)
{
	return TESTBIT(TCCR2, CS20);
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

	OCR2 = g_voltage_pwm_base;

    set_voltage_off();
	
	// PB7/OC2 output
    SETBIT(DDRB, PB7);
}

void set_voltage_pwm(uint8_t byte)
{
	if (g_max_voltage_pwm < byte)
		byte = g_max_voltage_pwm;
		
	if (byte < g_min_voltage_pwm)
		byte = g_min_voltage_pwm;
		
	OCR2 = byte;
	g_voltage_pwm = byte;
}

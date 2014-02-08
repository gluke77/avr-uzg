#include <avr/io.h>
#include <avr/interrupt.h>
#include "current.h"

uint8_t		g_bias_pwm;
uint8_t		g_bias_pwm_step = 1;
uint8_t		g_bias_pwm_base;
uint8_t		g_bias_pwm_shift;
uint8_t		g_max_bias_pwm;
uint8_t		g_min_bias_pwm;

uint16_t	g_bias_pwm_multiplier = 600;
uint8_t		g_supermax_bias_pwm = SUPERMAX_BIAS_PWM;

void bias_pwm_init(void)
{
	TCCR1A = 0x00;
	TCCR1B = 0x00;
	TCCR1C = 0x00;
	
	// Mode: 15 Fast PWM top=OCR1A
	SETBIT(TCCR1A, WGM10);
	SETBIT(TCCR1A, WGM11);
	SETBIT(TCCR1B, WGM12);
	SETBIT(TCCR1B, WGM13);
	
	// OC1A
	SETBIT(DDRB, PB5);

	// OC1A output: Toggle on Compare Match
	SETBIT(TCCR1A, COM1A0);
	
	// OC1B
	SETBIT(DDRB, PB6);

	// OC1B output: Inverted PWM
	SETBIT(TCCR1A, COM1B0);
	SETBIT(TCCR1A, COM1B1);
	
	
	// OC1C output: Disconnected

//	// Clear OC1B on compare match when upcounting
//	// Set OC1B on compare match when downcounting
//	SETBIT(TCCR1A, COM1B1);
	

	// Clock source: System clock
	// Clock value: System clock
	// No prescaling
	SETBIT(TCCR1B, CS10);
	
	cli();
	OCR1A = 255;
	OCR1B = 0;
	sei();
}

void set_bias_pwm(uint8_t byte)
{
	if (g_max_bias_pwm < byte)
		byte = g_max_bias_pwm;
		
	if ((0 != byte) && (byte < g_min_bias_pwm))
		byte = g_min_bias_pwm;

#ifdef _BIAS_CHANGEABLE
	cli();
	OCR1B = (uint16_t)byte;
	sei();
#endif _BIAS_CHANGEABLE
	
	g_bias_pwm = byte;
}

void set_bias_off(void)
{
	OCR1B = 0;
	g_bias_pwm = 0;
}

float bias_pwm_to_current(uint8_t pwm)
{
	return pwm * (g_bias_pwm_multiplier / 100.) / 255;
}
/*
void set_supermax_bias_pwm(uint8_t supermax)
{
	g_supermax_bias_pwm = supermax;
	
	while (bias_pwm_to_current(g_max_bias_pwm) > g_supermax_bias_pwm / 10.)
		g_max_bias_pwm--;
	
	if (g_min_bias_pwm > g_max_bias_pwm)
		g_min_bias_pwm = g_max_bias_pwm;
		
	if (g_bias_pwm_base > g_max_bias_pwm)
		g_bias_pwm_base = g_max_bias_pwm;
		
	if (g_bias_pwm_shift > g_max_bias_pwm - g_bias_pwm_base)
		g_bias_pwm_shift = g_max_bias_pwm - g_bias_pwm_base;
}
*/

void normalize_bias_pwm_base(void)
{
	while (bias_pwm_to_current(g_bias_pwm_base) > MAX_BIAS_PWM_BASE)
		g_bias_pwm_base--;
}

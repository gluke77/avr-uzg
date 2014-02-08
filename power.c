#include <avr/io.h>
#include <avr/interrupt.h>
#include "power.h"
#include "common.h"

uint8_t		g_power_pwm;
uint8_t		g_power_pwm_step = 1;
uint8_t		g_power_pwm_base;
uint8_t		g_power_pwm_shift;
uint8_t		g_max_power_pwm = POWER_PWM_MAX;
uint8_t		g_min_power_pwm = POWER_PWM_MIN;

void set_power_on(void)
{
	TCCR3A |= _BV(COM3B0) | _BV(COM3B1);
}

void set_power_off(void)
{
	TCCR3A &= ~(_BV(COM3B0) | _BV(COM3B1));
	SETBIT(PORTE, PE4);
}

uint8_t is_power_on(void)
{
	return TESTBIT(TCCR3A, COM3B0) | TESTBIT(TCCR3A, COM3B1);
}

void power_pwm_init(void)
{
	TCCR3A = 0x00;
	TCCR3B = 0x00;
	TCCR3C = 0x00;
	
	// Mode: 15 Fast PWM top=OCR1A
	SETBIT(TCCR3A, WGM30);
	SETBIT(TCCR3A, WGM31);
	SETBIT(TCCR3B, WGM32);
	SETBIT(TCCR3B, WGM33);
	
	// OC3A
	SETBIT(DDRE, PE3);

	// OC3A output: Toggle on Compare Match
	SETBIT(TCCR3A, COM3A0);
	
	// OC3B
	SETBIT(DDRE, PE4);

	// OC3B output: Inverted PWM
//	SETBIT(TCCR3A, COM3B0);
//	SETBIT(TCCR3A, COM3B1);	// moved to set_power_on()
	
	SETBIT(PORTE, PE4); // power off
	
	// OC3C output: Disconnected


	// Clock source: T3 input rising edge
	SETBIT(TCCR3B, CS30);
	SETBIT(TCCR3B, CS31);
	SETBIT(TCCR3B, CS32);
	
	CLEARBIT(DDRE, PE6);
	
	cli();
	OCR3A = 99;
	OCR3B = 0;
	sei();
}

void set_power_pwm(uint8_t byte)
{
	if (g_max_power_pwm < byte)
		byte = g_max_power_pwm;
		
	if (byte < g_min_power_pwm)
		byte = g_min_power_pwm;
		
	cli();
	OCR3B = (uint16_t)byte;
	sei();
	g_power_pwm = byte;
}
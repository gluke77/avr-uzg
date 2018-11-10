#include <avr/io.h>
#include <avr/interrupt.h>
#include "power.h"
#include "common.h"

uint8_t		g_power_pwm;
uint8_t		g_start_power;

void set_power_on(void)
{
    set_power_pwm(get_start_power());
	TCCR3A |= _BV(COM3B0) | _BV(COM3B1);
}

void set_power_off(void)
{
	TCCR3A &= ~(_BV(COM3B0) | _BV(COM3B1));
	SETBIT(PORTE, PE4);
	cli();
	OCR3B = (uint16_t)0;
	sei();
    g_power_pwm = POWER_BIAS;
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
	OCR3A = 9;
	OCR3B = 0;
	sei();
}

void set_power_pwm(uint8_t byte)
{
	if (byte > MAX_POWER_PWM)
        byte = MAX_POWER_PWM;

    if (byte < MIN_POWER_PWM)
        byte = MIN_POWER_PWM;
		
    byte += POWER_BIAS;

	cli();
	OCR3B = (uint16_t)byte;
	sei();
	g_power_pwm = byte;
}

uint8_t get_power_pwm()
{
    return g_power_pwm - POWER_BIAS;
}

void inc_power_pwm()
{
    uint8_t pwm = get_power_pwm();
    if ((pwm < MAX_POWER_PWM) && IS_UZG_RUN)
    {
        pwm++;
        set_power_pwm(pwm);
    }
}

void dec_power_pwm()
{
    uint8_t pwm = get_power_pwm();
    if ((pwm > MIN_POWER_PWM) && IS_UZG_RUN)
    {
        pwm--;
        set_power_pwm(pwm);
    }
}

uint8_t get_start_power()
{
    return g_start_power;
}

void set_start_power(uint8_t byte)
{

	if (byte > MAX_POWER_PWM)
        byte = MAX_POWER_PWM;

    if (byte < MIN_POWER_PWM)
        byte = MIN_POWER_PWM;
	
	g_start_power = byte;
}

void inc_start_power()
{
    if (g_start_power < MAX_POWER_PWM)
        g_start_power++;
}

void dec_start_power()
{
    if (g_start_power > MIN_POWER_PWM)
        g_start_power--;
}

void validate_start_power()
{
	if (g_start_power > MAX_POWER_PWM)
        g_start_power = MAX_POWER_PWM;

    if (g_start_power < MIN_POWER_PWM)
        g_start_power = MIN_POWER_PWM;
}

void reset_start_power()
{
    g_start_power = DEFAULT_POWER_PWM;
}

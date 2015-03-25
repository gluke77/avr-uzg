#include <avr/io.h>
#include <avr/interrupt.h>
#include "voltage.h"
#include "common.h"

uint8_t     _real_voltage = DEFAULT_REAL_VOLTAGE;
uint8_t		g_voltage_pwm = 0;
uint8_t		g_start_voltage = DEFAULT_VOLTAGE;

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
    g_voltage_pwm = 0;
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
	uint8_t tmp;
    
    if (byte > MAX_VOLTAGE)
        byte = MAX_VOLTAGE;

    if (byte < MIN_VOLTAGE)
        byte = MIN_VOLTAGE;
	
    tmp = byte + _real_voltage - DEFAULT_VOLTAGE;
    cli();
    OCR2 = tmp;
    sei();
	g_voltage_pwm = byte;
}

uint8_t get_voltage_pwm()
{
    return g_voltage_pwm;
}

void inc_voltage_pwm()
{
    uint8_t pwm = get_voltage_pwm();
    if (pwm < MAX_VOLTAGE)
    {
        pwm++;
        set_voltage_pwm(pwm);
    }
}

void dec_voltage_pwm()
{
    uint8_t pwm = get_voltage_pwm();
    if (pwm > MIN_VOLTAGE)
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
	if (byte > MAX_VOLTAGE)
        byte = MAX_VOLTAGE;

    if (byte < MIN_VOLTAGE)
        byte = MIN_VOLTAGE;
	
	g_start_voltage = byte;
}

void inc_start_voltage()
{
    if (g_start_voltage < MAX_VOLTAGE)
        g_start_voltage++;
}

void dec_start_voltage()
{
    if (g_start_voltage > MIN_VOLTAGE)
        g_start_voltage--;
}

void validate_start_voltage()
{
	if (g_start_voltage > MAX_VOLTAGE)
        g_start_voltage = MAX_VOLTAGE;

    if (g_start_voltage < MIN_VOLTAGE)
        g_start_voltage = MIN_VOLTAGE;
}

void reset_start_voltage()
{
    g_start_voltage = DEFAULT_VOLTAGE;
}

void set_default_real_voltage(uint8_t byte)
{
    if (byte > MAX_REAL_VOLTAGE)
        byte = MAX_REAL_VOLTAGE;

    if (byte < MIN_REAL_VOLTAGE)
        byte = MIN_REAL_VOLTAGE;

    _real_voltage = byte;
}

uint8_t get_default_real_voltage(void)
{
    return _real_voltage;
}

void validate_default_real_voltage(void)
{
    if (_real_voltage > MAX_REAL_VOLTAGE)
        _real_voltage = MAX_REAL_VOLTAGE;

    if (_real_voltage < MIN_REAL_VOLTAGE)
        _real_voltage = MIN_REAL_VOLTAGE;
}

void reset_default_real_voltage(void)
{
    _real_voltage = DEFAULT_REAL_VOLTAGE;
}

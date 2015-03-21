#include <avr/io.h>
#include <avr/interrupt.h>
#include "current.h"
#include "adc.h"
#include "beep.h"
#include "timer.h"

extern uint16_t g_int_timeout;

uint8_t		g_bias_pwm;
uint16_t	g_bias_pwm_multiplier = 600;
uint8_t     g_wanted_bias = 0;

uint8_t	g_adc_bias_multiplier = 50;

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
	if (MAX_BIAS_PWM < byte)
		byte = MAX_BIAS_PWM;
		
	if ((0 != byte) && (byte < MIN_BIAS_PWM))
		byte = MIN_BIAS_PWM;

	cli();
	OCR1B = (uint16_t)byte;
	sei();
	
	g_bias_pwm = byte;
}

void stop_bias(void)
{
	cli();
    OCR1B = 0;
    sei();
	g_bias_pwm = 0;
}

void inc_bias_pwm()
{
    if (g_bias_pwm < MAX_BIAS_PWM)
    {
        g_bias_pwm++;
        set_bias_pwm(g_bias_pwm);
    }
}

void dec_bias_pwm()
{
    if (g_bias_pwm > MIN_BIAS_PWM)
    {
        g_bias_pwm--;
        if (g_bias_pwm)
            set_bias_pwm(g_bias_pwm);
        else
            stop_bias();
    }
}

double bias_pwm_to_current(uint8_t pwm)
{
	return pwm * (g_bias_pwm_multiplier / 100.) / 255;
}

double get_wanted_bias()
{
    return g_wanted_bias / 10.;
}

void inc_wanted_bias()
{
    if (g_wanted_bias < MAX_WANTED_BIAS)
        g_wanted_bias++;
}

void dec_wanted_bias()
{
    if (g_wanted_bias > MIN_WANTED_BIAS)
        g_wanted_bias--;
}

double adc_value_to_bias(int16_t adc)
{
	return adc * g_adc_bias_multiplier / 1000.;
}

double get_bias_adc()
{
    int16_t adc_value = adc_mean_value(ADC_BIAS_CURRENT);
    double  value = adc_value_to_bias(adc_value);
    if (value < 0.)
        value = 0.;
    return value;
}

void validate_wanted_bias()
{
    if (g_wanted_bias > MAX_WANTED_BIAS)
        g_wanted_bias = MAX_WANTED_BIAS;

    if (g_wanted_bias < MIN_WANTED_BIAS)
        g_wanted_bias = MIN_WANTED_BIAS;
}

void set_wanted_bias(uint8_t wanted_bias)
{
    if (wanted_bias > MAX_WANTED_BIAS)
        wanted_bias = MAX_WANTED_BIAS;

    if (wanted_bias < MIN_WANTED_BIAS)
        wanted_bias = MIN_WANTED_BIAS;

    g_wanted_bias = wanted_bias;
}

void setup_bias()
{

	uint16_t timeout = adc_get_timeout(ADC_BIAS_CURRENT) + g_int_timeout;
    while (get_wanted_bias() > get_bias_adc() && g_bias_pwm < MAX_BIAS_PWM)
    {
        inc_bias_pwm();
        delay_ms(timeout);
    }
}


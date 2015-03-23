#include <avr/io.h>
#include <avr/interrupt.h>
#include "current.h"
#include "adc.h"
#include "beep.h"
#include "timer.h"

extern uint16_t g_int_timeout;

uint8_t		g_bias_pwm;
uint16_t	g_bias_pwm_multiplier = 600;
uint8_t     g_start_bias = 0;

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

uint8_t get_bias_pwm()
{
    return g_bias_pwm;
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

void setup_bias()
{
    set_bias_pwm(get_start_bias());
}

void inc_bias_pwm()
{
    if ((g_bias_pwm < MAX_BIAS_PWM) && IS_UZG_RUN)
    {
        g_bias_pwm++;
        set_bias_pwm(g_bias_pwm);
    }
}

void dec_bias_pwm()
{
    if ((g_bias_pwm > MIN_BIAS_PWM) && IS_UZG_RUN)
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


uint8_t get_start_bias()
{
    return g_start_bias;
}

void inc_start_bias()
{
    if (g_start_bias < MAX_BIAS_PWM)
        g_start_bias++;
}

void dec_start_bias()
{
    if (g_start_bias > MIN_BIAS_PWM)
        g_start_bias--;
}

void validate_start_bias()
{
    if (g_start_bias > MAX_BIAS_PWM)
        g_start_bias = MAX_BIAS_PWM;

    if (g_start_bias < MIN_BIAS_PWM)
        g_start_bias = MIN_BIAS_PWM;
}

void set_start_bias(uint8_t start_bias)
{
    if (start_bias > MAX_BIAS_PWM)
        start_bias = MAX_BIAS_PWM;

    if (start_bias < MIN_BIAS_PWM)
        start_bias = MIN_BIAS_PWM;

    g_start_bias = start_bias;
}

void reset_start_bias()
{
    set_start_bias(DEFAULT_BIAS);
}



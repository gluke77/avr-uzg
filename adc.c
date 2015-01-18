#include <avr/io.h>
#include <avr/signal.h>
#include "timer.h"
#include "common.h"
#include "adc.h"
#include "lcd.h"

static uint8_t	g_use_interrupt;

uint8_t	g_adc_multiplier = 50;
uint8_t	g_adc_bias_multiplier = 50;
uint8_t	g_adc_feedback_multiplier = 50;

uint8_t		admux_value;
adc_s		adc[ADC_CHANNEL_COUNT];
uint8_t		adc_current_channel;
int8_t		adc_work_channels[] = {0,1,2,-1}; 

ISR(ADC_vect)
//SIGNAL(SIG_ADC)
{
	uint8_t		lo, hi;
	uint8_t		ch;
	
	lo = ADCL;
	hi = ADCH;
	
	ch = adc_work_channels[adc_current_channel];
	
	adc[ch].value = ((int16_t)hi << 8) + lo;
	
	adc[ch].new_value += adc_value(ch);
	adc[ch].count++;

#ifdef _ADC_CALCULATE_MIN_MAX_DELTA	

	if (adc[ch].min_value > adc_value(ch))
		adc[ch].min_value = adc_value(ch);
	
	if (adc[ch].max_value < adc_value(ch))
		adc[ch].max_value = adc_value(ch);

#endif /* _ADC_CALCULATE_MIN_MAX_DELTA */

	if (adc[ch].count >= adc[ch].max_count)
	{
		adc[ch].count = 0;
		adc[ch].mean_value = 
			(int16_t)(adc[ch].new_value / adc[ch].max_count);
		adc[ch].new_value = 0;
#ifdef _ADC_CALCULATE_MIN_MAX_DELTA	
		adc[ch].delta = adc[ch].max_value - adc[ch].min_value;
		adc[ch].min_value = 1000;
		adc[ch].max_value = -1000;
#endif /* _ADC_CALCULATE_MIN_MAX_DELTA */
	}

	adc_current_channel++;
	if (-1 == adc_work_channels[adc_current_channel])
		adc_current_channel = 0;

	ADMUX = adc_work_channels[adc_current_channel] + admux_value;
	
	ADC_START;
}


void adc_init(uint8_t use_int)
{
	uint8_t	idx;
	
	g_use_interrupt = use_int;
	
	for (idx = 0; idx < ADC_CHANNEL_COUNT; idx++)
	{
		adc[idx].timer_id = 0;
		adc[idx].delay = ADC_TIMER_DELAY;
		adc[idx].count = 0;
		adc[idx].max_count = ADC_REPEAT_COUNT;
		adc[idx].value = 0;
		adc[idx].new_value = 0;
		adc[idx].mean_value = 0;
		adc[idx].bias = 0;
		adc[idx].shift = 0;
	}

    adc[0].bias = 511;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	adc[1].bias = 511;

	
	ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
	ADC_ON;
	
	admux_value = _BV(REFS0); // | _BV(ADLAR); // Vref = AVcc with ext capacitor at Aref pin
	
	if (ADC_USE_INTERRUPT == use_int)
	{
		adc_current_channel = 0;
		ADMUX = adc_work_channels[adc_current_channel] + admux_value;
	
		ADC_INT_ENABLE;
		ADC_START;
	}
	else
	{
		adc_current_channel = 0;
		ADMUX = adc_work_channels[adc_current_channel] + admux_value;
	
		ADC_INT_DISABLE;
	}

}

int16_t adc_single_value(uint8_t channel)
{
	uint8_t		lo, hi;
	
	if (channel >= ADC_CHANNEL_COUNT)
		channel = 0;
		
	ADMUX = channel + admux_value;
	
	ADC_START;
	
	while (!ADC_READY);

	lo = ADCL;
	hi = ADCH;
	
	adc[channel].value = ((int16_t)hi << 8) + lo;
	return adc_value(channel);
}

int16_t adc_mean_value(uint8_t channel)
{
	return adc[channel].mean_value;
}

int16_t adc_value(uint8_t channel)
{
	return (adc[channel].value - adc[channel].bias) >> adc[channel].shift;
}


void do_adc(void)
{
	uint8_t	channel = adc_work_channels[adc_current_channel];
	
	if (ADC_USE_INTERRUPT == g_use_interrupt)
		return;
	
	if ((0 == adc[channel].timer_id) ||
		(0 == timer_value(adc[channel].timer_id)))
	{
		adc[channel].new_value += adc_single_value(channel);
		adc[channel].count++;
		if (adc[channel].count > adc[channel].max_count - 1)
		{
			adc[channel].count = 0;
			adc[channel].mean_value = 
				(int16_t)(adc[channel].new_value / adc[channel].max_count);
			adc[channel].new_value = 0;
		}
		
		stop_timer(adc[channel].timer_id);
		adc[channel].timer_id = start_timer(adc[channel].delay);
	}
	
	adc_current_channel++;
	if (-1 == adc_work_channels[adc_current_channel])
		adc_current_channel = 0;
}

uint16_t adc_get_count(uint8_t channel)
{
	return adc[channel].max_count;
}

void adc_set_count(uint8_t channel, uint16_t count)
{
	if (channel >= ADC_CHANNEL_COUNT)
		channel = 0;

	if (0 == count)
		count = 1;
		
	adc[channel].max_count = count;
}

uint8_t adc_get_delay(uint8_t channel)
{
	return adc[channel].delay;
}

void adc_set_delay(uint8_t channel, uint8_t delay)
{
	if (channel >= ADC_CHANNEL_COUNT)
		channel = 0;

	adc[channel].delay = delay;
}

uint16_t adc_get_timeout(uint8_t channel)
{
	if (ADC_USE_INTERRUPT == g_use_interrupt)
		return adc[channel].max_count / ADC_TIMEOUT_DIV;
	else
		return adc[channel].max_count * adc[channel].delay;
}

#ifdef _ADC_CALCULATE_MIN_MAX_DELTA

int16_t	adc_get_delta(uint8_t ch)
{
	return adc[ch].delta;
}

#endif /* _ADC_CALCULATE_MIN_MAX_DELTA */

float adc_to_current(int16_t adc)
{
	return adc * g_adc_multiplier / 1000.;
}

double adc_bias_to_current(int16_t adc)
{
	return adc * g_adc_bias_multiplier / 1000.;
}

double adc_feedback_to_current(int16_t adc)
{
	return adc * g_adc_feedback_multiplier / 1000.;
}

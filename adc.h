#if !defined _ADC_INCLUDED
	#define _ADC_INCLUDED
	
#include "common.h"

#define ADC_ON		SETBIT(ADCSRA, ADEN)
#define ADC_OFF 	CLEARBIT(ADCSRA, ADEN)

#define ADC_START	SETBIT(ADCSRA, ADSC)
#define ADC_READY	(!TESTBIT(ADCSRA, ADSC))

#define ADC_FR_ON	SETBIT(ADCSRA, ADFR)
#define ADC_FR_OFF	CLEARBIT(ADCSRA, ADFR)

#define ADC_INT_ENABLE	SETBIT(ADCSRA, ADIE)
#define ADC_INT_DISABLE	CLEARBIT(ADCSRA, ADIE)

#define ADC_USE_INTERRUPT		(1)
#define ADC_NOT_USE_INTERRUPT	(0)

#define	ADC_BIASCURRENT			(0)
#define	ADC_CURRENT				(1)
#define	ADC_AMP					(2)

// #define _ADC_CALCULATE_MIN_MAX_DELTA

void adc_init(uint8_t);
int16_t adc_value(uint8_t);
int16_t adc_single_value(uint8_t);
int16_t adc_mean_value(uint8_t);
void adc_set_count(uint8_t, uint16_t);
uint16_t adc_get_count(uint8_t);
void adc_set_delay(uint8_t, uint8_t);
uint8_t adc_get_delay(uint8_t);
void do_adc(void);
uint16_t adc_get_timeout(uint8_t);

float adc_to_current(int16_t);
extern uint8_t		g_adc_multiplier;

#ifdef _ADC_CALCULATE_MIN_MAX_DELTA
	int16_t adc_get_delta(uint8_t);
#endif /* _ADC_CALCULATE_MIN_MAX_DELTA */

#define ADC_CHANNEL_COUNT		(3)
#define ADC_REPEAT_COUNT		(512)
#define ADC_TIMER_DELAY			(5)

#define ADC_TIMEOUT_MULT		(4)
#define ADC_TIMEOUT_DIV			(10)

/*
Timeout in ms after freq change to look at adc_value is
adc[].max_count / ADC_TIMEOUT_DIV
*/

typedef struct 
{
	uint8_t		timer_id;
	uint8_t		delay;
	uint16_t	count;
	uint16_t	max_count;
	int16_t		value;
	int32_t		new_value;
	int16_t		mean_value;
	int16_t		bias;
	uint8_t		shift;
#ifdef _ADC_CALCULATE_MIN_MAX_DELTA
	int16_t		min_value;
	int16_t		max_value;
	int16_t		delta;
#endif /* _ADC_CALCULATE_MIN_MAX_DELTA */	
} adc_s;

#endif /* _ADC_INCLUDED */
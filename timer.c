#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "common.h"

#include "beep.h"

volatile uint16_t	timers[TIMER_COUNT];

volatile uint32_t		timer_seconds_total = 0;
volatile uint32_t		timer_mseconds_total = 0;
volatile uint16_t		timer_seconds_timeout = TIMER_TICS_PER_SECOND;

void do_timer(void);

void timer_init(void)
{
	uint8_t	idx;
	
	for (idx = 0; idx < TIMER_COUNT; idx++)
		timers[idx] = 0xFFFF;
		
	SETBIT(TIMSK, OCIE0);

	OCR0 = 115;										// 1 ms @ 14.7456MHz
	TCCR0 = _BV(CS00) | _BV(CS02) | _BV(WGM01);	// CTC mode, 128 prescaler
}

SIGNAL(SIG_OUTPUT_COMPARE0)
{
	uint8_t				idx;

	for (idx = 0; idx < TIMER_COUNT; idx++)
		if ((0 < timers[idx]) && (timers[idx] < 0xFFFF))
			timers[idx]--;
			
#if defined _BEEP_INCLUDED
	if (1 == timers[1])
		BEEP_OFF;
#endif /* _BEEP_INCLUDED */
			
	do_timer();
	
	if (!timer_seconds_timeout--)
	{
		timer_seconds_total++;
		timer_seconds_timeout = TIMER_TICS_PER_SECOND;
	}
	timer_mseconds_total++;
}

uint16_t timer_value(uint8_t timer_id)
{
	uint16_t	value;
	
	GLOBAL_INT_DISABLE;
	value = (timer_id < TIMER_COUNT)?timers[timer_id]:0;
	GLOBAL_INT_ENABLE;

	return value;
}

uint8_t start_timer(uint16_t delay)
{
	uint8_t	timer_id;
	
	GLOBAL_INT_DISABLE;
	
	for (timer_id = 2; timer_id < TIMER_COUNT; timer_id++)
		if (0xFFFF == timers[timer_id])
		{
			timers[timer_id] = (delay < 0xFFFF)?delay:delay-1;
			break;
		}
			
	GLOBAL_INT_ENABLE;

	return timer_id;
}

void t_delay_ms(uint16_t delay)
{
	GLOBAL_INT_DISABLE;
	timers[0] = (delay < 0xFFFF) ? delay : delay - 1;
	GLOBAL_INT_ENABLE;
	
	while (timer_value(0));
}

void stop_timer(uint8_t timer_id)
{
	GLOBAL_INT_DISABLE;
	if ((1 < timer_id) && (timer_id < TIMER_COUNT))
	timers[timer_id] = 0xFFFF;
	GLOBAL_INT_ENABLE;
}

void delay_us(uint32_t delay)
{
	uint32_t	i;
	
	i = delay * 2 + (delay >> 2);
	
	while (i--);
}

void delay_ms(uint32_t delay)
{
	delay_us(delay * 1000);
}

uint8_t	free_timer_count(void)
{
	uint8_t	timer_id;
	uint8_t	count;
	
	for (timer_id = 2, count= 0; timer_id < TIMER_COUNT; timer_id++)
		if (0xFFFF == timers[timer_id])
			count++;
			
	return count;
}
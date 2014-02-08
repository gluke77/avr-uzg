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

ISR(TIMER0_COMP_vect)
//SIGNAL(SIG_OUTPUT_COMPARE0)
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
	uint8_t		sreg;
	
	sreg = SREG;
	GLOBAL_INT_DISABLE;
	value = (timer_id < TIMER_COUNT)?timers[timer_id]:0;
	SREG = sreg;

	return value;
}

uint8_t start_timer(uint16_t delay)
{
	uint8_t	timer_id;
	uint8_t		sreg;
	
	sreg = SREG;
	GLOBAL_INT_DISABLE;
	
	for (timer_id = 2; timer_id < TIMER_COUNT; timer_id++)
		if (0xFFFF == timers[timer_id])
		{
			timers[timer_id] = (delay < 0xFFFF)?delay:delay-1;
			break;
		}
			
	SREG = sreg;

	return timer_id;
}

void delay_ms(uint16_t delay)
{
	uint8_t		sreg;
	
	sreg = SREG;
	GLOBAL_INT_DISABLE;
	timers[0] = (delay < 0xFFFF) ? delay : delay - 1;
	SREG = sreg;
	
	while (timer_value(0));
}

void stop_timer(uint8_t timer_id)
{
	uint8_t		sreg;
	
	sreg = SREG;
	GLOBAL_INT_DISABLE;
	if ((1 < timer_id) && (timer_id < TIMER_COUNT))
	timers[timer_id] = 0xFFFF;
	SREG = sreg;
}
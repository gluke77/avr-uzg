#if !defined _TIMER_INCLUDED
#define _TIMER_INCLUDER

#if !defined F_CPU
#define F_CPU					(14745600UL)
#endif /* F_CPU */

#define	TIMER_COUNT				(32)
#define TIMER_TICS_PER_SECOND	(1002)

extern	volatile uint16_t	timers[TIMER_COUNT];

void timer_init(void);
uint8_t start_timer(uint16_t /* delay_ms */);
void stop_timer(uint8_t /* timer_id */);
uint16_t timer_value(uint8_t /* timer_id */);
void t_delay_ms(uint16_t /* delay_ms */);

void delay_us(uint32_t /* delay_us */);
void delay_ms(uint32_t /* delay_ms */);

uint8_t free_timer_count(void);

extern volatile uint32_t	timer_seconds_total;
extern volatile uint32_t	timer_mseconds_total;

#endif /* _TIMER_INCLUDED */
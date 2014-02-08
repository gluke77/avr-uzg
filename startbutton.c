#include <avr\io.h>
#include "common.h"
#include "startbutton.h"
#include "beep.h"
#include "timer.h"

uint8_t	startbutton_timeout;
uint8_t	startbutton_pressed;

#define STARTBUTTON_READ_COUNT	(10)
#define	STARTBUTTON_MAX_OFF		(3)
#define STARTBUTTON_MIN_ON		(7)

uint8_t	startbutton_reads[STARTBUTTON_READ_COUNT];
uint8_t	startbutton_read_number;

startbutton_mode_e	g_startbutton_mode;

void start(void);
void stop(stop_mode_e);

void startbutton_init(void)
{
	CLEARBIT(STARTBUTTON_DDR, STARTBUTTON_PIN);
	startbutton_clean();
}

void do_startbutton(void)
{
//	if (!startbutton_timeout--)
//	{
//		startbutton_timeout = STARTBUTTON_TIMEOUT;
		startbutton_scan();
//	}
}

void startbutton_scan(void)
{
	int on_count;
	int idx;
	
	static startbutton_mode_e prev_mode = STARTBUTTON_COUNT;
	
	if (prev_mode != g_startbutton_mode)
	{
		prev_mode = g_startbutton_mode;
		startbutton_clean();
	}
	
	if (STARTBUTTON_ON != g_startbutton_mode)
		return;
	
	startbutton_reads[startbutton_read_number] = TESTBIT(STARTBUTTON_PORT, STARTBUTTON_PIN);
	startbutton_read_number++;
	
	if (startbutton_read_number < STARTBUTTON_READ_COUNT)
		return;
		
	for (on_count = 0, idx = 0; idx < STARTBUTTON_READ_COUNT; idx++)
		if (0 == startbutton_reads[idx])
			on_count ++;
		
	if (on_count < STARTBUTTON_MAX_OFF)
	{
		if (IS_UZG_RUN)
			stop(STOP_BUTTON);
	}
	else if (on_count > STARTBUTTON_MIN_ON)
	{
		if (!IS_UZG_RUN)
			start();
	}

	startbutton_clean();
}

void startbutton_clean(void)
{
	int i;
	
	startbutton_read_number = 0;
	
	for (i = 0; i < STARTBUTTON_READ_COUNT; i++)
		startbutton_reads[i] = 1;
}
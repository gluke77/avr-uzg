#include <avr/io.h>
#include "common.h"
#include "kbd.h"
#include "beep.h"
#include "timer.h"

uint8_t		kbd_timeout;

uint16_t	key_pressed_counter[KEY_COUNT];
uint16_t	key_fastmode_counter[KEY_COUNT];

uint16_t	key_big_delay = 50;
uint16_t	key_small_delay = 5;
uint16_t	key_very_small_delay = 1;
uint16_t	key_fastmode_delay = 150;

volatile uint8_t	key_pressed;
volatile uint8_t	key_state;
volatile uint8_t	old_key_state;

void kbd_init(void)
{
	uint8_t	idx;
	
	key_pressed = 0x00;
	key_state = 0xFF;
	old_key_state = 0xFF;
	
	for (idx = 0; idx < KEY_COUNT; idx++)
		key_pressed_counter[idx] = 0;
	
	KBD_OUT_DDR = 0xFF;
	CLEARBIT(KBD_IN_DDR, KBD_IN_BIT);
}

void do_kbd(void)
{
	if (!kbd_timeout--)
	{
		kbd_timeout = KBD_TIMEOUT;
		kbd_scan();
	}
}

void kbd_clear(void)
{
	key_pressed = 0x00;
}

void one_key_scan(uint8_t key_id)
{
	if (KEY_STATE(key_id) && (!OLD_KEY_STATE(key_id)))  // key released
	{
		SET_KEY_PRESSED(key_id);
		key_pressed_counter[key_id] = 0;
		key_fastmode_counter[key_id] = 0;

	}
	else if ((!KEY_STATE(key_id)) && OLD_KEY_STATE(key_id))
	{
		// key pressed
		beep_timer_ms(50);
	}
	else if (!KEY_STATE(key_id) && !OLD_KEY_STATE(key_id)) // key still pressed
	{

		if (key_small_delay + key_big_delay < ++key_pressed_counter[key_id]) 
		{
			SET_KEY_PRESSED(key_id);
			key_pressed_counter[key_id] = key_big_delay;
			beep_timer_ms(20);
		}
		
/*
		if (key_fastmode_counter[key_id] > key_fastmode_delay)
		{	
			if (++key_pressed_counter[key_id] > key_big_delay + key_very_small_delay)
			{
				key_pressed_counter[key_id] = key_big_delay;
				SET_KEY_PRESSED(key_id);
				beep_timer_ms(5);
			}
		}
		else
			if (++key_pressed_counter[key_id] > key_big_delay + key_small_delay)
			{
				key_pressed_counter[key_id] = key_big_delay;
				key_fastmode_counter[key_id]++;
				SET_KEY_PRESSED(key_id);
				beep_timer_ms(20);
			}
*/
	}
}

void kbd_scan(void)
{
	uint8_t		key_id;
	uint8_t		count;
	uint8_t		idx;

	old_key_state = key_state;
	
	for (key_id = 0; key_id < KEY_COUNT; key_id++)
	{
		KBD_OUT_PORT = 0xFF & ~(_BV(key_id));
		
		_delay_us(5);
		
		count = 0;
		
		for (idx = 0; idx < KBD_REPEAT_COUNT; idx++)
			if (TESTBIT(KBD_IN_PORT, KBD_IN_BIT))
				count++;
				
		if (count > 7)
			SETBIT(key_state, key_id);
		else if (count < 3)
			CLEARBIT(key_state, key_id);
	
		one_key_scan(key_id);
	}
}

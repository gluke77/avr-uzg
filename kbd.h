#if !defined _KBD_INCLUDED
	#define _KBD_INCLUDED
	
#include "common.h"

#define KBD_OUT_PORT	PORTC
#define KBD_OUT_DDR		DDRC

#define KBD_IN_PORT		PIND
#define KBD_IN_DDR		DDRD
#define KBD_IN_BIT		(0)

#define KEY_COUNT		(8)						
						
#define KBD_REPEAT_COUNT	(10)

						
void kbd_init(void);
void kbd_clear(void);
void one_key_scan(uint8_t);
void kbd_scan(void);
void do_kbd(void);

#define KBD_TIMEOUT	(10)

#define KEY_PRESSED(key_id)				TESTBIT(key_pressed, key_id)
#define SET_KEY_PRESSED(key_id)			SETBIT(key_pressed, key_id)
#define CLEAR_KEY_PRESSED(key_id)		CLEARBIT(key_pressed, key_id)

#define KEY_STATE(key_id)				TESTBIT(key_state, key_id)
#define SET_KEY_STATE(key_id)			SETBIT(key_state, key_id)
#define CLEAR_KEY_STATE(key_id)			CLEARBIT(key_state, key_id)

#define OLD_KEY_STATE(key_id)			TESTBIT(old_key_state, key_id)
#define SET_OLD_KEY_STATE(key_id)		SETBIT(old_key_state, key_id)
#define CLEAR_OLD_KEY_STATE(key_id)		CLEARBIT(old_key_state, key_id)

extern volatile uint8_t	key_pressed;
extern volatile uint8_t	key_state;
extern volatile uint8_t	old_key_state;

#define KEY_RUN		(0)
#define KEY_STOP	(1)
#define KEY_MENU	(7)
#define KEY_ENTER	(5)
#define KEY_LEFT	(2)
#define KEY_RIGHT	(4)
#define KEY_UP		(6)
#define KEY_DOWN	(3)

#endif /* _KBD_INCLUDED */
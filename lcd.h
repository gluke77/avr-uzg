#if !defined _LCD_INCLUDED
	#define _LCD_INCLUDED

#include "common.h"

#define LCD_DATA_PORT	PORTC
#define LCD_DATA_DDR	DDRC

#define LCD_CMD_PORT	PORTC
#define LCD_CMD_DDR		DDRC

#define LCD_RS_PORT		PORTD
#define LCD_RS_DDR		DDRD
#define LCD_RS_BIT		(7)

#define LCD_RW_PORT		PORTG
#define LCD_RW_DDR		DDRG
#define LCD_RW_BIT		(0)

#define LCD_EN_PORT		PORTG
#define LCD_EN_DDR		DDRG
#define LCD_EN_BIT		(1)

#define LCD_CTRL_INIT	{LCD_RS_DDR |= _BV(LCD_RS_BIT); LCD_RW_DDR |= _BV(LCD_RW_BIT); LCD_EN_DDR |= _BV(LCD_EN_BIT); }

#define LCD_SET_CMD_MODE	CLEARBIT(LCD_RS_PORT, LCD_RS_BIT)
#define LCD_SET_DATA_MODE	SETBIT(LCD_RS_PORT, LCD_RS_BIT)

#define LCD_SET_READ_MODE	SETBIT(LCD_RW_PORT, LCD_RW_BIT)
#define LCD_SET_WRITE_MODE	CLEARBIT(LCD_RW_PORT, LCD_RW_BIT)

#define LCD_LATCH_ON		SETBIT(LCD_EN_PORT, LCD_EN_BIT)
#define LCD_LATCH_OFF		CLEARBIT(LCD_EN_PORT, LCD_EN_BIT)

#define	LCD_TIMEOUT	(500)

extern	char lcd_line0[40];
extern	char lcd_line1[40];

void do_lcd(void);
//void update_lcd(void);
void lcd_init(void);
void lcd_write_cmd(uint8_t);
void lcd_write_data(uint8_t);
void lcd_write_data_(uint8_t);
void lcd_clear(void);
void lcd_puts(uint8_t /* line */, const char * /* msg */);
void lcd_puts_(const char * /* msg */);
	
#endif /* _LCD_INCLUDED */
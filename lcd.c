#include <avr/io.h>
#include "common.h"
#include "lcd.h"
#include "timer.h"

uint16_t	lcd_timeout;
char		lcd_line0[40];
char		lcd_line1[40];

//In ANSII codepage russians go from 0xC0 to 0xFF

uint8_t	rus[] = {'A', 0xA0, 'B', 0xA1, 0xE0, 'E', /* 0xA2, */ 0xA3, 0xA4, 0xA5, 0xA6,
				'K', 0xA7, 'M', 'H', 'O', 0xA8, 'P', 'C', 'T', 0xA9, 0xAA,
				'X', 0xE1, 0xAB, 0xAC, 0xE2, 0xAD, 0xAE, 'b', 0xAF, 0xB0, 0xB1,
				'a', 0xB2, 0xB3, 0xB4, 0xE3, 'e', /* 0xB5,*/ 0xB6, 0xB7, 0xB8, 0xB9,
				0xBA, 0xBB, 0xBC, 0xBD, 'o', 0xBE, 'p', 'c', 0xBF, 'y', 0xE4,
				'x', 0xE5, 0xC0, 0xC1, 0xE6, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7};
						

void do_lcd(void)
{
	lcd_puts(0, lcd_line0);
	lcd_puts(1, lcd_line1);
}

/*
void do_lcd(void)
{
	if (!lcd_timeout--)
	{
		lcd_timeout = LCD_TIMEOUT;
		update_lcd();
	}
}
*/

uint8_t	lcd_decode(uint8_t ch)
{
	if (0xA8 == ch)
		return 0xA2;
		
	if (0xB8 == ch)
		return 0xB5;
	
	if (0xBF < ch)
		return rus[ch - 0xC0];

	return ch;
}
		
void lcd_write_cmd(uint8_t cmd)
{
	cli();

	LCD_SET_CMD_MODE;
	LCD_SET_WRITE_MODE;
//	LCD_CMD_DDR = 0xFF;

#if !defined _LCD4
	LCD_CMD_PORT = cmd;

	LCD_LATCH_ON;
	_delay_us(1);
	LCD_LATCH_OFF;
#else

	LCD_CMD_PORT &= 0xF0;
	LCD_CMD_PORT |= (cmd >> 4);
	LCD_LATCH_ON;
	_delay_us(1);
	LCD_LATCH_OFF;
	_delay_us(1);
	
	LCD_CMD_PORT &= 0xF0;
	LCD_CMD_PORT |= (cmd & 0x0F);
	LCD_LATCH_ON;
	_delay_us(1);
	LCD_LATCH_OFF;

#endif

	sei();

	_delay_us(50);
}

void lcd_write_data_(uint8_t data)
{
	cli();

	LCD_SET_DATA_MODE;
	LCD_SET_WRITE_MODE;
//	LCD_DATA_DDR = 0xFF;

#if !defined _LCD4
	LCD_DATA_PORT = data;

	LCD_LATCH_ON;
	_delay_us(1);
	LCD_LATCH_OFF;
#else

	LCD_DATA_PORT &= 0xF0;
	LCD_DATA_PORT |= (data >> 4);
	LCD_LATCH_ON;
	_delay_us(1);
	LCD_LATCH_OFF;
	_delay_us(1);
	
	LCD_DATA_PORT &= 0xF0;
	LCD_DATA_PORT |= (data & 0x0F);
	LCD_LATCH_ON;
	_delay_us(1);
	LCD_LATCH_OFF;

#endif

	sei();

	_delay_us(50);
}	

void lcd_write_data(uint8_t data)
{
	lcd_write_data_(lcd_decode(data));
}

void lcd_init(void)
{
	LCD_CTRL_INIT;

	LCD_DATA_DDR = 0xFF;

	_delay_ms(100);
	
#if !defined _LCD4
	lcd_write_cmd(0x38);
/*	_delay_ms(5);
	lcd_write_cmd(0x38);
	_delay_us(100);
	lcd_write_cmd(0x38);
	_delay_ms(5);
*/
#else	
	lcd_write_cmd(0x2C);
#endif

	lcd_write_cmd(0x0C);
	lcd_write_cmd(0x06);
	lcd_write_cmd(0x01);
	_delay_ms(2);
}

void lcd_clear(void)
{
	lcd_write_cmd(0x01);
	_delay_ms(2);
}

void lcd_puts(uint8_t line, const char * msg)
{
	lcd_write_cmd(0x80 + line * 0x40);
	if (0 != msg)
		while (*msg)
			lcd_write_data(*msg++);
}

void lcd_puts_(const char * msg)
{
	if (0 != msg)
		while (*msg)
			lcd_write_data(*msg++);
}

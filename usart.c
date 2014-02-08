// #define ENABLE_BIT_DEFINITIONS
#include <avr/io.h>
#include <avr/signal.h>
#include <string.h>
#include "usart.h"
#include "common.h"
#include "timer.h"

#if defined _USART0_ENABLED

volatile uint8_t	usart0_inbuf[USART0_INBUF_SIZE];
volatile uint8_t	usart0_msg_ready = 0;
volatile uint8_t	usart0_inbuf_pos = 0;

uint8_t		usart0_mode;
uint8_t		usart0_protocol;

uint8_t usart0_getmode(void)
{
	return usart0_mode;
}

uint8_t usart0_getprotocol(void)
{
	return usart0_protocol;
}

void usart0_setprotocol_modbus(void)
{
	USART0_SET_7N2;
	usart0_protocol = USART_PROTOCOL_MODBUS;
}

void usart0_setprotocol_101(void)
{
	USART0_SET_8N1;
	usart0_protocol = USART_PROTOCOL_101;
}

void usart0_setprotocol_uzs(void)
{
	USART0_SET_8N1;
	usart0_protocol = USART_PROTOCOL_UZS;
}

void usart0_init(uint8_t mode, uint32_t baud)
{
	uint16_t	mult;
	
	mult = FOSC / 16 / baud - 1;
	
	usart0_mode = mode;

	UCSR0A = 0;
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
	UCSR0C = _BV(UCSZ01) | _BV(USBS0);
	UBRR0H = (uint8_t)(mult >> 8); 
	UBRR0L = (uint8_t)(mult); 

	usart0_setprotocol_modbus();
	
	if (usart0_mode != USART_RS232)
		SETBIT(USART0_DIR_DDR, USART0_DIR_BIT);

	if (USART_RS485_MASTER == usart0_mode)
	{
		CLEARBIT(UCSR0B, RXEN0);
		SETBIT(USART0_DIR_PORT, USART0_DIR_BIT); 
		USART0_RX_INT_DISABLE;
	}
	else if (USART_RS485_SLAVE == usart0_mode)
	{
		SETBIT(UCSR0B, RXEN0);
		CLEARBIT(USART0_DIR_PORT, USART0_DIR_BIT); 
		USART0_RX_INT_ENABLE;
	}
	else if (USART_RS232 == usart0_mode)
	{
		USART0_RX_INT_ENABLE;
	}
}

void usart0_write(void)
{
	if (usart0_mode != USART_RS232)
	{
		SETBIT(USART0_DIR_PORT, USART0_DIR_BIT); 
		CLEARBIT(UCSR0B, RXEN0);
		delay_ms(5);
		USART0_RX_INT_DISABLE;
	}
}

void usart0_read(void)
{
	if (usart0_mode != USART_RS232)
	{
		while (!TESTBIT(UCSR0A, TXC0));
		delay_ms(2);
		SETBIT(UCSR0B, RXEN0);
		CLEARBIT(USART0_DIR_PORT, USART0_DIR_BIT); 
//		delay_ms(5);
		USART0_RX_INT_ENABLE;
	}
}

SIGNAL(SIG_UART0_RECV)
{
	uint8_t	byte;
	
	byte = UDR0;

	if (USART_PROTOCOL_MODBUS == usart0_getprotocol())
	{
		if (':' == byte)
		{
			usart0_inbuf[0] = ':';
			usart0_inbuf_pos = 1;
			usart0_msg_ready = 0;
		}
		else if (usart0_inbuf_pos)
			usart0_inbuf[usart0_inbuf_pos++] = byte;
		
		if (':' == usart0_inbuf[0] &&
			'\r' == usart0_inbuf[usart0_inbuf_pos-2] &&
			'\n' == usart0_inbuf[usart0_inbuf_pos-1])
		{
			usart0_msg_ready = 1;
			usart0_inbuf_pos = 0;
		}
		else if (USART0_INBUF_SIZE == usart0_inbuf_pos)
		{
			usart0_inbuf_pos = 0;
			usart0_msg_ready = 0;
		}
	}
	else if (USART_PROTOCOL_101 == usart0_getprotocol())
	{
		if (0x23 == byte)
		{
			usart0_inbuf[0] = 0x23;
			usart0_inbuf_pos = 1;
			usart0_msg_ready = 0;
		}
		else if (usart0_inbuf_pos)
			usart0_inbuf[usart0_inbuf_pos++] = byte;
		
		if (0x23 == usart0_inbuf[0] &&
			0x0d == usart0_inbuf[usart0_inbuf_pos-1])
		{
			usart0_msg_ready = 1;
			usart0_inbuf_pos = 0;
		}
		else if (USART0_INBUF_SIZE == usart0_inbuf_pos)
		{
			usart0_inbuf_pos = 0;
			usart0_msg_ready = 0;
		}
	}
}

int	usart0_putchar( char ch )
{
	while (!TESTBIT(UCSR0A, UDRE0));
	UDR0 = ch;
	return 0;
}

int	usart0_getchar( void )
{
	while (!TESTBIT(UCSR0A, RXC0));
	return UDR0;
}

int usart0_data_ready(void)
{
	return TESTBIT(UCSR0A, RXC0);
}

result_e usart0_cmd(uint8_t * req, uint8_t * ack, size_t ack_size, uint16_t delay)
{
	result_e		res;
	uint8_t			timer_id;
	uint8_t			idx;
	
	if (USART_RS485_SLAVE == usart0_getmode())
		usart0_write();
	
	if (USART_PROTOCOL_MODBUS == usart0_getprotocol())
	{
		for (idx = 0; req[idx] != '\n'; idx++)
			usart0_putchar(req[idx]);
		usart0_putchar('\n');
	}
	else if (USART_PROTOCOL_101 == usart0_getprotocol())
	{
		for (idx = 0; req[idx] != 0x0d; idx++)
			usart0_putchar(req[idx]);
		usart0_putchar(0x0d);
	}

	if (NULL == ack)
	{
		if (USART_RS485_SLAVE == usart0_getmode())
			usart0_read();
		return RESULT_OK;
	}

	usart0_read();
	
	res = RESULT_TIMEOUT;
		
	timer_id = start_timer(delay);
		
	while (timer_value(timer_id))
	{
		USART0_RX_INT_DISABLE;
		if (usart0_msg_ready)
		{
			res = RESULT_OK;
			usart0_msg_ready = 0;
			memcpy(ack, usart0_inbuf, (ack_size < USART0_INBUF_SIZE)?ack_size:USART0_INBUF_SIZE);
			stop_timer(timer_id);
			break;
		}
		USART0_RX_INT_ENABLE;	
	}
	stop_timer(timer_id);

	if (USART_RS485_MASTER == usart0_getmode())
		usart0_write();	

	return res;
}

#endif /* _USART0_ENABLED */

#if defined _USART1_ENABLED

volatile uint8_t	usart1_inbuf[USART1_INBUF_SIZE];
volatile uint8_t	usart1_msg_ready = 0;
volatile uint8_t	usart1_inbuf_pos = 0;

uint8_t		usart1_mode;
uint8_t		usart1_protocol;

uint8_t usart1_getmode(void)
{
	return usart1_mode;
}

uint8_t usart1_getprotocol(void)
{
	return usart1_protocol;
}

void usart1_setprotocol_modbus(void)
{
	USART1_SET_7N2;
	usart1_protocol = USART_PROTOCOL_MODBUS;
}

void usart1_setprotocol_101(void)
{
	USART1_SET_8N1;
	usart1_protocol = USART_PROTOCOL_101;
}

void usart1_setprotocol_uzs(void)
{
	USART1_SET_8N1;
	usart1_protocol = USART_PROTOCOL_UZS;
}

void usart1_init(uint8_t mode, uint32_t baud)
{
	uint16_t	mult;
	
	mult = FOSC / 16 / baud - 1;

	usart1_mode = mode;

	UCSR1A = 0;
	UCSR1B = _BV(TXEN1) | _BV(RXEN1);
	UCSR1C = _BV(UCSZ11) | _BV(USBS1);		// 7N2
	UBRR1H = (uint8_t)(mult>>8); 
	UBRR1L = (uint8_t)(mult); 

	usart1_setprotocol_modbus();

	if (usart1_mode != USART_RS232)
		SETBIT(USART1_DIR_DDR, USART1_DIR_BIT);

	if (USART_RS485_MASTER == usart1_mode)
	{
		CLEARBIT(UCSR1B, RXEN1);
		SETBIT(USART1_DIR_PORT, USART1_DIR_BIT);
		USART1_RX_INT_DISABLE; 
	}
	else if (USART_RS485_SLAVE == usart1_mode)
	{
		SETBIT(UCSR1B, RXEN1);
		CLEARBIT(USART1_DIR_PORT, USART1_DIR_BIT); 
		USART1_RX_INT_ENABLE;
	}
	else if (USART_RS232 == usart1_mode)
	{
		USART1_RX_INT_ENABLE;
	}
}


void usart1_write(void)
{
	if (usart1_mode != USART_RS232)
	{
		SETBIT(USART1_DIR_PORT, USART1_DIR_BIT); 
		CLEARBIT(UCSR1B, RXEN1);
		delay_ms(5);
		USART1_RX_INT_DISABLE;
	}
}

void usart1_read(void)
{
	if (usart1_mode != USART_RS232)
	{
		while (!TESTBIT(UCSR1A, TXC1));
	//	DELAY(2800);
		delay_ms(2);
		SETBIT(UCSR1B, RXEN1);
		CLEARBIT(USART1_DIR_PORT, USART1_DIR_BIT); 
//		delay_ms(5);
		USART1_RX_INT_ENABLE;
	}
}

SIGNAL(SIG_UART1_RECV)
{
	uint8_t	byte;
	
	byte = UDR1;
//	lcd_write_data(byte);

	if (USART_PROTOCOL_MODBUS == usart1_getprotocol())
	{
		if (':' == byte)
		{
			usart1_inbuf[0] = ':';
			usart1_inbuf_pos = 1;
			usart1_msg_ready = 0;
		}
		else if (usart1_inbuf_pos)
			usart1_inbuf[usart1_inbuf_pos++] = byte;
		
		if (':' == usart1_inbuf[0] &&
			'\r' == usart1_inbuf[usart1_inbuf_pos-2] &&
			'\n' == usart1_inbuf[usart1_inbuf_pos-1])
		{
			usart1_msg_ready = 1;
		}
		else if (USART1_INBUF_SIZE == usart1_inbuf_pos)
		{
			memset(usart1_inbuf, 0x00, USART1_INBUF_SIZE);
			usart1_inbuf_pos = 0;
			usart1_msg_ready = 0;
		}
	}
	else if (USART_PROTOCOL_UZS == usart1_getprotocol())
	{
		if (('>' != byte) && ('\n' != byte))
		{
			usart1_inbuf[usart1_inbuf_pos++] = byte;
			usart1_msg_ready = 0;
		}
		
		if ('\r' == usart1_inbuf[usart1_inbuf_pos-1])
		{
			usart1_msg_ready = 1;
			if (usart1_inbuf_pos < USART1_INBUF_SIZE)
				usart1_inbuf[usart1_inbuf_pos++] = 0x00;
			usart1_inbuf_pos = 0;
//			parse_inbuf(usart1_inbuf_pos);
		}
		else if (USART1_INBUF_SIZE == usart1_inbuf_pos)
		{
			memset(usart1_inbuf, 0x00, USART1_INBUF_SIZE);
			usart1_inbuf_pos = 0;
			usart1_msg_ready = 0;
		}
	}
}

int	usart1_putchar( char ch )
{
	while (!TESTBIT(UCSR1A, UDRE1));
	UDR1 = ch;
	return 0;
}

int	usart1_getchar( void )
{
	while (!TESTBIT(UCSR1A, RXC1));
	return UDR1;
}

int usart1_data_ready(void)
{
	return TESTBIT(UCSR1A, RXC1);
}

result_e usart1_cmd(uint8_t * req, uint8_t * ack, size_t ack_size, uint16_t delay)
{
	result_e		res;
	uint8_t			timer_id;
	uint8_t			idx;
	
	if (USART_RS485_SLAVE == usart1_getmode())
		usart1_write();
	
	if (USART_PROTOCOL_MODBUS == usart1_getprotocol())
	{
		for (idx = 0; req[idx] != '\n'; idx++)
			usart1_putchar(req[idx]);
		usart1_putchar('\n');
	}
	else if (USART_PROTOCOL_101 == usart1_getprotocol())
	{
		for (idx = 0; req[idx] != 0x0d; idx++)
			usart1_putchar(req[idx]);
		usart1_putchar(0x0d);
	}
	
	
	if (NULL == ack)
	{
		if (USART_RS485_SLAVE == usart1_getmode())
			usart1_read();
		return RESULT_OK;
	}

	usart1_read();
	
	res = RESULT_TIMEOUT;
		
	timer_id = start_timer(delay);
		
	while (timer_value(timer_id))
	{
		USART1_RX_INT_DISABLE;
		if (usart1_msg_ready)
		{
			res = RESULT_OK;
			usart1_msg_ready = 0;
			memcpy(ack, usart1_inbuf, (ack_size < USART1_INBUF_SIZE)?ack_size:USART1_INBUF_SIZE);
			stop_timer(timer_id);
			break;
		}
		USART1_RX_INT_ENABLE;	
	}
	stop_timer(timer_id);

	if (USART_RS485_MASTER == usart1_getmode())
		usart1_write();	

	return res;
}

#endif /* _USART1_ENABLED */

#if !defined __USART_H_INCLUDED_
#define __USART_H_INCLUDED_

#include <avr/io.h>
#include <string.h>
#include "common.h"

#define USART_OK        ( 0 )
#define USART_BUSY      ( 1 )
#define USART_TRUNCATED ( 2 )

#define USART_PROTOCOL_MODBUS	(0)
#define USART_PROTOCOL_101		(1)
#define USART_PROTOCOL_UZS		(2)

#define USART_RS232			(0)
#define USART_RS485_MASTER	(1)
#define USART_RS485_SLAVE	(2)

#if defined _USART0_ENABLED

#define	USART0_INBUF_SIZE	(64)

#define USART0_RX_INT_ENABLE	SETBIT(UCSR0B, RXCIE0)
#define USART0_RX_INT_DISABLE	CLEARBIT(UCSR0B, RXCIE0)

#define USART0_RX_INT_ENABLED	TESTBIT(UCSR0B, RXCIE0)
#define USART0_RX_INT_DISABLED	(!USART0_RX_INT_ENABLED)

#define USART0_SET_8N1			(UCSR0C = _BV(UCSZ01) | _BV(UCSZ00))
#define USART0_SET_7N2			(UCSR0C = _BV(UCSZ01) | _BV(USBS0))

extern volatile uint8_t	usart0_inbuf[USART0_INBUF_SIZE];
extern volatile uint8_t	usart0_inbuf_pos;
extern volatile uint8_t	usart0_msg_ready;

int	usart0_putchar(char /* ch */);
int	usart0_getchar(void);
int	usart0_data_ready(void);
void usart0_init(uint8_t /* mode */, uint32_t /* baud */);
void usart0_write(void);
void usart0_read(void);
uint8_t usart0_getmode(void);
result_e usart0_cmd(uint8_t * /* req */, uint8_t * /* ack */,
	size_t /* ack_size */, uint16_t /* delay */);

uint8_t	usart0_getprotocol(void);
void usart0_setprotocol_modbus(void);
void usart0_setprotocol_101(void);
void usart0_setprotocol_uzs(void);

#endif /* _USART0_ENABLED */

#if defined _USART1_ENABLED

#define	USART1_INBUF_SIZE	(64)

extern volatile uint8_t	usart1_inbuf[USART1_INBUF_SIZE];
extern volatile uint8_t	usart1_inbuf_pos;
extern volatile uint8_t	usart1_msg_ready;

int	usart1_putchar(char /* ch */);
int	usart1_getchar(void);
int	usart1_data_ready(void);
void usart1_init(uint8_t /* mode */, uint32_t /* baud */);
void usart1_write(void);
void usart1_read(void);
uint8_t usart1_getmode(void);
result_e usart1_cmd(uint8_t * /* req */, uint8_t * /* ack */,
	size_t /* ack_size */, uint16_t /* delay */);

uint8_t	usart1_getprotocol(void);
void usart1_setprotocol_modbus(void);
void usart1_setprotocol_101(void);
void usart1_setprotocol_uzs(void);

#define USART1_RX_INT_ENABLE	SETBIT(UCSR1B, RXCIE1)
#define USART1_RX_INT_DISABLE	CLEARBIT(UCSR1B, RXCIE1)

#define USART1_RX_INT_ENABLED	TESTBIT(UCSR1B, RXCIE1)
#define USART1_RX_INT_DISABLED	(!USART1_RX_INT_ENABLED)

#define USART1_SET_8N1			(UCSR1C = _BV(UCSZ11) | _BV(UCSZ10))
#define USART1_SET_7N2			(UCSR1C = _BV(UCSZ11) | _BV(USBS1))

#endif /* _USART1_ENABLED */

#if defined _UZS

	#define USART0_BAUDRATE		(7)		// 115200 @ 14.4756 MHz
//	#define USART1_BAUDRATE		(383)	// 2400 @ 14.4756 MHz
	#define USART1_BAUDRATE		(7)

	#define USART0_DIR_DDR		DDRE
	#define USART0_DIR_PORT		PORTE
	#define USART0_DIR_BIT		(2)

	#define USART1_DIR_DDR		DDRD
	#define USART1_DIR_PORT		PORTD
	#define USART1_DIR_BIT		(4)

#endif /* _UZS */


#endif /* __USART_H_INCLUDED_ */

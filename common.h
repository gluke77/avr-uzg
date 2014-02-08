#if !defined __COMMON_H_INCLUDED_
#define __COMMON_H_INCLUDED_

#include <avr/io.h>
#include <avr/interrupt.h>

#define FW_VERSION	("20070718")

#define TRUE 1
#define FALSE 0

#define FOSC	(14745600UL)

#define TESTBIT(port, bit)	(port & _BV(bit))
#define SETBIT(port, bit)	(port |= _BV(bit))
#define CLEARBIT(port, bit)	(port &= ~(_BV(bit)))

#define	EXT_MEM_INIT	{MCUCR = (1<<SRE);} // \
			//			SETBIT(XMCRB, XMM0); \
			//			SETBIT(XMCRB, XMM1); \
			//			SETBIT(XMCRB, XMM2);}

//#define	GLOBAL_INT_ENABLE   SETBIT( SREG, 7 )
//#define	GLOBAL_INT_DISABLE  CLEARBIT( SREG, 7 )

#define	GLOBAL_INT_ENABLE   sei()
#define	GLOBAL_INT_DISABLE  cli()

#define DELAY(ch)	{uint32_t i = ch; while (i--);}

// 1 us ~= DELAY(3)

#define F_CPU (14745600UL)

#include <avr/delay.h>

#define TRUE 1
#define FALSE 0

typedef enum
{
	RESULT_OK = 0,
	RESULT_BAD_ACK,
	RESULT_BAD_MSG,
	RESULT_BAD_CRC,
	RESULT_BUFFER_OVERFLOW,
	RESULT_MSG_TOO_BIG,
	RESULT_ALARM,
	RESULT_TIMEOUT,
	RESULT_UNKNOWN_PORT,
	RESULT_IGNORE_CMD
} result_e;

/*
#define RUN_PORT		PORTC
#define RUN_DDR			DDRC
#define RUN_BIT			0

#define	UZG_RUN			SETBIT(RUN_PORT, RUN_BIT)
#define UZG_STOP		CLEARBIT(RUN_PORT, RUN_BIT)
#define IS_UZG_RUN		TESTBIT(RUN_PORT, RUN_BIT)
*/

extern uint8_t is_power_on(void);

#define IS_UZG_RUN		(is_power_on())

#define RUN_PFC_PORT	PORTG
#define RUN_PFC_DDR		DDRG
#define RUN_PFC_BIT		3

#define PFC_RUN			CLEARBIT(RUN_PFC_PORT, RUN_PFC_BIT)
#define PFC_STOP		SETBIT(RUN_PFC_PORT, RUN_PFC_BIT)
#define IS_PFC_RUN		(!TESTBIT(RUN_PFC_PORT, RUN_PFC_BIT))

typedef enum
{
	PFC_OFF = 1,
	PFC_ON,
	PFC_AUTO,
	PFC_COUNT
} pfc_mode_e;

extern pfc_mode_e	g_pfc_mode;

typedef enum
{
	KEEP_OFF = 1,
	KEEP_CURRENT,
	KEEP_AMP,
	KEEP_COUNT
} keep_mode_e;

extern keep_mode_e	g_keep_mode;

typedef enum
{
	AUTOSEARCH_OFF = 0,
	AUTOSEARCH_ON,
	AUTOSEARCH_COUNT
} autosearch_mode_e;

extern autosearch_mode_e	g_autosearch_mode;

typedef enum
{
	FAULT_INTERRUPTS_OFF = 0,
	FAULT_INTERRUPTS_ON,
	FAULT_INTERRUPTS_COUNT
} fault_interrupts_mode_e;

extern fault_interrupts_mode_e	g_fault_interrupts_mode;

#define TEST_FAULT_PWM	((!TESTBIT(PINE, PE5)) && (FAULT_INTERRUPTS_ON == g_fault_interrupts_mode))
#define TEST_FAULT_BIAS	((!TESTBIT(PINE, PE7)) && (FAULT_INTERRUPTS_ON == g_fault_interrupts_mode))

typedef enum
{
	STOP_BUTTON = 0,
	STOP_485,
	STOP_TEMPERATURE,
	STOP_FAULT_PWM,
	STOP_FAULT_BIAS,
	STOP_NOT_CHANGE,
	STOP_COUNT
} stop_mode_e;

extern stop_mode_e g_stop_mode;

extern int8_t		g_keep_freq_step;
extern int8_t		g_keep_freq_max_delta;

extern uint32_t	g_freq_upper;
extern uint32_t	g_freq_lower;

extern uint32_t	g_baudrate;
extern uint8_t		g_modbus_id;

#endif /* __COMMON_H_INCLUDED_*/

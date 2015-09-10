	#if !defined __COMMON_H_INCLUDED_
#define __COMMON_H_INCLUDED_

#define FOSC	(F_CPU)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <inttypes.h>
#include <util/delay.h>

#define FW_VERSION	("20150910")

#define TRUE 1
#define FALSE 0

#define TESTBIT(port, bit)	(port & _BV(bit))
#define SETBIT(port, bit)	(port |= _BV(bit))
#define CLEARBIT(port, bit)	(port &= ~(_BV(bit)))

#define TESTBITL(port, bit)		((port) & (1L << (bit)))
#define SETBITL(port, bit)		((port) |= (1L << (bit)))
#define CLEARBITL(port, bit)	((port) &= ~(1L << (bit)))


#define	EXT_MEM_INIT	{MCUCR = (1<<SRE);} //
			//			SETBIT(XMCRB, XMM0); 
			//			SETBIT(XMCRB, XMM1);
			//			SETBIT(XMCRB, XMM2);}

//#define	GLOBAL_INT_ENABLE   SETBIT( SREG, 7 )
//#define	GLOBAL_INT_DISABLE  CLEARBIT( SREG, 7 )

#define	GLOBAL_INT_ENABLE   sei()
#define	GLOBAL_INT_DISABLE  cli()

#define DELAY(ch)	{uint32_t i = ch; while (i--);}

// 1 us ~= DELAY(3)

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
	STOP_TEMPERATURE1,
	STOP_TEMPERATURE2,
	STOP_FAULT_PWM,
	STOP_FAULT_BIAS,
	STOP_NOT_CHANGE,
	STOP_COUNT
} stop_mode_e;

extern stop_mode_e g_stop_mode;

typedef enum
{
	STARTBUTTON_OFF = 0,
	STARTBUTTON_ON,
	STARTBUTTON_COUNT
} startbutton_mode_e;

extern startbutton_mode_e g_startbutton_mode;


extern int8_t		g_keep_freq_step;
extern int8_t		g_keep_freq_max_delta;

extern uint32_t	g_freq_upper;
extern uint32_t	g_freq_lower;

extern uint32_t	g_freq_supermax;
extern uint32_t	g_freq_supermin;

extern uint32_t	g_baudrate;
extern uint8_t		g_modbus_id;

#define DIN_SIZE	(16)
extern	char	g_din[DIN_SIZE];

#define FREQ_UPPER_ADDR	(0)
#define FREQ_LOWER_ADDR	(2)
#define FREQ_ADDR		(4)
#define PWM_SHIFT_ADDR	(6)
#define	PWM_BASE_ADDR	(8)

#define ADC0_DELAY_ADDR	(9)
#define ADC0_COUNT_ADDR	(10)
#define ADC1_DELAY_ADDR	(12)
#define ADC1_COUNT_ADDR	(13)
#define ADC2_DELAY_ADDR	(15)
#define ADC2_COUNT_ADDR	(16)
#define ADC3_DELAY_ADDR	(18)
#define ADC3_COUNT_ADDR	(19)

#define PFC_MODE_ADDR	(21)
#define INT_TIMEOUT_ADDR	(22)
#define KEEP_MODE_ADDR	(24)
#define KEEP_STEP_ADDR	(25)
#define KEEP_DELTA_ADDR	(26)
#define MAX_PWM_ADDR	(27)
#define MIN_PWM_ADDR	(28)
#define BAUD_LO_ADDR	(29)
#define BAUD_HI_ADDR	(31)
#define MODBUS_ID_ADDR	(33)
#define TEMP_ALARM_ADDR	(34)
#define TEMP_STOP_ADDR	(36)

#define POWER_PWM_SHIFT_ADDR		(38)
#define POWER_PWM_BASE_ADDR			(39)
#define MAX_POWER_PWM_ADDR			(40)
#define MIN_POWER_PWM_ADDR			(41)
#define AUTOSEARCH_MODE_ADDR		(42)
#define FAULT_INTERRUPTS_MODE_ADDR	(43)
#define BIAS_PWM_MULTIPLIER_ADDR	(44)
#define ADC_MULTIPLIER_ADDR			(46)
#define DIN_ADDR					(47)
#define SUPERMAX_BIAS_PWM_ADDR		(63)

#define ADC0_BIAS_ADDR				(64)
#define ADC1_BIAS_ADDR				(66)
#define ADC2_BIAS_ADDR				(68)
#define ADC3_BIAS_ADDR				(70)

#define ADC_BIAS_MULTIPLIER_ADDR		(72)
#define ADC_FEEDBACK_MULTIPLIER_ADDR	(73)

#define SUPERMAX_FREQ_ADDR			(74)
#define SUPERMIN_FREQ_ADDR			(76)

#define TEMP2_ALARM_ADDR	(78)
#define TEMP2_STOP_ADDR	(80)

#define STARTBUTTON_MODE_ADDR	(82)

// next addr 83

#undef _NARROW_FREQ
#define _BIAS_CHANGEABLE
#undef _BIAS_SHIFT_CHANGEABLE
#define _MAX_BIAS_CHANGEABLE
#define _MIN_BIAS_CHANGEABLE
#define _INT_TIMEOUT_CHANGEABLE
#undef _SUPERMAX_BIAS_CHANGEABLE
#define _POWER_CHANGEABLE
#define _KEEP_CHANGEABLE
#define _ADC_SHOW
#undef _STARTBUTTON_ENABLED

#define DEFAULT_INT_TIMEOUT			(500)

#define DEFAULT_PFC_MODE			(PFC_OFF)

#define MIN_BIAS_PWM_MULTIPLIER		(700)
#define MAX_BIAS_PWM_MULTIPLIER		(1500)
#define DEFAULT_BIAS_PWM_MULTIPLIER	(950)

// PWM (current) settings
#define SUPERMAX_BIAS_PWM		(100)
#define MAX_BIAS_PWM_BASE		(8.)
#define MIN_BIAS_PWM			(10)
#define DEFAULT_BIAS_PWM_BASE	(50)
#define DEFAULT_BIAS_PWM_SHIFT	(0)



#endif /* __COMMON_H_INCLUDED_*/

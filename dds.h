#if !defined __DDS_H_INCLUDED_
#define __DDS_H_INCLUDED_

#include "common.h"
//#include "spi.h"

/*
#define PORT_DDS	PORTD
#define DDR_DDS		DDRD
#define DDS_RESET	PD4
#define DDS_SLEEP	PD5
#define DDS_PSEL	PD6
#define	DDS_FSEL	PD7
*/

#define	DDR_SPI		DDRB
#define PORT_SPI	PORTB
#define	DD_SCK		PB3
//#define DD_MISO		PB3
#define	DD_MOSI		PB4
#define	DD_SS		PB2

/*
#define SPI_ON			CLEARBIT(PORT_SPI, DD_SS)
#define SPI_OFF			SETBIT(PORT_SPI, DD_SS)
*/

#define DDS_RESET_ON	SETBIT(PORT_DDS, DDS_RESET)
#define DDS_RESET_OFF	CLEARBIT(PORT_DDS, DDS_RESET)
#define TEST_DDS_RESET	TESTBIT(PORT_DDS, DDS_RESET)

#define DDS_SLEEP_ON	SETBIT(PORT_DDS, DDS_SLEEP)
#define DDS_SLEEP_OFF	CLEARBIT(PORT_DDS, DDS_SLEEP)
#define TEST_DDS_SLEEP	TESTBIT(PORT_DDS, DDS_SLEEP)

#define	DDS_CONTROL_WORD	(0x2068)
#define DDS_MCLK_FREQ		(F_CPU)

void dds_init(void);
void dds_setmultiplier(uint32_t);
void dds_setfreq(uint32_t);

extern 	uint32_t	g_freq_supermax;
extern	uint32_t	g_freq_supermin;

extern	uint32_t	g_freq_upper;
extern	uint32_t	g_freq_lower;

extern	uint32_t	g_dds_mult;
extern	uint32_t	g_dds_freq;

#define DDS_MAX_FREQ	(40000)
#define DDS_MIN_FREQ	(15000)

#ifdef _NARROW_FREQ
	#define DDS_MAX_FREQ	(22500)
	#define DDS_MIN_FREQ	(18000)
#endif // _NARROW_FREQ


#endif /* __DDS_H_INCLUDED_ */

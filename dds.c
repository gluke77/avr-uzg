#include <avr/io.h>
#include "common.h"
#include "dds.h"
//#include "spi.h"

uint32_t	g_dds_mult;
uint32_t	g_dds_freq;
/*
void spi_writeword(uint16_t word)
{
	SPI_ON;
	
	SPI_WRITE((uint8_t)(word >> 8));
	SPI_WRITE((uint8_t)(word & 0x00FF));

	SPI_OFF;
}
*/
void dds_writeword(uint16_t word)
{
	uint8_t	i;
	
	SETBIT(PORT_SPI, DD_SCK);
	CLEARBIT(PORT_SPI, DD_SS);
	
	for (i = 0; i < 16; i++)
	{
		if (word & 0x8000)
			SETBIT(PORT_SPI, DD_MOSI);
		else
			CLEARBIT(PORT_SPI, DD_MOSI);
		
		CLEARBIT(PORT_SPI, DD_SCK);
		
		word <<= 1;
		
		SETBIT(PORT_SPI, DD_SCK);
	}
	
	SETBIT(PORT_SPI, DD_SS);
}

void dds_init(void)
{

	SETBIT(DDR_SPI, DD_SS);
	SETBIT(DDR_SPI, DD_SCK);
	SETBIT(DDR_SPI, DD_MOSI);
/*
	SETBIT(DDR_DDS, DDS_RESET);
	SETBIT(DDR_DDS, DDS_SLEEP);
	SETBIT(DDR_DDS, DDS_PSEL);
	SETBIT(DDR_DDS, DDS_FSEL);
	DDS_SLEEP_OFF;
	DDS_RESET_OFF;
	CLEARBIT(PORT_DDS, DDS_PSEL);
	CLEARBIT(PORT_DDS, DDS_FSEL);
*/

	dds_writeword(0x1000); // RESET = 1
	dds_writeword(0xC000); // PHASE0 = 0
	dds_writeword(0xE000); // PHASE1 = 0
	dds_writeword(DDS_CONTROL_WORD);
}

void dds_setmultiplier(uint32_t mult)
{
	uint16_t word;

	dds_writeword(DDS_CONTROL_WORD);
	
	mult &= 0x0FFFFFFF;

	word = 0x4000 | ((uint16_t)(mult & 0x00003FFF));
	dds_writeword(word);
	
	word = 0x4000 | ((uint16_t)((mult >> 14) & 0x00003FFF));
	dds_writeword(word);
	
//	g_dds_freq = (uint32_t)((mult >> 4) * (uint64_t)DDS_MCLK_FREQ / 0x0FFFFFFF);
}

void dds_setfreq(uint32_t freq)
{
	if (DDS_MIN_FREQ > freq)
		freq = DDS_MIN_FREQ;
	
	if (DDS_MAX_FREQ < freq)
		freq = DDS_MAX_FREQ;
		
	g_dds_freq = freq;
	
	freq *= 200; 	// на схеме Виталика мощность задается ШИМом 0-99
					// опорная частота берется с выхода синтезатора
	
	uint32_t mult = (uint32_t)(((uint64_t)freq << 28) / DDS_MCLK_FREQ);
	// F_out = MULT * F_mclk / 2^28, MULT = freq * (2^28) / F_mclk
	
//	mult <<= 4; на Юркиной схеме был делитель
	
	dds_setmultiplier(mult);
	
	g_dds_mult = mult;
}
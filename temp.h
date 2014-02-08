#if !defined _TEMP_INCLUDED
#define _TEMP_INCLUDED

#include "common.h"

#include "ds18x20.h"

#define MAX_DS18X20_COUNT	(2)

#define TEMP_TIMEOUT		(2000)

extern uint16_t g_temp_alarm;
extern uint16_t g_temp_stop;

float	temp_value(void);
void	do_temp(void);
void	temp_init(void);

#endif /* _TEMP_INCLUDED */
#if !defined _TEMP_INCLUDED
#define _TEMP_INCLUDED

#include "common.h"

#include "ds18x20.h"

#define MAX_DS18X20_COUNT	(2)

#define TEMP_TIMEOUT		(2000)

extern uint16_t g_temp_alarm[MAX_DS18X20_COUNT];
extern uint16_t g_temp_stop[MAX_DS18X20_COUNT];

//extern uint16_t g_temp_alarm2;
//extern uint16_t g_temp_stop2;

float	temp_value(int /* sensor_id */);
void	do_temp(void);
void	temp_init(void);

#endif /* _TEMP_INCLUDED */
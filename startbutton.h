#if !defined _STARTBUTTON_INCLUDED
	#define _STARTBUTTON_INCLUDED
	
#include "common.h"

#define STARTBUTTON_PORT	(PINF)
#define STARTBUTTON_DDR		(DDRF)
#define STARTBUTTON_PIN		(3)

#define STARTBUTTON_TIMEOUT	(10)

void startbutton_init(void);
void startbutton_scan(void);
void do_startbutton(void);	
void startbutton_clean(void);
	
#endif /* _STARTBUTTON_INCLUDED */

#include <avr/io.h>
#include "common.h"
#include "menu.h"

menu_item_t menu_items[MENU_MODE_COUNT][MENU_ITEM_COUNT];

uint8_t	g_menu_mode = 0;
uint8_t	g_menu_item[MENU_MODE_COUNT];

void menu_init(void)
{
	int	mode, item;
	
	for (mode = 0; mode < MENU_MODE_COUNT; mode++)
	{
		g_menu_item[mode] = 0;
		for (item = 0; item < MENU_ITEM_COUNT; item++)
			menu_items[mode][item] = 0;
	}
}

void menu_mode_next(void)
{
	g_menu_mode++;

	if (MENU_MODE_COUNT == g_menu_mode)
		g_menu_mode = 0;
}

void menu_mode_prev(void)
{
	if (0 == g_menu_mode)
		g_menu_mode = MENU_MODE_COUNT;
		
	g_menu_mode--;
}

void menu_item_next(void)
{
	g_menu_item[g_menu_mode]++;
	
	if (MENU_ITEM_COUNT == g_menu_item[g_menu_mode])
		g_menu_item[g_menu_mode] = 0;
		
	if (0 == menu_items[g_menu_mode][g_menu_item[g_menu_mode]])
		g_menu_item[g_menu_mode] = 0;
}

void menu_item_prev(void)
{
	if (0 == g_menu_item[g_menu_mode])
		g_menu_item[g_menu_mode] = MENU_ITEM_COUNT;

	do 
	{
		g_menu_item[g_menu_mode]--;
	} 
	while (0 == menu_items[g_menu_mode][g_menu_item[g_menu_mode]] &&
		0 != g_menu_item[g_menu_mode]);
}

uint8_t menu_getmode(void)
{
	return g_menu_mode;
}

uint8_t menu_getitem(void)
{
	return g_menu_item[g_menu_mode];
}

void menu_doitem(void)
{
	(*menu_items[g_menu_mode][g_menu_item[g_menu_mode]])();
}

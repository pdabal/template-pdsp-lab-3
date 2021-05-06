/*
 * Controller.c
 *
 *  Created on: 22 kwi 2021
 *      Author: pdabal
 */

#include "main.h"
#include "pdsp/pdsp.h"

// 0-3 bits
#define MENU_PARAMETER_MASK 0x0F
#define MENU_PARAMETER_IS_NUMBER 1
#define MENU_PARAMETER_IS_STRING 2
// 4 - checkbox bit
#define MENU_ITEM_IS_CHECKBOX		0x10
// 5
#define MENU_ITEM_IS_CHECKED		0x20
// 6 - submenu bit
#define MENU_CALLBACK_IS_SUBMENU	0x40
// 7 - callback bit
#define MENU_CALLBACK_IS_FUNCTION 	0x80

// Number of items on one screen - not including title
#define MENU_LINES 1
// Symbol which is displayed in front of the selected item. This symbol doesn't appear when MENU_LINES == 1
#define ARROW_SYMBOL ">"
// How many spaces is between arrow symbol and menu item useful to set zero on smaller displays
#define ARROW_GAP 1

// Clear display
#define DisplayClear()					 BSP_LCD_Clear()
#define DisplayClearLine(posx)			{BSP_LCD_GoTo(posx, 1); BSP_LCD_WriteText((uint8_t *)"                ");}
// Display string
#define DisplayString(str, posx, posy) 	{BSP_LCD_GoTo(posx, posy); BSP_LCD_WriteText((uint8_t *)str);}
// Display number
#define DisplayNumber(num, posx, posy) 	{BSP_LCD_GoTo(posx, posy); BSP_LCD_WriteNumber(num); }

#define GEN_CONTROL_TYPE_NUM		4



GenControl_t GenControl = { 0 };

void callbackAmpl(void *m) {
	Menu_t *menu = ((Menu_t*) m);
	MenuItem_t *selectedItem = menu->items[menu->selectedIndex];

	selectedItem->parameter = (uint32_t) ((PDSP_CODEC_Vpp / 0.002f / BSP_POT_Bres) * BSP_POT_Read());

	GenControl.amplitude = (float) (selectedItem->parameter);
}
void callbackFreq(void *m) {
	Menu_t *menu = ((Menu_t*) m);
	MenuItem_t *selectedItem = menu->items[menu->selectedIndex];

	selectedItem->parameter = (uint32_t) ((PDSP_CODEC_Fs / 2.0f / BSP_POT_Bres) * BSP_POT_Read());

	GenControl.frequency = (float) (selectedItem->parameter);
}
void callbackType(void *m) {
	static uint32_t type = 0;

	Menu_t *menu = ((Menu_t*) m);
	MenuItem_t *selectedItem = menu->items[menu->selectedIndex];

	type++;
	if (type == GEN_CONTROL_TYPE_NUM)
		type = 0;

	switch (type) {
	case 0:
		selectedItem->parameter = (uint32_t) "SIN";
		break;
	case 1:
		selectedItem->parameter = (uint32_t) "TRIANGLE";
		break;
	case 2:
		selectedItem->parameter = (uint32_t) "SQUARE";
		break;
	case 3:
		selectedItem->parameter = (uint32_t) "SAWTOOTH";
		break;
	default:
		break;
	}

	GenControl.type = (uint16_t) (selectedItem->parameter);
}
void callbackOutput(void *m) {
	Menu_t *menu = ((Menu_t*) m);
	MenuItem_t *selectedItem = menu->items[menu->selectedIndex];

	if (selectedItem->parameter == (uint32_t) "Off") {
		selectedItem->parameter = (uint32_t) "On";
		GenControl.status = 1;
	} else {
		selectedItem->parameter = (uint32_t) "Off";
		GenControl.status = 0;
	}
}

MenuItem_t itemBaud = { (uint8_t*) "Baud.", NULL, MENU_PARAMETER_IS_NUMBER, 96000 };

MenuItem_t itemAmplitude = { (uint8_t*) "Ampl.", callbackAmpl, MENU_PARAMETER_IS_NUMBER | MENU_CALLBACK_IS_FUNCTION,
		1100 };
MenuItem_t itemFrequency = { (uint8_t*) "Freq.", callbackFreq, MENU_PARAMETER_IS_NUMBER | MENU_CALLBACK_IS_FUNCTION,
		1000 };
MenuItem_t itemType = { (uint8_t*) "Type", callbackType, MENU_PARAMETER_IS_STRING | MENU_CALLBACK_IS_FUNCTION,
		(uint32_t) "SIN" };
MenuItem_t itemOutput = { (uint8_t*) "Output", callbackOutput, MENU_PARAMETER_IS_STRING | MENU_CALLBACK_IS_FUNCTION,
		(uint32_t) "Off" };

//
Menu_t Menu = { (uint8_t*) "Menu generator", .items =
		{ &itemBaud, &itemAmplitude, &itemFrequency, &itemType, &itemOutput, 0 } };
Menu_t *hMenu = &Menu;

void MENU_Init(Menu_t *menu) {
	menu->menuItem = 0;
	menu->lastMenuItem = 255;
	menu->cursorTopPos = 0;
	menu->menuTopPos = 0;

	menu->len = 0;
	MenuItem_t **iList = menu->items;

	// Get number of items in menu, search for the first NULL
	for (; *iList != 0; ++iList)
		menu->len++;

	// Functional :) - menuItem, menuTopPos, cursorTopPos
	if (menu->selectedIndex != -1) {				// If item on the first screen
		if (menu->selectedIndex < MENU_LINES) {
			menu->menuItem = menu->selectedIndex;
			menu->cursorTopPos = menu->selectedIndex;
			menu->menuTopPos = 0;
		} else {									// Item is on other screen
			menu->menuItem = menu->selectedIndex;
			menu->cursorTopPos = MENU_LINES - 1;
			menu->menuTopPos = menu->selectedIndex - menu->cursorTopPos;
		}
	}

	menu->items[0]->parameter = PDSP_CODEC_BAUDRATE;
	DisplayClear();
	DisplayString(menu->title, 1, 1);
	MENU_Operation(menu, JOY_OK_EXTI_LINE);
}

void MENU_Value(Menu_t *menu) {
	uint16_t value = BSP_POT_Read();
	switch (menu->menuItem) {
	case 1:
		value = (uint16_t) ((PDSP_CODEC_Vpp / 0.002f / BSP_POT_Bres) * value);
		BSP_SEG_Display(value);
		break;
	case 2:
		value = (uint16_t) ((PDSP_CODEC_Fs / 2.0f / BSP_POT_Bres) * value);
		BSP_SEG_Display(value);
		break;
	default:
		BSP_SEG_Display(10000);
		break;
	}
}

__STATIC_INLINE void PressDown(Menu_t *menu) {
	if (menu->menuItem != menu->len - 1) {	// Move to next item
		menu->menuItem++;
	} else {	// Last item in menu => go to first item
		menu->menuItem = 0;
	}
}

__STATIC_INLINE void PressUp(Menu_t *menu) {
	if (menu->menuItem != 0) {
		menu->menuItem--;
	} else {					// go to the last item in menu
		menu->menuItem = menu->len - 1;
	}
}

__STATIC_INLINE void PressRight(Menu_t *menu) {
	menu->selectedIndex = menu->menuItem;

	int flags = menu->items[menu->selectedIndex]->flags;

	if (flags & MENU_ITEM_IS_CHECKBOX) {
		menu->items[menu->selectedIndex]->flags ^= MENU_ITEM_IS_CHECKED;
		menu->lastMenuItem = -1;
	}
	if (flags & MENU_CALLBACK_IS_FUNCTION && menu->items[menu->selectedIndex]->callback) {
		(*menu->items[menu->selectedIndex]->callback)(menu);
		menu->lastMenuItem = -1;
	}
}

__STATIC_INLINE void PressLeft(Menu_t *menu) {
	menu->selectedIndex = 0;
}

void MENU_Operation(Menu_t *menu, uint16_t GPIO_Pin) {

	switch (GPIO_Pin) {
	case JOY_LEFT_EXTI_LINE:
		PressLeft(menu);
		break;
	case JOY_DOWN_EXTI_LINE:
		PressDown(menu);
		break;
	case JOY_UP_EXTI_LINE:
		PressUp(menu);
		break;
	case JOY_RIGHT_EXTI_LINE:
	case JOY_OK_EXTI_LINE:
		PressRight(menu);
		break;
	default:
		break;
	}

	// If menu item changed -> refresh screen
	if (menu->lastMenuItem != menu->menuItem) {
		DisplayClearLine(2);
		DisplayString(menu->title, 1, 1);

		uint32_t index = menu->menuItem;

		// Print submenu
		DisplayString((menu->items[index]->text), 2, 1);

		int posx = strlen((char*) menu->items[index]->text) + 2;
		if ((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_NUMBER) {
			DisplayNumber(((int )menu->items[index]->parameter), 2, posx);
		}
		if ((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_STRING) {
			DisplayString((menu->items[index]->parameter), 2, posx);
		}
		if (menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX) {
			if (menu->items[index]->flags & MENU_ITEM_IS_CHECKED) {
				DisplayString("\xF6", 2, posx);
			} else {
				DisplayString("\xF7", 2, posx);
			}
		}
		menu->lastMenuItem = menu->menuItem;
	}
}


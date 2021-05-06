/*
 * Controller.h
 *
 *  Created on: 22 kwi 2021
 *      Author: pdabal
 */

#ifndef MIDDLEWARE_CONTROLLER_CONTROLLER_H_
#define MIDDLEWARE_CONTROLLER_CONTROLLER_H_

#include "Kameleon.h"

typedef struct {
	uint8_t *text;
	void (*callback)(void*);
	uint32_t flags;
	uint32_t parameter;
} MenuItem_t;

typedef struct {
	uint8_t *title;
	int32_t selectedIndex;
	int32_t refresh;
	int32_t len;
	uint8_t menuItem;
	uint8_t lastMenuItem;
	uint8_t cursorTopPos;
	uint8_t menuTopPos;
	MenuItem_t *items[];

} Menu_t;

typedef struct {
	float amplitude;
	float frequency;
	uint16_t type;
	uint16_t status;
} GenControl_t;

extern Menu_t * hMenu;
extern

void MENU_Init(Menu_t *menu);
void MENU_Operation(Menu_t *menu, uint16_t GPIO_Pin);
void MENU_Value(Menu_t *menu);

#endif /* MIDDLEWARE_CONTROLLER_CONTROLLER_H_ */

/*
 * hd44780.h
 *
 * HD44780 LCD display driver
 *      Author: pdaba
 */

#ifndef BSP_KAMELEON_INC_HD44780_H_
#define BSP_KAMELEON_INC_HD44780_H_

#define HD44780_CLEAR					 	0x01
#define HD44780_HOME						0x02

#define HD44780_ENTRY_MODE				  	0x04
#define HD44780_EM_SHIFT_CURSOR		 		0x00
#define HD44780_EM_SHIFT_DISPLAY	 		0x01
#define HD44780_EM_DECREMENT		   		0x00
#define HD44780_EM_INCREMENT		   		0x02

#define HD44780_DISPLAY_ONOFF			   	0x08
#define HD44780_DISPLAY_OFF		     		0x00
#define HD44780_DISPLAY_ON			   		0x04
#define HD44780_CURSOR_OFF			   		0x00
#define HD44780_CURSOR_ON			    	0x02
#define HD44780_CURSOR_NOBLINK	   			0x00
#define HD44780_CURSOR_BLINK		   		0x01

#define HD44780_DISPLAY_CURSOR_SHIFT 		0x10
#define HD44780_SHIFT_CURSOR		   		0x00
#define HD44780_SHIFT_DISPLAY		   		0x08
#define HD44780_SHIFT_LEFT			   		0x00
#define HD44780_SHIFT_RIGHT		     		0x04

#define HD44780_FUNCTION_SET			   	0x20
#define HD44780_FONT5x7				    	0x00
#define HD44780_FONT5x10			    	0x04
#define HD44780_ONE_LINE			    	0x00
#define HD44780_TWO_LINE			    	0x08
#define HD44780_4_BIT				    	0x00
#define HD44780_8_BIT				        0x10

#define HD44780_CGRAM_SET				    0x40
#define HD44780_DDRAM_SET				    0x80

/*
 * Select the entry mode.  inc determines whether the address counter
 * auto-increments, shift selects an automatic display shift.
 */
#define HD44780_ENTMODE(inc, shift)         		(0x04 | ((inc)? 0x02: 0) | ((shift)? 1: 0))

/*
 * Selects disp[lay] on/off, cursor on/off, cursor blink[ing]
 * on/off.
 */
#define HD44780_DISPCTL(disp, cursor, blink)    	(0x08 | ((disp)? 0x04: 0) | ((cursor)? 0x02: 0) | ((blink)? 1: 0))

/*
 * With shift = 1, shift display right or left.
 * With shift = 0, move cursor right or left.
 */
#define HD44780_SHIFT(shift, right)        			(0x10 | ((shift)? 0x08: 0) | ((right)? 0x04: 0))

/*
 * Function set.  if8bit selects an 8-bit data path, twoline arranges
 * for a two-line display, font5x10 selects the 5x10 dot font (5x8
 * dots if clear).
 */
#define HD44780_FNSET(if8bit, twoline, font5x10)	(0x20 | ((if8bit)? 0x10: 0) | ((twoline)? 0x08: 0) |        ((font5x10)? 0x04: 0))

/*
 * Set the next character generator address to addr.
 */
#define HD44780_CGADDR(addr)   						(0x40 | ((addr) & 0x3f))

/*
 * Set the next display address to addr.
 */
#define HD44780_DDADDR(addr)      					(0x80 | ((addr) & 0x7f))

#endif /* BSP_KAMELEON_INC_HD44780_H_ */

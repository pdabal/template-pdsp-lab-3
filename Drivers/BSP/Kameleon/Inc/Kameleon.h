/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __KAMELEON_H
#define __KAMELEON_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_lpuart.h"
#include "stm32l4xx_ll_usart.h"

#include "Kameleon_disp.h"
#include "Kameleon_led.h"
#include "Kameleon_sensor.h"
#include "Kameleon_motor.h"
//#include "Kameleon_audio.h"
//#include "Kameleon_mem.h"

//#include "Protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Exported Types ------------------------------------------------------------*/
typedef enum {
	JOY_NONE = 0, JOY_RIGHT = 1, JOY_LEFT = 2, JOY_DOWN = 4, JOY_UP = 8, JOY_SEL = (1 << 15)
} JOY_State_TypeDef;

typedef enum {
	JOY_MODE_GPIO = 0, JOY_MODE_EXTI = 1
} JOY_Mode_TypeDef;

/* Exported Constants --------------------------------------------------------*/
#if !defined (USE_KAMELEON)
#define USE_KAMELEON
#endif

//#define LED_RGB_MODE_PWM						1

// JOY
#define JOY_RIGHT_Pin 							GPIO_PIN_0
#define JOY_LEFT_Pin 							GPIO_PIN_1
#define JOY_DOWN_Pin 							GPIO_PIN_2
#define JOY_UP_Pin 								GPIO_PIN_3
#define JOY_OK_Pin 								GPIO_PIN_15
#define JOY_ALL_Pins                   		   (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_15)
#define JOY_NONE_Pins							0

#define JOY_GPIO_Port 							GPIOE

#define	JOY_ALL_MSK							   (JOY_DOWN_Pin | JOY_UP_Pin | JOY_OK_Pin | JOY_RIGHT_Pin | JOY_LEFT_Pin)


/* The macros below can be used to check if the respective switch is pressed.
 * They have the value of 1 as long as the switch is down. */
#define JOY_RIGHT_DOWN	(~JOY_GPIO_Port->IDR & JOY_RIGHT_Pin ? 1 : 0)
#define JOY_LEFT_DOWN 	(~JOY_GPIO_Port->IDR & JOY_LEFT_Pin ? 1 : 0)
#define JOY_DOWN_DOWN 	(~JOY_GPIO_Port->IDR & JOY_DOWN_Pin ? 1 : 0)
#define JOY_UP_DOWN 	(~JOY_GPIO_Port->IDR & JOY_UP_Pin ? 1 : 0)
#define JOY_OK_DOWN 	(~JOY_GPIO_Port->IDR & JOY_OK_Pin ? 1 : 0)

#define JOY_RIGHT_EXTI_LINE                    	GPIO_PIN_0
#define JOY_LEFT_EXTI_LINE                    	GPIO_PIN_1
#define JOY_DOWN_EXTI_LINE                    	GPIO_PIN_2
#define JOY_UP_EXTI_LINE                    	GPIO_PIN_3
#define JOY_OK_EXTI_LINE                    	GPIO_PIN_15

#define JOY_RIGHT_EXTI_IRQn                    	EXTI0_IRQn
#define JOY_LEFT_EXTI_IRQn                    	EXTI1_IRQn
#define JOY_DOWN_EXTI_IRQn                    	EXTI2_IRQn
#define JOY_UP_EXTI_IRQn                    	EXTI3_IRQn
#define JOY_UOK_EXTI_IRQn                    	EXTI15_10_IRQn

#define JOY_DEBOUNCE_TIME						(1000 / HAL_TICK_FREQ_DEFAULT) / 33

//extern BSP_CMD_DrvTypeDef						BSP_CMD_Driver;

void Error_HandlerData(char *file, uint32_t line);

/*-----------------------------------------------------------------------------*/
uint32_t BSP_GetVersion(void);
void BSP_InitAll(void);
/*-----------------------------------------------------------------------------*/
/* Initialize joystick.
 * The function should be called before using any other joystick function.
 * It initializes GPIO lines. */
void BSP_JOY_Init(JOY_Mode_TypeDef JoyMode);
/*
 *
 * */
void BSP_JOY_DeInit(JOY_Mode_TypeDef JoyMode);
/* Get the debounced state of the joystick switches.
 * The bitmasks defined above should be used to test the respective switches.
 * If the switch is pressed, the respective bit is 1, oterwise it is 0.
 * @retval The debounced state of the joystick switches. */
GPIO_PinState BSP_JOY_GetState(JOY_State_TypeDef JoyState);
/*
 *
 * */
GPIO_PinState BSP_JOY_GetState(JOY_State_TypeDef JoyState);
/*
 *
 * */
JOY_State_TypeDef BSP_JOY_GetCurrentState(void);
/*
 *
 * */
void BSP_JOY_Update(void);
/*
 *
 * */
uint32_t BSP_JOY_GetStateDebounced(void);
/*
 *
 * */
void BSP_JOY_Callback(uint16_t GPIO_Pin);
/*
 *
 * */
void JOY_StateUpdate(void);
/*-----------------------------------------------------------------------------*/
extern UART_HandleTypeDef hlpuart1;
void BSP_COM_Init(uint32_t baudrate);
void BSP_COM_DeInit(void);

/*-----------------------------------------------------------------------------*/
extern uint16_t AD_GPIO_PIN[];
extern GPIO_TypeDef *AD_GPIO_PORT[];

#define AD_Toggle(i)	LL_GPIO_TogglePin(AD_GPIO_PORT[i], AD_GPIO_PIN[i])
#define AD_On(i)		LL_GPIO_SetOutputPin(AD_GPIO_PORT[i], AD_GPIO_PIN[i])
#define AD_Off(i)		LL_GPIO_ResetOutputPin(AD_GPIO_PORT[i], AD_GPIO_PIN[i])

void BSP_AD_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __KAMELEON_H */

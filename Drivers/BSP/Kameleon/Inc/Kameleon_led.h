#ifndef __KAMELEON_LED_H_
#define __KAMELEON_LED_H_

/* Exported Types ------------------------------------------------------------*/
typedef enum {
	LED0 = 0, LED1 = 1, LED2 = 2, LED3 = 3, LED4 = 4, LED5 = 5, LED6 = 6, LED7 = 7
} LED_TypeDef;

typedef enum {
	LED_RGB0 = 0
} LED_RGB_TypeDef;

/* Exported Constants --------------------------------------------------------*/
#define LEDn                                    8

/*-----------------------------------------------------------------------------*/
void BSP_LED_Init(LED_TypeDef Led);
void BSP_LED_InitAll(void);
void BSP_LED_DeInit(LED_TypeDef Led);
void BSP_LED_On(LED_TypeDef Led);
void BSP_LED_Off(LED_TypeDef Led);
void BSP_LED_Toggle(LED_TypeDef Led);
void BSP_LED_On_All(uint8_t Led);
void BSP_LED_OffAll(uint8_t Led);
void BSP_LED_SetAll(uint8_t Led);
void BSP_LED_GetAll(uint8_t *Led);
void BSP_LED_Toggle_All(uint8_t Led);

/*-----------------------------------------------------------------------------*/
void BSP_LED_RGB_Init(void);
void BSP_LED_RGB_SetIntensity(uint16_t r, uint16_t g, uint16_t b);
void BSP_LED_RGB_GetIntensity(uint16_t *r, uint16_t *g, uint16_t *b);
#endif /* __KAMELEON_LED_H_ */

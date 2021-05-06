/* Includes ------------------------------------------------------------------*/
#include "Kameleon.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LED0_Pin 								GPIO_PIN_6
#define LED1_Pin 								GPIO_PIN_7
#define LED2_Pin 								GPIO_PIN_8
#define LED3_Pin 								GPIO_PIN_9
#define LED4_Pin 								GPIO_PIN_4
#define LED5_Pin 								GPIO_PIN_3
#define LED6_Pin 								GPIO_PIN_5
#define LED7_Pin 								GPIO_PIN_6

#define LED0_GPIO_Port 							GPIOC
#define LED1_GPIO_Port 							GPIOC
#define LED2_GPIO_Port 							GPIOC
#define LED3_GPIO_Port 							GPIOC
#define LED4_GPIO_Port 							GPIOE
#define LED5_GPIO_Port 							GPIOD
#define LED6_GPIO_Port 							GPIOE
#define LED7_GPIO_Port 							GPIOE

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define LED_GPIO_CLK_ENABLE(__INDEX__)     { __HAL_RCC_GPIOC_CLK_ENABLE(); __HAL_RCC_GPIOD_CLK_ENABLE(); __HAL_RCC_GPIOE_CLK_ENABLE(); }
#define LED_GPIO_CLK_DISABLE(__INDEX__)    { __HAL_RCC_GPIOC_CLK_DISABLE(); __HAL_RCC_GPIOD_CLK_DISABLE(); __HAL_RCC_GPIOE_CLK_DISABLE(); }

/* Private variables ---------------------------------------------------------*/
uint16_t LED_GPIO_PIN[LEDn] = { LED0_Pin, LED1_Pin, LED2_Pin, LED3_Pin, LED4_Pin, LED5_Pin, LED6_Pin, LED7_Pin };
GPIO_TypeDef *LED_GPIO_PORT[LEDn] = { LED0_GPIO_Port, LED1_GPIO_Port, LED2_GPIO_Port, LED3_GPIO_Port, LED4_GPIO_Port, LED5_GPIO_Port, LED6_GPIO_Port, LED7_GPIO_Port };

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/**
 * @brief  Configures LED GPIO.
 * @param  Led: Specifies the Led to be configured.
 */
void BSP_LED_Init(LED_TypeDef Led) {
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable the GPIO_LED Clock */
	LED_GPIO_CLK_ENABLE(Led);

	HAL_GPIO_WritePin(LED_GPIO_PORT[Led], LED_GPIO_PIN[Led], GPIO_PIN_RESET);

	/* Configure the GPIO_LED pin */
	GPIO_InitStruct.Pin = LED_GPIO_PIN[Led];
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(LED_GPIO_PORT[Led], &GPIO_InitStruct);
}

void BSP_LED_InitAll(void) {
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable the GPIO_LED Clock */
	LED_GPIO_CLK_ENABLE(Led);

	HAL_GPIO_WritePin(GPIOC, LED0_Pin | LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, LED5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, LED4_Pin | LED6_Pin | LED7_Pin, GPIO_PIN_RESET);

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	GPIO_InitStruct.Pin = LED0_Pin | LED1_Pin | LED2_Pin | LED3_Pin;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED5_Pin;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED4_Pin | LED6_Pin | LED7_Pin;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
 * @brief  DeInit LEDs.
 * @param  Led: LED to be de-init.
 * @note Led DeInit does not disable the GPIO clock nor disable the Mfx
 */
void BSP_LED_DeInit(LED_TypeDef Led) {
	GPIO_InitTypeDef gpio_init_structure;

	/* Turn off LED */
	HAL_GPIO_WritePin(LED_GPIO_PORT[Led], LED_GPIO_PIN[Led], GPIO_PIN_RESET);
	/* DeInit the GPIO_LED pin */
	gpio_init_structure.Pin = LED_GPIO_PIN[Led];
	HAL_GPIO_DeInit(LED_GPIO_PORT[Led], gpio_init_structure.Pin);
}

void BSP_LED_DeInitAll() {
	/* Turn off LED */
	HAL_GPIO_WritePin(GPIOC, LED0_Pin | LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, LED5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, LED4_Pin | LED6_Pin | LED7_Pin, GPIO_PIN_RESET);

	/* DeInit the GPIO_LED pin */
	HAL_GPIO_DeInit(GPIOC, LED0_Pin | LED1_Pin | LED2_Pin | LED3_Pin);
	HAL_GPIO_DeInit(GPIOD, LED5_Pin);
	HAL_GPIO_DeInit(GPIOE, LED4_Pin | LED6_Pin | LED7_Pin);
}

/**
 * @brief  Turns selected LED On.
 * @param  Led: Specifies the Led to be set on.
 */
void BSP_LED_On(LED_TypeDef Led) {
	LL_GPIO_SetOutputPin(LED_GPIO_PORT[Led], LED_GPIO_PIN[Led]);
}

/**
 * @brief  Turns selected LED Off.
 * @param  Led: Specifies the Led to be set off.
 */
void BSP_LED_Off(LED_TypeDef Led) {
	LL_GPIO_ResetOutputPin(LED_GPIO_PORT[Led], LED_GPIO_PIN[Led]);
}

/**
 * @brief  Toggles the selected LED.
 * @param  Led: Specifies the Led to be toggled.
 */
void BSP_LED_Toggle(LED_TypeDef Led) {
	LL_GPIO_TogglePin(LED_GPIO_PORT[Led], LED_GPIO_PIN[Led]);
}

void BSP_LED_OnAll(uint8_t Led) {
	LL_GPIO_SetOutputPin(GPIOC, ((0x0000000F & Led) << 6));  // LED0 .. LED3
	LL_GPIO_SetOutputPin(GPIOE, ((0x00000010 & Led) << 0));  // LED4
	LL_GPIO_SetOutputPin(GPIOD, ((0x00000020 & Led) >> 2));  // LED5
	LL_GPIO_SetOutputPin(GPIOE, ((0x000000C0 & Led) >> 1));  // LED4, LED6 .. LED7
}

void BSP_LED_OffAll(uint8_t Led) {
	LL_GPIO_ResetOutputPin(GPIOC, ((0x0000000F & Led) << 6));  // LED0 .. LED3
	LL_GPIO_ResetOutputPin(GPIOE, ((0x00000010 & Led) << 0));  // LED4
	LL_GPIO_ResetOutputPin(GPIOD, ((0x00000020 & Led) >> 2));  // LED5
	LL_GPIO_ResetOutputPin(GPIOE, ((0x000000C0 & Led) >> 1));  // LED4, LED6 .. LED7
}

void BSP_LED_SetAll(uint8_t Led) {
	for (uint8_t i = 0; i < LEDn; i++) {
		if ((Led & 1)) {
			LL_GPIO_SetOutputPin(LED_GPIO_PORT[i], LED_GPIO_PIN[i]);
		} else {
			LL_GPIO_ResetOutputPin(LED_GPIO_PORT[i], LED_GPIO_PIN[i]);
		}
		Led >>= 1;
	}
}

void BSP_LED_GetAll(uint8_t *Led) {
	for (uint8_t i = 0; i < LEDn; i++) {
		if (LL_GPIO_IsOutputPinSet(LED_GPIO_PORT[i], LED_GPIO_PIN[i])) {
			*Led |= (1 << i);
		}
	}
}

void BSP_LED_ToggleAll(uint8_t Led) {
	for (uint8_t i = 0; i < LEDn; i++) {
		if ((Led & 1)) {
			LL_GPIO_TogglePin(LED_GPIO_PORT[i], LED_GPIO_PIN[i]);
		}
		Led >>= 1;
	}
}

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LED_G_Pin 								GPIO_PIN_8
#define LED_B_Pin 								GPIO_PIN_12
#define LED_R_Pin 								GPIO_PIN_13

#define LED_G_GPIO_Port 						GPIOB
#define LED_B_GPIO_Port 						GPIOD
#define LED_R_GPIO_Port 						GPIOD

#define LED_RGBn                                3
#define LED_RGB_PERIOD							0x00FF
#define LED_RGB_PRESCALER						0
// LED RGB - TIM4 - PWM Mode

/* Private macro -------------------------------------------------------------*/
#define LED_RGB_CLK_ENABLE()				  { __HAL_RCC_GPIOB_CLK_ENABLE(); __HAL_RCC_GPIOD_CLK_ENABLE(); }
#define LED_RGB_CLK_DISABLE()				  { __HAL_RCC_GPIOB_CLK_DISABLE(); __HAL_RCC_GPIOD_CLK_DISABLE(); }

/* Private variables ---------------------------------------------------------*/
uint16_t LED_RGB_GPIO_PIN[LED_RGBn] = { LED_R_Pin, LED_G_Pin, LED_B_Pin };
GPIO_TypeDef *LED_RGB_GPIO_PORT[LED_RGBn] = { LED_R_GPIO_Port, LED_G_GPIO_Port, LED_B_GPIO_Port };

TIM_HandleTypeDef htim4;

/* Private function prototypes -----------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
void BSP_LED_RGB_Init(void) {
#ifdef LED_RGB_MODE_PWM
	/* Dioda RGB sterowana wyjściem licznika TIM4 w trybie PWM */
	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	htim4.Instance = TIM4;
	htim4.Init.Period = LED_RGB_PERIOD;
	htim4.Init.Prescaler = LED_RGB_PRESCALER;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	/* TIM4 GPIO Configuration: PD12 -> TIM4_CH1,  PD13 -> TIM4_CH2, PB8 -> TIM4_CH3 */
	// Clock enable
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	// GPIO configuration
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = LED_B_Pin | LED_R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED_G_Pin;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Start timer
	HAL_TIM_Base_Start(&htim4);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
#else
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin = LED_B_Pin | LED_R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LED_G_Pin;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
}

void BSP_LED_RGB_SetIntensity(uint16_t r, uint16_t g, uint16_t b) {
#ifdef LED_RGB_MODE_PWM
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, b);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, r);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, g);
#else
	if (r > 0)
		LL_GPIO_SetOutputPin(GPIOD, LED_R_Pin);
	else
		LL_GPIO_ResetOutputPin(GPIOD, LED_R_Pin);
	if (b > 0)
		LL_GPIO_SetOutputPin(GPIOD, LED_B_Pin);
	else
		LL_GPIO_ResetOutputPin(GPIOD, LED_B_Pin);
	if (g > 0)
		LL_GPIO_SetOutputPin(GPIOB, LED_G_Pin);
	else
		LL_GPIO_ResetOutputPin(GPIOB, LED_G_Pin);
#endif
}

void BSP_LED_RGB_GetIntensity(uint16_t *r, uint16_t *g, uint16_t *b) {
#ifdef LED_RGB_MODE_PWM
	*b = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_1);
	*r = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_2);
	*g = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_3);
#else
	*r = LL_GPIO_IsOutputPinSet(GPIOD, LED_R_Pin);
	*b = LL_GPIO_IsOutputPinSet(GPIOD, LED_B_Pin);
	*g = LL_GPIO_IsOutputPinSet(GPIOB, LED_G_Pin);
#endif
}

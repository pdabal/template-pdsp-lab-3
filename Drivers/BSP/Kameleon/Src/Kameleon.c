/* Includes ------------------------------------------------------------------*/
#include "Kameleon.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define __KAMELEON_BSP_VERSION_MAIN   (0x01) /*!< [31:24] main version */
#define __KAMELEON_BSP_VERSION_SUB1   (0x00) /*!< [23:16] sub1 version */
#define __KAMELEON_BSP_VERSION_SUB2   (0x03) /*!< [15:8]  sub2 version */
#define __KAMELEON_BSP_VERSION_RC     (0x00) /*!< [7:0]  release candidate */
#define __KAMELEON_BSP_VERSION()      ((__KAMELEON_BSP_VERSION_MAIN << 24)\
                                             |(__KAMELEON_BSP_VERSION_SUB1 << 16)\
                                             |(__KAMELEON_BSP_VERSION_SUB2 << 8 )\
                                             |(__KAMELEON_BSP_VERSION_RC))

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#define JOYn									5

#define JOY_DOWN_Pin GPIO_PIN_2
#define JOY_DOWN_GPIO_Port GPIOE
#define JOY_UP_Pin GPIO_PIN_3
#define JOY_UP_GPIO_Port GPIOE

#define JOY_OK_Pin GPIO_PIN_15
#define JOY_OK_GPIO_Port GPIOE

#define JOY_RIGHT_Pin GPIO_PIN_0
#define JOY_RIGHT_GPIO_Port GPIOE
#define JOY_LEFT_Pin GPIO_PIN_1
#define JOY_LEFT_GPIO_Port GPIOE

#define JOY_CLK_ENABLE()				      __HAL_RCC_GPIOE_CLK_ENABLE()
#define JOY_CLK_DISABLE()				   	  __HAL_RCC_GPIOE_CLK_DISABLE()

uint32_t joyState = 0;
JOY_Mode_TypeDef joyMode = JOY_MODE_GPIO;
const IRQn_Type JOY_IRQn[JOYn] = { JOY_RIGHT_EXTI_IRQn, JOY_LEFT_EXTI_IRQn, JOY_DOWN_EXTI_IRQn, JOY_UP_EXTI_IRQn,
JOY_UOK_EXTI_IRQn };
uint16_t JOY_GPIO_PIN[JOYn] = { JOY_RIGHT_Pin, JOY_LEFT_Pin, JOY_DOWN_Pin, JOY_UP_Pin, JOY_OK_Pin };

uint16_t AD_GPIO_PIN[] = {
GPIO_PIN_8, GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_10, GPIO_PIN_14, GPIO_PIN_15, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14,
GPIO_PIN_3, GPIO_PIN_2, 0, 0, GPIO_PIN_3, GPIO_PIN_13, GPIO_PIN_12 };
GPIO_TypeDef *AD_GPIO_PORT[] = {
GPIOA, GPIOC, GPIOC, GPIOB, GPIOB, GPIOB, GPIOB, GPIOG, GPIOG, GPIOA, GPIOA, NULL, NULL, GPIOB, GPIOD, GPIOD };

#define vCOM				LPUART1
#define vCOM_RX_Pin 		GPIO_PIN_0
#define vCOM_RX_GPIO_Port 	GPIOC
#define vCOM_TX_Pin 		GPIO_PIN_1
#define vCOM_TX_GPIO_Port 	GPIOC
#define vCOM_IRQ_Prioryty	0
#define vCOM_Baudrate		115200

UART_HandleTypeDef hlpuart1;
/*----------------------------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Common --------------------------------------------------------------------*/
/**
 * @brief  This method returns the KAMELEON BSP Driver revision
 * @retval version: 0xXYZR (8bits for each decimal, R for RC)
 */
uint32_t BSP_GetVersion(void) {
	return __KAMELEON_BSP_VERSION();
}

void BSP_InitAll(void) {
	BSP_AD_Init();

	BSP_COM_Init(vCOM_Baudrate);
	BSP_JOY_Init(JOY_MODE_EXTI);
	BSP_LCD_Init();
	BSP_LED_InitAll();
	BSP_LED_RGB_Init();
	BSP_POT_Init();
	BSP_SEG_Init();
	BSP_MEMS_Init(MEMS_ACC_DATARATE_10HZ, MEMS_ACC_FULLSCALE_2G);
}

/* JOY -----------------------------------------------------------------------*/

/**
 * @brief  Configures JOY GPIO and EXTI Line.
 * @param  Button: Specifies the Button to be configured.
 * @param  ButtonMode: Specifies Button mode.
 *   This parameter can be one of following parameters:
 *     @arg JOY_MODE_EXTI: Button will be connected to EXTI line with interrupt generation capability
 */
void BSP_JOY_Init(JOY_Mode_TypeDef JoyMode) {
	/* Enable the JOY Clock */
	JOY_CLK_ENABLE();

	joyMode = JoyMode;

	GPIO_InitTypeDef GPIO_InitStruct;
	for (int i = 0; i < JOYn; ++i) {
		GPIO_InitStruct.Pin = JOY_GPIO_PIN[i];
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		if (joyMode == JOY_MODE_GPIO) {
			/* Configure Button pin as input */
			GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
			HAL_GPIO_Init(JOY_GPIO_Port, &GPIO_InitStruct);
		}
		if (joyMode == JOY_MODE_EXTI) {
			/* Configure Button pin as input with External interrupt */
			GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
			HAL_GPIO_Init(JOY_GPIO_Port, &GPIO_InitStruct);
			/* Enable and set Button EXTI Interrupt to the lowest priority */
			HAL_NVIC_SetPriority((IRQn_Type) (JOY_IRQn[i]), 0x0F, 0x00);
			HAL_NVIC_EnableIRQ((IRQn_Type) (JOY_IRQn[i]));
		}
	}
}

/**
 * @brief  Push Button DeInit.
 * @param  Button: Button to be configured
 *   This parameter should be: BUTTON_USER
 * @note PB DeInit does not disable the GPIO clock
 */
void BSP_JOY_DeInit(JOY_Mode_TypeDef JoyMode) {
	GPIO_InitTypeDef GPIO_InitStruct;

	for (int i = 0; i < JOYn; ++i) {
		GPIO_InitStruct.Pin = JOY_GPIO_PIN[i];
		if (JoyMode == JOY_MODE_EXTI)
			HAL_NVIC_DisableIRQ((JOY_IRQn[i]));
		HAL_GPIO_DeInit(JOY_GPIO_Port, GPIO_InitStruct.Pin);
	}
}

/**
 * @brief  Returns the selected Button state.
 * @param  Button: Specifies the Button to be checked.
 *   This parameter should be: BUTTON_USER
 * @retval The Button GPIO pin value.
 */
GPIO_PinState BSP_JOY_GetState(JOY_State_TypeDef JoyState) {
	return HAL_GPIO_ReadPin(JOY_GPIO_Port, JOY_GPIO_PIN[JoyState]);
}

/**
 * @brief  Returns the current joystick status.
 * @retval Code of the joystick key pressed
 *          This code can be one of the following values:
 *            @arg  JOY_NONE
 *            @arg  JOY_SEL
 *            @arg  JOY_DOWN
 *            @arg  JOY_LEFT
 *            @arg  JOY_RIGHT
 *            @arg  JOY_UP
 */
JOY_State_TypeDef BSP_JOY_GetCurrentState(void) {
	uint32_t pin_status = 0;

	/* Read the status joystick pins */
	pin_status = BSP_JOY_GetStateDebounced();

	/* Check the pressed keys */
	if ((pin_status & JOY_ALL_Pins) == JOY_ALL_Pins) {
		return JOY_NONE;
	} else if ((pin_status & JOY_OK_Pin)) {
		return JOY_SEL;
	} else if ((pin_status & JOY_DOWN_Pin)) {
		return JOY_DOWN;
	} else if ((pin_status & JOY_LEFT_Pin)) {
		return JOY_LEFT;
	} else if ((pin_status & JOY_RIGHT_Pin)) {
		return JOY_RIGHT;
	} else if ((pin_status & JOY_UP_Pin)) {
		return JOY_UP;
	} else {
		return JOY_NONE;
	}
}

uint32_t BSP_JOY_GetStateDebounced(void) {
	return joyState;
}

void JOY_StateUpdate(void) {
	uint32_t static delay = JOY_DEBOUNCE_TIME;

	if (--delay == 0) {
		delay = JOY_DEBOUNCE_TIME;
		joyState = ((~JOY_GPIO_Port->IDR) & JOY_ALL_MSK);
	}
}

__weak void BSP_JOY_Callback(uint16_t GPIO_Pin) {
	UNUSED(GPIO_Pin);
}

/*----------------------------------------------------------------------------*/
void BSP_COM_Init(uint32_t baudrate) {
	__HAL_RCC_GPIOC_CLK_ENABLE();
	/**LPUART1 GPIO Configuration: PC0     ------> LPUART1_RX, PC1     ------> LPUART1_TX	 */
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = vCOM_TX_Pin | vCOM_RX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Peripheral clock enable */
	__HAL_RCC_LPUART1_CLK_ENABLE();

	hlpuart1.Instance = vCOM;
	hlpuart1.Init.BaudRate = baudrate;
	hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
	hlpuart1.Init.StopBits = UART_STOPBITS_1;
	hlpuart1.Init.Parity = UART_PARITY_NONE;
	hlpuart1.Init.Mode = UART_MODE_TX_RX;
	hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&hlpuart1) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	/* LPUART1 interrupt Init */
	HAL_NVIC_SetPriority(LPUART1_IRQn, vCOM_IRQ_Prioryty, 0);
	HAL_NVIC_EnableIRQ(LPUART1_IRQn);
}

void BSP_COM_DeInit(void) {
	HAL_UART_DeInit(&hlpuart1);
	/* Peripheral clock disable */
	__HAL_RCC_LPUART1_CLK_DISABLE();
	/* LPUART1 GPIO Configuration: PC0     ------> LPUART1_RX,  PC1     ------> LPUART1_TX	 */
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_1);
	/* LPUART1 interrupt DeInit */
	HAL_NVIC_DisableIRQ(LPUART1_IRQn);
}

/*----------------------------------------------------------------------------*/
void BSP_AD_Init(void) {
	uint32_t i;
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_PWREx_EnableVddIO2();

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	for (i = 0; i < 16; i++) {
		if (AD_GPIO_PORT[i] != NULL) {
			GPIO_InitStruct.Pin = AD_GPIO_PIN[i];
			HAL_GPIO_Init(AD_GPIO_PORT[i], &GPIO_InitStruct);
			LL_GPIO_ResetOutputPin(AD_GPIO_PORT[i], AD_GPIO_PIN[i]);
		}
	}
}

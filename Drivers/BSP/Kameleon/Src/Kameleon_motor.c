/* Includes ------------------------------------------------------------------*/
#include "Kameleon.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MOTOR_PHASE_Pin 					GPIO_PIN_10
#define MOTOR_ENABLE_Pin 					GPIO_PIN_11
#define MOTOR_FAULT_Pin 					GPIO_PIN_12
#define MOTOR_Port 							GPIOC

#define MOTOR_TIM_Prescaler					HAL_RCC_GetHCLKFreq() / 10000 / 120 - 1
#define MOTOR_TIM_Period					120 - 1
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

void BSP_MOTOR_Init(void) {
	{ //	MX_TIM1_Init
		TIM_ClockConfigTypeDef sClockSourceConfig;
		TIM_MasterConfigTypeDef sMasterConfig;
		TIM_OC_InitTypeDef sConfigOC;
		TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

		htim1.Instance = TIM1;
		htim1.Init.Prescaler = MOTOR_TIM_Prescaler;
		htim1.Init.Period = MOTOR_TIM_Period;
		htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim1.Init.RepetitionCounter = 0;
		if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);

		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);

		if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);

		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);

		sConfigOC.Pulse = 0;
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
		sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
		if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);

		sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
		sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
		sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
		sBreakDeadTimeConfig.DeadTime = 0;
		sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
		sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
		sBreakDeadTimeConfig.BreakFilter = 0;
		sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
		sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
		sBreakDeadTimeConfig.Break2Filter = 0;
		sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
		if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);
	}

	LL_GPIO_ResetOutputPin(MOTOR_Port, MOTOR_PHASE_Pin | MOTOR_ENABLE_Pin);

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = MOTOR_PHASE_Pin | MOTOR_ENABLE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(MOTOR_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = MOTOR_FAULT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(MOTOR_Port, &GPIO_InitStruct);

	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

void BSP_MOTOR_On(void) {
	LL_GPIO_SetOutputPin(MOTOR_Port, MOTOR_ENABLE_Pin);
}

void BSP_MOTOR_Off(void) {
	LL_GPIO_ResetOutputPin(MOTOR_Port, MOTOR_ENABLE_Pin);
}

void BSP_MOTOR_SetDir(uint8_t dir) {
	if (dir == MOTOR_DIR_CLOCKWISE)
		LL_GPIO_ResetOutputPin(MOTOR_Port, MOTOR_ENABLE_Pin);
	else if (dir == MOTOR_DIR_COUNTERCLOCKWISE)
		LL_GPIO_SetOutputPin(MOTOR_Port, MOTOR_ENABLE_Pin);
}

void BSP_MOTOR_SetSpeed(uint8_t speed) {
	if (speed > 100)
		speed = 100;

	TIM_OC_InitTypeDef sConfigOC;
	sConfigOC.Pulse = speed;
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}


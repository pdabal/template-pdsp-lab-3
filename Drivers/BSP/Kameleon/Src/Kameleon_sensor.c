#include "Kameleon.h"

#define SENSOR_ACC_ADDR 				0x1D
#define SENSOR_MAG_ADDR 				0x1D

#define SENSOR_RDY_MAG_Pin 				GPIO_PIN_10
#define SENSOR_RDY_MAG_GPIO_Port 		GPIOG
#define SENSOR_RDY_MAG_EXTI_IRQn 		EXTI15_10_IRQn
#define SENSOR_INT_AXL_Pin 				GPIO_PIN_11
#define SENSOR_INT_AXL_GPIO_Port 		GPIOG
#define SENSOR_INT_AXL_EXTI_IRQn 		EXTI15_10_IRQn
#define SENSOR_INT_MAG_Pin 				GPIO_PIN_12
#define SENSOR_INT_MAG_GPIO_Port 		GPIOG
#define SENSOR_INT_MAG_EXTI_IRQn 		EXTI15_10_IRQn

#define MEMS_I2C3_SCL_Pin GPIO_PIN_7
#define MEMS_I2C3_SCL_GPIO_Port GPIOG
#define MEMS_I2C3_SDA_Pin GPIO_PIN_8
#define MEMS_I2C3_SDA_GPIO_Port GPIOG

#define MEMS_ACC_AXIS_X				1
#define MEMS_ACC_AXIS_Y				2
#define MEMS_ACC_AXIS_Z				4
#define MEMS_ACC_AXIS_XYZ			8

#define	MEMS_ACC_ID_REG		0xf
#define	MEMS_ACC_ID			0x41
#define	MEMS_ACC_CTRL1_REG	0x20
#define	MEMS_ACC_CTRL4_REG	0x23
#define	MEMS_ACC_X_REG		0x28
#define	MEMS_ACC_Y_REG		0x2a
#define	MEMS_ACC_Z_REG		0x2c

#define	MEMS_MAG_ID_REG		0xf
#define	MEMS_MAG_ID     	0x3d
#define	MEMS_MAG_CTRL1_REG	0x20
#define	MEMS_MAG_CTRL2_REG	0x21
#define	MEMS_MAG_CTRL3_REG	0x22
#define	MEMS_MAG_CTRL4_REG	0x23
#define	MEMS_MAG_X_REG		0x28
#define	MEMS_MAG_Y_REG		0x2a
#define	MEMS_MAG_Z_REG		0x2c

#define	I2C_MRW				0x80	// multiple read/write


I2C_HandleTypeDef hi2c3;

static void MEMS_IO_Init(void){
	RCC_PeriphCLKInitTypeDef PeriphClkInit;
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
	PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	__HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_PWREx_EnableVddIO2();

	GPIO_InitTypeDef GPIO_InitStruct;
	/* I2C3 GPIO Configuration: PG7 -> I2C3_SCL, PG8 -> I2C3_SDA */
	GPIO_InitStruct.Pin = MEMS_I2C3_SCL_Pin | MEMS_I2C3_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* Peripheral clock enable */
	__HAL_RCC_I2C3_CLK_ENABLE();

	hi2c3.Instance = I2C3;
	hi2c3.Init.Timing = 0x10909CEC;
	hi2c3.Init.OwnAddress1 = 0;
	hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c3.Init.OwnAddress2 = 0;
	hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c3) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	/* Configure Analogue filter */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	/* Configure Digital filter */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
}

/**
 * @brief  Configures COMPASS/ACCELEROMETER io interface.
 * @retval None
 */
void ACCELERO_IO_Init(void) {
	MEMS_IO_Init();

	__HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_PWREx_EnableVddIO2();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = SENSOR_INT_AXL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SENSOR_INT_AXL_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief  De-Configures COMPASS/ACCELEROMETER io interface.
 * @retval None
 */
void ACCELERO_IO_DeInit(void) {


	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = SENSOR_INT_AXL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SENSOR_INT_AXL_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief  Writes one byte to the COMPASS / ACCELEROMETER.
 * @param  RegisterAddr specifies the COMPASS / ACCELEROMETER register to be written.
 * @param  Value : Data to be written
 * @retval   None
 */
void ACCELERO_IO_Write(uint8_t RegisterAddr, uint8_t Value) {
	if (HAL_I2C_Mem_Write(&hi2c3, SENSOR_ACC_ADDR << 1, RegisterAddr, 1, &Value, 1, 1000) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
}

/**
 * @brief  Reads a block of data from the COMPASS / ACCELEROMETER.
 * @param  RegisterAddr : specifies the COMPASS / ACCELEROMETER internal address register to read from
 * @retval ACCELEROMETER register value
 */
uint8_t ACCELERO_IO_Read(uint8_t RegisterAddr) {
	uint8_t val;

	if (HAL_I2C_Mem_Read(&hi2c3, SENSOR_ACC_ADDR << 1, RegisterAddr, 1, &val, 1, 1000) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	return val;
}

/********************************* LINK MAGNETO *******************************/
/**
 * @brief  Configures COMPASS/MAGNETO SPI interface.
 * @retval None
 */
void MAGNETO_IO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_PWREx_EnableVddIO2();

	GPIO_InitStruct.Pin = SENSOR_INT_MAG_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SENSOR_RDY_MAG_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SENSOR_RDY_MAG_GPIO_Port, &GPIO_InitStruct);

	MEMS_IO_Init();
}

/**
 * @brief  de-Configures COMPASS/MAGNETO SPI interface.
 * @retval None
 */
void MAGNETO_IO_DeInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = SENSOR_INT_MAG_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SENSOR_RDY_MAG_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SENSOR_RDY_MAG_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief  Writes one byte to the COMPASS/MAGNETO.
 * @param  RegisterAddr specifies the COMPASS/MAGNETO register to be written.
 * @param  Value : Data to be written
 * @retval   None
 */
void MAGNETO_IO_Write(uint8_t RegisterAddr, uint8_t Value) {
	if (HAL_I2C_Mem_Write(&hi2c3, SENSOR_MAG_ADDR << 1, RegisterAddr, 1, &Value, 1, 1000) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
}

/**
 * @brief  Reads a block of data from the COMPASS/MAGNETO.
 * @param  RegisterAddr : specifies the COMPASS/MAGNETO internal address register to read from
 * @retval ACCELEROMETER register value
 */
uint8_t MAGNETO_IO_Read(uint8_t RegisterAddr) {
	uint8_t val;

	if (HAL_I2C_Mem_Read(&hi2c3, SENSOR_MAG_ADDR << 1, RegisterAddr, 1, &val, 1, 1000) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	return val;
}
/*----------------------------------------------------------------------*/




uint8_t BSP_MEMS_Init(uint8_t acc_datarate, uint8_t acc_fullscale) {
	Lsm303cDrv_accelero.Init(0);

	uint8_t acc_axis = 7;  // all axis
	uint8_t r[4];

	if (HAL_I2C_Mem_Read(&hi2c3, SENSOR_ACC_ADDR << 1, MEMS_ACC_ID_REG, 1, r, 1, 1000) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	if (r[0] != MEMS_ACC_ID)
		return 1;

	if (acc_axis == MEMS_ACC_AXIS_XYZ)
		acc_axis = MEMS_ACC_AXIS_X | MEMS_ACC_AXIS_Y | MEMS_ACC_AXIS_Z;  // each axis is enabled individually
	if (acc_datarate == 0 || acc_datarate >= 7 || acc_axis > 7 || (acc_fullscale & 0xfc) != 0 || acc_fullscale == 1)
		return 1;

	r[0] = (acc_datarate << 4) | acc_axis;
	if (HAL_I2C_Mem_Write(&hi2c3, SENSOR_ACC_ADDR << 1, MEMS_ACC_CTRL1_REG, 1, r, 1, 1000) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	r[0] = (acc_fullscale << 4) | 4;  // set fullscale, turn address autoincrement on
	if (HAL_I2C_Mem_Write(&hi2c3, SENSOR_ACC_ADDR << 1, MEMS_ACC_CTRL4_REG, 1, r, 1, 1000) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	return 0;
}

void BSP_MEMS_ACC_GetValue(int16_t * value){
	Lsm303cDrv_accelero.GetXYZ(value);
}


/*----------------------------------------------------------------------------------*/
#define TEMP_LM75_I2C1_SDA_Pin 				GPIO_PIN_13
#define TEMP_LM75_I2C1_SDA_GPIO_Port 		GPIOG
#define TEMP_LM75_I2C1_SCL_Pin 				GPIO_PIN_14
#define TEMP_LM75_I2C1_SCL_GPIO_Port 		GPIOG

#define TEMP_LM75_INT_Pin 					GPIO_PIN_15
#define TEMP_LM75_INT_GPIO_Port 			GPIOG
#define TEMP_LM75_INT_EXTI_IRQn 			EXTI15_10_IRQn

#define TEMP_LM75_TEMP_REG	0
#define TEMP_LM75_CONF_REG	1
#define TEMP_LM75_THYS_REG	2
#define TEMP_LM75_TOS_REG	3

#define	TEMP_LM75_ADDR		0x48

I2C_HandleTypeDef hi2c1;

void BSP_TEMP_LM75_Init(void) {
	RCC_PeriphCLKInitTypeDef PeriphClkInit;
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_PWREx_EnableVddIO2();

	GPIO_InitTypeDef GPIO_InitStruct;
	/*Configure GPIO pin : TEMP_LM75_INT_Pin */
	GPIO_InitStruct.Pin = TEMP_LM75_INT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(TEMP_LM75_INT_GPIO_Port, &GPIO_InitStruct);

	/**I2C1 GPIO Configuration: PG13 -> I2C1_SDA, PG14 -> I2C1_SCL */
	GPIO_InitStruct.Pin = TEMP_LM75_I2C1_SDA_Pin | TEMP_LM75_I2C1_SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* Peripheral clock enable */
	__HAL_RCC_I2C1_CLK_ENABLE();

	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x10909CEC;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	/**Configure Analogue filter */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);
	/*Configure Digital filter */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

}

int8_t BSP_TEMP_LM75_Read(void) {
	uint8_t temp;
	if (HAL_I2C_Mem_Read(&hi2c1, TEMP_LM75_ADDR << 1, TEMP_LM75_TEMP_REG, 1, (uint8_t*) &temp, 1, 1000) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	return temp;
}

/*----------------------------------------------------------------------------------*/
#define POT_Pin 		GPIO_PIN_2
#define POT_GPIO_Port 	GPIOC

ADC_HandleTypeDef hadc3;

void BSP_POT_Init(void) {
	// Configure clock for ADC
	RCC_PeriphCLKInitTypeDef PeriphClkInit;
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	__HAL_RCC_ADC_CLK_ENABLE();

	// Configure GPIO for ADC
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	/* ADC3 GPIO Configuration PC2     ------> ADC3_IN3 */
	GPIO_InitStruct.Pin = POT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(POT_GPIO_Port, &GPIO_InitStruct);

	{
		ADC_ChannelConfTypeDef sConfig;
		/* Common config 	 */
		hadc3.Instance = ADC3;
		hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
		hadc3.Init.Resolution = ADC_RESOLUTION_12B;
		hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
		hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
		hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
		hadc3.Init.LowPowerAutoWait = DISABLE;
		hadc3.Init.ContinuousConvMode = DISABLE;
		hadc3.Init.NbrOfConversion = 1;
		hadc3.Init.DiscontinuousConvMode = DISABLE;
		hadc3.Init.NbrOfDiscConversion = 1;
		hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T6_TRGO;
		hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
		hadc3.Init.DMAContinuousRequests = DISABLE;
		hadc3.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
		hadc3.Init.OversamplingMode = ENABLE;
		hadc3.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_256;
		hadc3.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_8;
		hadc3.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_MULTI_TRIGGER;
		hadc3.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
		if (HAL_ADC_Init(&hadc3) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);
		if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);

		/* Configure Regular Channel 	 */
		sConfig.Channel = ADC_CHANNEL_3;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
		sConfig.SingleDiff = ADC_SINGLE_ENDED;
		sConfig.OffsetNumber = ADC_OFFSET_NONE;
		sConfig.Offset = 0;
		if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);
		if (HAL_ADC_Start(&hadc3) != HAL_OK)
			Error_HandlerData(__FILE__, __LINE__);
	}
}

uint16_t BSP_POT_Read(void) {
	return HAL_ADC_GetValue(&hadc3);
}


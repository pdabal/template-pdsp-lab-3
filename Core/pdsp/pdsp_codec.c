/* Includes ------------------------------------------------------------------*/
#include "pdsp.h"
#include "main.h"
#include "Kameleon.h"
#include "stm32l4xx_ll_dac.h"
#include "stm32l4xx_ll_lpuart.h"

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DAC_HandleTypeDef hdac1;
UART_HandleTypeDef hlpuart1;
TIM_HandleTypeDef htim6;
DMA_HandleTypeDef hdma_lpuart_rx;
DMA_HandleTypeDef hdma_lpuart_tx;

uint32_t CODEC_CNT[] = { 80, 40, 26, 20, 10, 25, 10 };
uint32_t CODEC_PSC[] = { 125, 125, 125, 125, 175, 65, 75 };
uint32_t CODEC_FS[] = { 8000, 16000, 24000, 32000, 44000, 48000, 96000 };

/* Private user code ---------------------------------------------------------*/
void CODEC_IRQHandler(void);

__STATIC_INLINE void ADC_ReceiveData(ADC_TypeDef *ADCx) {
	if (PDSP_SAMPLE_SIZE == 1) {
		valueUartBuffRx[LEFT] = LL_ADC_REG_ReadConversionData8(ADCx);
		if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
			valueUartBuffRx[RIGHT] = valueUartBuffRx[LEFT];
	} else {
		pValueUartBuffRx[LEFT] = LL_ADC_REG_ReadConversionData12(ADCx);
		if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
			pValueUartBuffRx[RIGHT] = pValueUartBuffRx[LEFT];
	}
	dataRx = true;
}

__STATIC_INLINE void UART_ReceiveData(void) {
	HAL_UART_Receive_DMA(&hlpuart1, (uint8_t*) valueUartBuffRx, PDSP_CODEC_BUFFOR_LENGTH);
}

__STATIC_INLINE void DAC_TransmitData8(void) {
	if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_MONO)
		LL_DAC_ConvertData8RightAligned(DAC1, LL_DAC_CHANNEL_1, (valueUartBuffTx[LEFT] + (uint8_t) PDSP_CODEC_OFFSET_ADC));

	else
		LL_DAC_ConvertData8RightAligned(DAC1, LL_DAC_CHANNEL_1,
				((valueUartBuffTx[LEFT] + valueUartBuffTx[RIGHT]) >> 1) + (uint8_t) PDSP_CODEC_OFFSET_ADC);

	dataTx = false;
}

__STATIC_INLINE void DAC_TransmitData16(void) {
	if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_MONO)
		LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1,
				(pValueUartBuffTx[LEFT] + (uint16_t) PDSP_CODEC_OFFSET_ADC));
	else
		LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1,
				((pValueUartBuffTx[LEFT] + pValueUartBuffTx[RIGHT]) >> 1) + (uint16_t) PDSP_CODEC_OFFSET_ADC);

	dataTx = false;
}

__STATIC_INLINE void UART_TransmitData(int8_t value) {
	while (!LL_LPUART_IsActiveFlag_TXE(LPUART1))
		;
	LL_LPUART_TransmitData8(LPUART1, value);
}

__STATIC_INLINE void UART_TransmitData8(void) {
	UART_TransmitData(valueUartBuffTx[LEFT]);
	if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
		UART_TransmitData(valueUartBuffTx[RIGHT]);

	dataTx = false;
}

__STATIC_INLINE void UART_TransmitData16(void) {
	if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_MONO) {
		UART_TransmitData(valueUartBuffTx[0]);
		UART_TransmitData(valueUartBuffTx[1]);
	} else {
		HAL_UART_Transmit_DMA(&hlpuart1, (uint8_t*) valueUartBuffTx, PDSP_CODEC_BUFFOR_LENGTH);
	}
	dataTx = false;
}
/*---------------------------------------------------------------------------
 * Funkcje konfigurujące emulacje kodeka - TIM6, LPUART1, ADC1, DAC1
 *-------------------------------------------------------------------------*/

// Emulacja za pomocą licznika przerwania od kodeka audio z częstotliwością Fs
__STATIC_INLINE void TIM6_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// Odbieranie danych z UART via DMA - niezależne od liczby kanałów i długości - potwierdzenie w CallbackRx
	if ((PDSP_INPUT & PDSP_IN_UART) == PDSP_IN_UART)
		UART_ReceiveData();
	// Odbieranie danych z ADC - dla stereo skopiowanie do drugiego kanału, uwzględnienie składowej stałej
	if ((PDSP_INPUT & PDSP_IN_ADC_MIC) == PDSP_IN_ADC_MIC)
		ADC_ReceiveData(ADC1);

	/* Wysyłanie danych -----------------------------------------------------------------------------------------*/
	if ((PDSP_OUTPUT & PDSP_OUT_UART) == PDSP_OUT_UART)
		(PDSP_SAMPLE_SIZE == 1) ? UART_TransmitData8() : UART_TransmitData16();
	if ((PDSP_OUTPUT & PDSP_OUT_DAC) == PDSP_OUT_DAC)
		(PDSP_SAMPLE_SIZE == 1) ? DAC_TransmitData8() : DAC_TransmitData16();

	if (PDSP_MODE != PDSP_MODE_POLL) 					// Work in INT mode
		CODEC_IRQHandler();
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == LPUART1) {
		dataTx = false;
		AD_Toggle(0);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == LPUART1) {
		dataRx = true;
		AD_Toggle(1);
	}
}

void CODEC_Init(void) {
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* TIM6 init */
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = CODEC_PSC[PDSP_FS] - 1;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = CODEC_CNT[PDSP_FS] - 1;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
		Error_Handler();

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
		Error_Handler();

	/* Start TIM6 */
	HAL_TIM_Base_Start_IT(&htim6);
}

void CODEC_InitIn(void) {
	ADC_MultiModeTypeDef multimode = { 0 };
	ADC_ChannelConfTypeDef sConfig = { 0 };

	/** Common config	  */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ((PDSP_SAMPLE_SIZE == 2) ? ADC_RESOLUTION_12B : ADC_RESOLUTION_8B);
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T6_TRGO;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	hadc1.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
		Error_Handler();

	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
		Error_HandlerData(__FILE__, __LINE__);

	/** Configure the ADC multi-mode	  */
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
		Error_Handler();

	/** Configure Regular Channel	  */
	switch (PDSP_INPUT) {
	case PDSP_IN_ADC_MIC:
		sConfig.Channel = ADC_CHANNEL_4;
		break;
	case PDSP_IN_ADC_WVG:
		sConfig.Channel = ADC_CHANNEL_5;
		break;
	default:
		sConfig.Channel = ADC_CHANNEL_4;
	}
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_24CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_1;
	sConfig.Offset = (uint16_t) PDSP_CODEC_OFFSET_ADC;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		Error_Handler();

	HAL_ADC_Start(&hadc1);
}

void CODEC_InitOut(void) {
	/* DAC Initialization	  */
	hdac1.Instance = DAC1;
	if (HAL_DAC_Init(&hdac1) != HAL_OK)
		Error_Handler();

	DAC_ChannelConfTypeDef sConfig = { 0 };
	/* DAC channel OUT1 config	 */
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
	if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
		Error_Handler();

	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
}

/*-----------------------------------------------------------------------------
 * Funkcje inicjalizacji niskiego poziomu HAL MSP
 *---------------------------------------------------------------------------*/
void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {
	if (uartHandle->Instance == LPUART1) {
		/* LPUART1 DMA Init - LPUART_RX Init */
		hdma_lpuart_rx.Instance = DMA2_Channel7;
		hdma_lpuart_rx.Init.Request = DMA_REQUEST_4;
		hdma_lpuart_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_lpuart_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_lpuart_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_lpuart_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_lpuart_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_lpuart_rx.Init.Mode = DMA_NORMAL;
		hdma_lpuart_rx.Init.Priority = DMA_PRIORITY_LOW;
		if (HAL_DMA_Init(&hdma_lpuart_rx) != HAL_OK)
			Error_Handler();

		/* LPUART1 DMA Init - LPUART_TX Init */
		hdma_lpuart_tx.Instance = DMA2_Channel6;
		hdma_lpuart_tx.Init.Request = DMA_REQUEST_4;
		hdma_lpuart_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_lpuart_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_lpuart_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_lpuart_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_lpuart_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_lpuart_tx.Init.Mode = DMA_NORMAL;
		hdma_lpuart_tx.Init.Priority = DMA_PRIORITY_LOW;
		if (HAL_DMA_Init(&hdma_lpuart_tx) != HAL_OK)
			Error_Handler();

		__HAL_LINKDMA(uartHandle, hdmarx, hdma_lpuart_rx);
		__HAL_LINKDMA(uartHandle, hdmatx, hdma_lpuart_tx);
	}
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle) {
	if (tim_baseHandle->Instance == TIM6) {
		/* TIM6 clock enable */
		__HAL_RCC_TIM6_CLK_ENABLE();
		/* TIM6 interrupt Init */
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 13, 0);
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	}
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle) {
	if (tim_baseHandle->Instance == TIM6) {
		/* Peripheral clock disable */
		__HAL_RCC_TIM6_CLK_DISABLE();
		/* TIM6 interrupt Deinit */
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
	}
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (adcHandle->Instance == ADC1) {
		/* ADC1 clock enable */
		__HAL_RCC_ADC_CLK_ENABLE();
		/**ADC1 GPIO Configuration    PC3     ------> ADC1_IN4 - MIC_OUT   */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		/**ADC1 GPIO Configuration    PA0     ------> ADC1_IN5 - WVG   */
		__HAL_RCC_GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	} else if (adcHandle->Instance == ADC2) {
		/* ADC2 clock enable */
		__HAL_RCC_ADC_CLK_ENABLE();
		/**ADC2 GPIO Configuration    PC2     ------> ADC2_IN3 - POT   */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	}
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle) {
	if (adcHandle->Instance == ADC1) {
		/* Peripheral clock disable */
		__HAL_RCC_ADC_CLK_DISABLE();
		/**ADC1 GPIO Configuration		 PC3     ------> ADC1_IN4		 */
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_3);
		/**ADC1 GPIO Configuration    	 PA0     ------> ADC1_IN5 - WVG   */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
	} else if (adcHandle->Instance == ADC2) {
		/* Peripheral clock disable */
		__HAL_RCC_ADC_CLK_DISABLE();
		/**ADC2 GPIO Configuration		 PC2     ------> ADC2_IN3		 */
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2);
	}
}

void HAL_DAC_MspInit(DAC_HandleTypeDef *dacHandle) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (dacHandle->Instance == DAC1) {
		__HAL_RCC_DAC1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**DAC1 GPIO Configuration: PA4     	------> DAC1_OUT1 - DAC_OUT */
		GPIO_InitStruct.Pin = GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef *dacHandle) {
	if (dacHandle->Instance == DAC1) {
		__HAL_RCC_DAC1_CLK_DISABLE();
		/**DAC1 GPIO Configuration:  PA4     	------> DAC1_OUT1    */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
	}
}

/*-----------------------------------------------------------------------------
 * Funkcje obsługi przerwań: TIM6, DMA_UART_TX, DMA_UART_RX, UART
 *---------------------------------------------------------------------------*/
/** @brief This function handles TIM6 global interrupt, DAC channel1 and channel2 underrun error interrupts.  */
void TIM6_DAC_IRQHandler(void) {
	AD_On(DEBUG_TIM6_IRQ_IO);
	AD_On(DEBUG_PROCESSING_IO);
	if (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) != RESET) {
		if (__HAL_TIM_GET_IT_SOURCE(&htim6, TIM_IT_UPDATE) != RESET) {
			__HAL_TIM_CLEAR_IT(&htim6, TIM_IT_UPDATE);
			TIM6_PeriodElapsedCallback(&htim6);
		}
	}
	AD_Off(DEBUG_TIM6_IRQ_IO);
}
/** @brief This function handles DMA2 channel6 global interrupt.  */
void DMA2_Channel6_IRQHandler(void) {
	AD_Toggle(4);
	HAL_DMA_IRQHandler(&hdma_lpuart_tx);
	AD_Toggle(4);
}
/** @brief This function handles DMA2 channel7 global interrupt.  */
void DMA2_Channel7_IRQHandler(void) {
	AD_Toggle(3);
	HAL_DMA_IRQHandler(&hdma_lpuart_rx);
	AD_Toggle(3);
}
/** @brief This function handles LPUART1 global interrupt.  */
void LPUART1_IRQHandler(void) {
	AD_Toggle(2);
	HAL_UART_IRQHandler(&hlpuart1);
	AD_Toggle(2);
}

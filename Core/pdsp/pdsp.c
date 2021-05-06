/* Includes ------------------------------------------------------------------*/
#include "pdsp.h"
#include "main.h"

/* Private variables ---------------------------------------------------------*/
volatile bool DataNew = false;
volatile bool dataRx = false;
volatile bool dataTx = false;

CODEC_Data Samples[PDSP_BUFFER_DEFAULT_SIZE];     	// Bufor próbek
uint32_t SampleNumber;

CODEC_Data DataIn, DataOut; // Zmienne do przechowywania próbek odebranej/wysyłanej

int8_t valueUartBuffRx[PDSP_CODEC_BUFFOR_LENGTH];
int8_t valueUartBuffTx[PDSP_CODEC_BUFFOR_LENGTH];
uint16_t *pValueUartBuffRx = (uint16_t*) valueUartBuffRx;
uint16_t *pValueUartBuffTx = (uint16_t*) valueUartBuffTx;

// Konfiguracja MAIN PLL w zależności od wybranej częstotliwości próbkowania
uint8_t PDSP_PLLM[] = { 40, 40, 39, 40, 77, 39, 36 };
uint8_t PDSP_PLLR[] = { 2, 2, 2, 2, 4, 2, 2 };

/* Private function prototypes -----------------------------------------------*/
void CODEC_InitIn(void);
void CODEC_InitOut(void);
void CODEC_Init(void);

/* Private user code ---------------------------------------------------------*/
void PDSP_Init(void) {
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Configure LSE Drive Capability	  */
	  HAL_PWR_EnableBkUpAccess();
	  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

	/** Initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure. */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = PDSP_PLLM[PDSP_FS];
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = PDSP_PLLR[PDSP_FS];
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		Error_Handler();

	/** Initializes the CPU, AHB and APB buses clocks */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
		Error_Handler();

	HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInit);
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1 | RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
	PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_SYSCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		Error_Handler();

	/** Configure the main internal regulator output voltage */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
		Error_Handler();

	  /** Enable MSI Auto calibration  */
	  HAL_RCCEx_EnableMSIPLLMode();

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* BSP platform init */
	BSP_InitAll();

	/*  */
	PDSP_CODEC_Init();

	/*  */
	BSP_LCD_GoTo(0, 0);
	uint8_t buff[48];
	sprintf((char *)buff, "Baud = %d", (int)PDSP_CODEC_BAUDRATE);
	BSP_LCD_WriteText(buff);
}

void PDSP_CODEC_Init() {
	/* LPUART1 init */
	BSP_COM_Init(PDSP_CODEC_BAUDRATE);

	CODEC_InitIn();
	CODEC_InitOut();
	CODEC_Init();

	/* DMA interrupt init for LPUART*/
	/* DMA2_Channel6_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Channel6_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(DMA2_Channel6_IRQn);
	/* DMA2_Channel7_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Channel7_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(DMA2_Channel7_IRQn);

	/* Global interrupt enable */
	__enable_irq();
}

/*---------------------------------------------------------------------------
 * Funkcje dostępu do danych
 *-------------------------------------------------------------------------*/

CODEC_Data CODEC_GetSample(void) {
	CODEC_Data sample;
	if (PDSP_MODE == PDSP_MODE_POLL) {
		while (!dataRx)
			;
	}

	if (PDSP_SAMPLE_SIZE == 1) {
		sample.channel[LEFT] = valueUartBuffRx[LEFT];
		if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
			sample.channel[RIGHT] = valueUartBuffRx[RIGHT];
	} else {
		sample.channel[LEFT] = pValueUartBuffRx[LEFT];
		if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
			sample.channel[RIGHT] = pValueUartBuffRx[RIGHT];
	}

	dataRx = false;
	return sample;
}

void CODEC_SetSample(CODEC_Data sample) {
	if (PDSP_MODE == PDSP_MODE_POLL) {
		while (dataTx)
			;
	}

	if (PDSP_SAMPLE_SIZE == 1) {
		valueUartBuffTx[LEFT] = sample.channel[LEFT];
		if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
			valueUartBuffTx[RIGHT] = sample.channel[RIGHT];
	} else {
		pValueUartBuffTx[LEFT] = sample.channel[LEFT];
		if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
			pValueUartBuffTx[RIGHT] = sample.channel[RIGHT];
	}

	dataTx = true;
}

CODEC_DataValue CODEC_GetSampleValue(void) {
	CODEC_DataValue sample;

	sample.channel[LEFT] = (pValueUartBuffRx[LEFT] * PDSP_CODEC_Vres); // Rzeczywista wartość napięcia
	if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
		sample.channel[RIGHT] = (pValueUartBuffRx[RIGHT] * PDSP_CODEC_Vres); // Rzeczywista wartość napięcia#endif

	return sample;
}

void CODEC_SetSampleValue(CODEC_DataValue sample) {
	pValueUartBuffTx[LEFT] = (sample.channel[LEFT] / PDSP_CODEC_Vres);
	if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
		pValueUartBuffTx[RIGHT] = (sample.channel[RIGHT] / PDSP_CODEC_Vres);
}

CODEC_Data CODEC_SetVolume(CODEC_DataValue * value, float ampl) {
	CODEC_Data sample;

	if (PDSP_SAMPLE_SIZE == 1) {
		sample.channel[LEFT] = (int8_t) (ampl * value->channel[LEFT]);
		if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
			sample.channel[RIGHT] = (int8_t) (ampl * value->channel[RIGHT]);
	} else {
		sample.channel[LEFT] = (int16_t) (ampl * value->channel[LEFT]);
		if (PDSP_NUM_CHANNELS == PDSP_CHANNELS_NUM_STEREO)
			sample.channel[RIGHT] = (int16_t) (ampl * value->channel[RIGHT]);

	}

	return sample;
}

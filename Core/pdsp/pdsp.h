#ifndef INC_PDSP_H_
#define INC_PDSP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

/* Private typedef -----------------------------------------------------------*/
/* -----------------------------------------------------------------------------
 * Definicja typów do konfiguracji systemu
 * -------------------------------------------------------------------------- */
enum {
	FREQ_8_KHz = 0,
	FREQ_16_KHz,
	FREQ_24_KHz,
	FREQ_32_KHz,
	FREQ_44_KHz,
	FREQ_48_KHz,
	FREQ_96_KHz
};

enum {
	PDSP_MODE_POLL = 0, PDSP_MODE_INT, PDSP_MODE_INT_FRAME, PDSP_MODE_DMA_FRAME
};

enum {
	PDSP_IN_NONE = 0, PDSP_IN_UART = 1, PDSP_IN_ADC_MIC = 2, PDSP_IN_ADC_WVG = 4
};

enum {
	PDSP_OUT_UART = 1, PDSP_OUT_DAC = 2
};

enum {
	PDSP_CHANNELS_NUM_MONO = 1, PDSP_CHANNELS_NUM_STEREO = 2
};

enum {
	PDSP_BUFFOR_SINGLE = 1, PDSP_BUFFOR_PING_PONG = 2, PDSP_BUFFOR_TRIPLE = 3
};

#include "pdsp/pdsp_config.h"

/* ----------------------------------------------------------------------------
 * Definicja typów do przechowywania wartości próbek
 * --------------------------------------------------------------------------*/
typedef union {
#if (PDSP_SAMPLE_SIZE == 1)
#if PDSP_NUM_CHANNELS == 1
	uint8_t uint;
	int8_t channel[PDSP_NUM_CHANNELS];
	uint8_t tmp[3];
#elif PDSP_NUM_CHANNELS == 2
	uint16_t uint;
	int8_t channel[PDSP_NUM_CHANNELS];
	uint8_t tmp[2];
#endif
#else
#if (PDSP_NUM_CHANNELS == 1)
	uint16_t uint;
	int16_t channel[PDSP_NUM_CHANNELS];
	uint8_t tmp[2];
#elif (PDSP_NUM_CHANNELS == 2)
	uint32_t uint;
	int16_t channel[PDSP_NUM_CHANNELS];
#endif
#endif
} CODEC_Data;

typedef struct {
	float channel[PDSP_NUM_CHANNELS];
} CODEC_DataValue;

extern uint32_t CODEC_FS[];

/* Private define ------------------------------------------------------------*/
#define PDSP_CODEC_Fs						    (float)CODEC_FS[PDSP_FS]
#define PDSP_CODEC_Ts                   (1.0f / (float)CODEC_FS[PDSP_FS])

#if (PDSP_SAMPLE_SIZE == 1)
#define PDSP_CODEC_Vres       (PDSP_CODEC_Vpp / PDSP_CODEC_Bres)	//
#else
#define PDSP_CODEC_Vres       (PDSP_CODEC_Vpp / PDSP_CODEC_Bres)	//
#endif

#define PDSP_CODEC_BUFFOR_LENGTH				PDSP_SAMPLE_SIZE * PDSP_NUM_CHANNELS
#define PDSP_CODEC_OFFSET_ADC				   (PDSP_CODEC_Bres / 2)
#define PDSP_CODEC_BAUDRATE			 (uint32_t)(CODEC_FS[PDSP_FS] * PDSP_CODEC_BUFFOR_LENGTH * 8.0f * 2.0f)

/* Określenie numeru kanalu */
#define LEFT              	0
#define RIGHT              	1
/**/
#define PDSP_PI                     (float)(M_PI)
#define PDSP_PI_DIV_2               (float)(M_PI / 2.0)
#define PDSP_2PI                    (float)(M_PI * 2.0)
#define PDSP_2PI_DIV_FS             (float)((M_PI * 2.0) / PDSP_CODEC_Fs)
#define PDSP_2PI_TS                 (float)((M_PI * 2.0) * PDSP_CODEC_Ts)

/* Exported constants ---------------------------------------------------------*/
// Definicja funkcji obsługi  kodeka - emulacja za pomocą licznika
#define DEBUG_TIM6_IRQ_IO			5
#define DEBUG_MAIN_LOOP_IO			6
#define DEBUG_PROCESSING_IO			7

/* Exported macros ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/

extern CODEC_Data DataIn, DataOut;
extern CODEC_Data DataSamples[];
extern uint32_t SampleNumber;
extern volatile bool DataNew, dataRx, dataTx;

extern int8_t valueUartBuffRx[PDSP_CODEC_BUFFOR_LENGTH];
extern int8_t valueUartBuffTx[PDSP_CODEC_BUFFOR_LENGTH];
extern uint16_t *pValueUartBuffRx;
extern uint16_t *pValueUartBuffTx;


/* Exported functions -------------------------------------------------------*/
void PDSP_Init();
void PDSP_CODEC_Init();
void PDSP_INT_Init(void);

CODEC_Data CODEC_GetSample(void);
void CODEC_SetSample(CODEC_Data sample);

CODEC_DataValue CODEC_GetSampleValue(void);
void CODEC_SetSampleValue(CODEC_DataValue sample);

CODEC_Data CODEC_SetVolume(CODEC_DataValue * value, float ampl);

void CODEC_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_PDSP_H_ */

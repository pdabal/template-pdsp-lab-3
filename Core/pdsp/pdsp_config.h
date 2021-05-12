#ifndef INC_PDSP_CONFIG_H_
#define INC_PDSP_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Public constants ---------------------------------------------------------*/
// Do wyboru jedna z lini konfigurującej tryb pracy:
//#define PDSP_MODE             PDSP_MODE_POLL
#define PDSP_MODE               PDSP_MODE_INT
//#define PDSP_MODE             PDSP_MODE_INT_FRAME
//#define PDSP_MODE             PDSP_MODE_DMA_FRAME

// Do wyboru jedna z lini konfigurującej liczbę kanałów:
#define PDSP_NUM_CHANNELS       1
//#define PDSP_NUM_CHANNELS     2

// Do wyboru jedna z lini konfigurującej rozmiar próbki w bajtach
#define PDSP_SAMPLE_SIZE		1
//#define PDSP_SAMPLE_SIZE		2

// Do wyboru jedna z lini konfigurującej wejście kodeka AIC23:
//#define PDSP_INPUT      		PDSP_IN_UART
//#define PDSP_INPUT        	PDSP_IN_ADC_MIC
//#define PDSP_INPUT        	PDSP_IN_ADC_WVG
#define PDSP_INPUT        	PDSP_IN_NONE

#define PDSP_OUTPUT			(PDSP_OUT_UART | PDSP_OUT_DAC)
//#define PDSP_OUTPUT				PDSP_OUT_UART
//#define PDSP_OUTPUT				PDSP_OUT_DAC
//#define PDSP_OUTPUT				PDSP_OUT_NONE


// Do wyboru jedna z lini konfigurującej częstotliwość próbkowania
//#define PDSP_FS          		FREQ_96_KHz    // 6713 DSK at 96kHz sample rate
//#define PDSP_FS          		FREQ_48_KHz    // 6713 DSK at 48kHz sample rate
//#define PDSP_FS          		FREQ_44_KHz    // 6713 DSK at 44kHz sample rate
//#define PDSP_FS          		FREQ_32_KHz    // 6713 DSK at 32kHz sample rate
//#define PDSP_FS          		FREQ_24_KHz    // 6713 DSK at 24kHz sample rate
//#define PDSP_FS          		FREQ_16_KHz    // 6713 DSK at 16kHz sample rate
#define PDSP_FS          		FREQ_8_KHz     // 6713 DSK at 8kHz sample rate

// Do wyboru jedna z lini konfigurującej typ bufora
#define PDSP_BUFFOR             PDSP_BUFFOR_SINGLE
//#define PDSP_BUFFOR           PDSP_BUFFOR_PING_PONG
//#define PDSP_BUFFOR           PDSP_BUFFOR_TRIPLE

// Konfiguracja domyślnego rozmiaru bufora
#define PDSP_BUFFER_DEFAULT_SIZE   	    8

// Konfiguracja parametrów kodeka (przetworników ADC/DAC)
#define PDSP_CODEC_Vpp                   3.3f	//
#if (PDSP_SAMPLE_SIZE == 1)
#define PDSP_CODEC_Bit					 8.0f
#define PDSP_CODEC_Bres                256.0f	//
#else
#define PDSP_CODEC_Bit					12.0f
#define PDSP_CODEC_Bres               4096.0f	//
#endif

/* Exported macros ------------------------------------------------------------*/
#ifdef ARM_MATH_CM4
#include "arm_math.h"
#define	pdsp_sinf(phase)			 arm_sin_f32(phase)
#define	pdsp_cosf(phase)			 arm_cos_f32(phase)

#else
#define	pdsp_sinf(phase)			 sinf(phase)
#define	pdsp_cosf(phase)			 cosf(phase)

#endif

//powf(10.f,x) is exactly exp(log(10.0f)*x)
#define pow10f(x) 					expf(x * 2.302585092994046f)


#ifdef __cplusplus
}
#endif

#endif /* INC_PDSP_CONFIG_H_ */

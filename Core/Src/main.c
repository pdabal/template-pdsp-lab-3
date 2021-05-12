/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "pdsp/pdsp.h"
#include "arm_common_tables.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define     TICK_START()        tickStart = SysTick->VAL
#define     TICK_TIME()     {   tickStop = SysTick->VAL;      \
                                tickDelay = tickStart - tickStop;   \
                                processingTime = (tickDelay - 3) * (1.0f / 80.0f);  }

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
float A = PDSP_CODEC_Vpp / 2;
float f = 100.0f;
uint32_t n = 23;
float Ts;
float fi = 0.0f;
float y;
float step;

uint32_t N = 1;

// Wyznaczanie czasu realizacji
uint32_t tickStart, tickStop,tickDelay;
float processingTime;

//
float sinTab[24];

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    Ts = PDSP_CODEC_Ts;
    A *= (PDSP_CODEC_Vpp / PDSP_CODEC_Bres) * 1000.0f;
    step = (2 * M_PI * f * Ts);

    for(n = 0; n < 24; n++){
        sinTab[n] = A * pdsp_sinf(2 * M_PI * Ts * n * 333);
    }
    n = 0;

    PDSP_Init();
	MENU_Init(hMenu);

	while (1) {
		AD_On(DEBUG_MAIN_LOOP_IO);
		if (PDSP_MODE == PDSP_MODE_POLL) {
//			DataIn = CODEC_GetSample();  	// Odczytanie nowej próbki od kodeka
			CODEC_SetSample(DataOut);  		// Wysłanie próbki do kodeka
			DataOut = DataIn;               // Przetwarzanie
		} else {
			if (DataNew == true) {
				DataNew = false;
//				DataOut = DataIn;           // Przetwarzanie
				AD_On(DEBUG_PROCESSING_IO);
				tickStart = SysTick->VAL;

//				if(fi > PDSP_2PI)
//				    fi -= PDSP_2PI;
//				y = A * pdsp_sinf(fi);
//				fi += step;

//				y = A * sinTable_f32[n%512];
				y =  sinTab[n];
				n += 1;
				if (n > 23)
				    n = 0;

				TICK_START();
				TICK_TIME();
				AD_Off(DEBUG_PROCESSING_IO);

				DataOut.channel[LEFT] = (int8_t)y;
			}
		}
		AD_Off(DEBUG_MAIN_LOOP_IO);
	}
}

void CODEC_IRQHandler(void){
//	DataIn = CODEC_GetSample();   		// Odczytanie nowej próbki od kodeka
//	DataOut = DataIn;			        // Przetwarzanie - opcjonalnie
	CODEC_SetSample(DataOut);     		// Wysłanie próbki do kodeka
	DataNew = true;             		// Ustawienie flagi obecności nowych danych
	SampleNumber++;
}

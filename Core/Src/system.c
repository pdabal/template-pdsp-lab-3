/*
 * system.c
 *
 *  Created on: 14 kwi 2021
 *      Author: pdabal
 */

#include "main.h"

int __io_putchar(int ch) {

	while (!LL_LPUART_IsActiveFlag_TXE(LPUART1))
		;
	LL_LPUART_TransmitData8(LPUART1, (uint8_t) ch);
	return ch;
}

int __io_getchar(void) {
	// Clear the Overrun flag just before receiving the first character
	LL_LPUART_ClearFlag_ORE(LPUART1);

	while (!LL_LPUART_IsActiveFlag_RXNE(LPUART1))
		;
	int ch = LL_LPUART_ReceiveData8(LPUART1);

	/* Echo */
	while (!LL_LPUART_IsActiveFlag_TXE(LPUART1))
		;
	LL_LPUART_TransmitData8(LPUART1, (uint8_t) ch);

	return ch;
}

/**
 * @brief  EXTI line detection callback.
 * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	MENU_Operation(hMenu, GPIO_Pin);
	BSP_JOY_Callback(GPIO_Pin);
	AD_Toggle(13);
}

void HAL_SYSTICK_Callback(void) {
	static uint32_t timer = 0;

	AD_On(15);
	timer++;
	BSP_SEG_Callback();
	if ((timer % 20) == 0) {
		JOY_StateUpdate();
	}
	if (timer == 59) {
		MENU_Value(hMenu);
		timer = 0;
	}
	AD_Off(15);
}


/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

void Error_HandlerData(char *file, uint32_t line) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

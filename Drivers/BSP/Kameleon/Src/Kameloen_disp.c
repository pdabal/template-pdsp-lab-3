/* Includes ------------------------------------------------------------------*/
#include "Kameleon.h"
#include "hd44780.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DIG1_Pin 								GPIO_PIN_2
#define DIG2_Pin 								GPIO_PIN_3
#define DIG3_Pin 								GPIO_PIN_4
#define DIG4_Pin 								GPIO_PIN_5

#define DIG_Port								GPIOB
#define	DIG_Num			 						4
#define DIG_MSK 								(0x0Fu << 2)

#define SEG_A_Pin 								GPIO_PIN_0
#define SEG_B_Pin 								GPIO_PIN_1
#define SEG_C_Pin 								GPIO_PIN_2
#define SEG_D_Pin 								GPIO_PIN_3
#define SEG_E_Pin 								GPIO_PIN_4
#define SEG_F_Pin 								GPIO_PIN_5
#define SEG_G_Pin 								GPIO_PIN_6
#define SEG_DP_Pin 								GPIO_PIN_9

#define SEG_Port								GPIOG
#define SEG_MSK 								(0xEFu | SEG_DP_Pin | SEG_E_Pin)

#define SEG_Prescaler 							(((HAL_RCC_GetHCLKFreq() / 400) / DIG_Num) / 2) - 1
#define SEG_Period 								1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
uint16_t display[DIG_Num];

const uint8_t segments[] = {
		SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin,	// 0
		SEG_B_Pin | SEG_C_Pin,	// 1
		SEG_A_Pin | SEG_B_Pin | SEG_D_Pin | SEG_E_Pin | SEG_G_Pin,	// 2
		SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_G_Pin,	// 3
		SEG_B_Pin | SEG_C_Pin | SEG_F_Pin | SEG_G_Pin,	// 4
		SEG_A_Pin | SEG_C_Pin | SEG_D_Pin | SEG_F_Pin | SEG_G_Pin,	// 5
		SEG_A_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin,	// 6
		SEG_A_Pin | SEG_B_Pin | SEG_C_Pin,	// 7
		SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin,	// 8
		SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_F_Pin | SEG_G_Pin,	// 9
		SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin,	// A
		SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin,	// b
		SEG_D_Pin | SEG_E_Pin | SEG_G_Pin,	// c
		SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_G_Pin,	// d
		SEG_A_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin,	// E
		SEG_A_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin, // F
		SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin, //G
		SEG_F_Pin | SEG_G_Pin | SEG_B_Pin | SEG_E_Pin | SEG_C_Pin, // H
		SEG_B_Pin | SEG_C_Pin, // I
		SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin, // J
		0,
		SEG_F_Pin | SEG_E_Pin | SEG_D_Pin, // L
		0,
		SEG_E_Pin | SEG_G_Pin | SEG_C_Pin, // n
		SEG_G_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin, // o
		SEG_A_Pin | SEG_B_Pin | SEG_G_Pin | SEG_F_Pin | SEG_E_Pin, // P
		0,
		SEG_G_Pin | SEG_E_Pin, // r
		SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin, // S (wyswietl 0)
		0,
		SEG_F_Pin | SEG_E_Pin | SEG_D_Pin | SEG_C_Pin | SEG_B_Pin, // U
		0, 0, 0, 0 };

const uint8_t digits[] = { DIG4_Pin, DIG3_Pin, DIG2_Pin, DIG1_Pin };

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
void BSP_SEG_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_PWREx_EnableVddIO2();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, DIG1_Pin | DIG2_Pin | DIG3_Pin | DIG4_Pin, GPIO_PIN_RESET);
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG,
	SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin | SEG_DP_Pin, GPIO_PIN_RESET);

	/* Configure GPIO pins : DIG1_Pin DIG2_Pin DIG3_Pin DIG4_Pin */
	GPIO_InitStruct.Pin = DIG1_Pin | DIG2_Pin | DIG3_Pin | DIG4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	/* Configure GPIO pins : SEG_A_Pin SEG_B_Pin SEG_C_Pin SEG_D_Pin SEG_E_Pin SEG_F_Pin SEG_G_Pin SEG_DP_Pin */
	GPIO_InitStruct.Pin =
	SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin | SEG_DP_Pin;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

uint32_t BSP_SEG_Display(int16_t value) {
	if ((value >= 10000) || (value < -999)) {
		display[3] = segments[20];
		display[2] = segments[20];
		display[1] = segments[20];
		display[0] = segments[20];
		return 1;
	} else {
		if (value >= 0) {
			display[0] = segments[value % 10];
			value /= 10;
			display[1] = value ? segments[value % 10] : 0;
			value /= 10;
			display[2] = value ? segments[value % 10] : 0;
			value /= 10;
			display[3] = value ? segments[value % 10] : 0;
		} else {
			display[0] = abs(value) ? segments[abs(value) % 10] : 0;
			value /= 10;
			display[1] = abs(value) ? segments[abs(value) % 10] : 0;
			value /= 10;
			display[2] = abs(value) ? segments[abs(value) % 10] : 0;
			// Sign
			display[3] = SEG_G_Pin;
		}
		return 0;
	}
}

uint32_t BSP_SEG_DisplayFixedPoint(uint16_t value, uint8_t fraction_digits) {
	if (value > 9999 || fraction_digits > 3)
		return 1;

	display[0] = segments[value % 10];
	value /= 10;
	display[1] = value ? segments[value % 10] : (fraction_digits >= 1 ? segments[0] : 0);
	value /= 10;
	display[2] = value ? segments[value % 10] : (fraction_digits >= 2 ? segments[0] : 0);
	value /= 10;
	display[3] = value ? segments[value % 10] : (fraction_digits >= 3 ? segments[0] : 0);
	if (fraction_digits > 0)
		display[fraction_digits] |= SEG_DP_Pin;

	return 0;
}

void BSP_SEG_Callback(void) {
	static uint8_t dig = 0;

	LL_GPIO_ResetOutputPin(DIG_Port, DIG_MSK);
	LL_GPIO_ResetOutputPin(SEG_Port, SEG_MSK);
	LL_GPIO_SetOutputPin(SEG_Port, display[dig]);
	LL_GPIO_SetOutputPin(DIG_Port, digits[dig++]);
	dig = dig & 3;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} LCD_IO_TypeDef;

/* Private define ------------------------------------------------------------*/
#define LCD_D0_Pin 			GPIO_PIN_14
#define LCD_D1_Pin 			GPIO_PIN_15
#define LCD_D2_Pin 			GPIO_PIN_0
#define LCD_D3_Pin 			GPIO_PIN_1
#define LCD_D4_Pin 			GPIO_PIN_7
#define LCD_D5_Pin 			GPIO_PIN_8
#define LCD_D6_Pin 			GPIO_PIN_9
#define LCD_D7_Pin 			GPIO_PIN_10

#define LCD_BKL_Pin 		GPIO_PIN_13
#define LCD_E_Pin 			GPIO_PIN_5
#define LCD_RW_Pin 			GPIO_PIN_4
#define LCD_RS_Pin 			GPIO_PIN_2

#define LCD_D0_GPIO_Port 	GPIOD
#define LCD_D1_GPIO_Port	GPIOD
#define LCD_D2_GPIO_Port 	GPIOD
#define LCD_D3_GPIO_Port 	GPIOD
#define LCD_D4_GPIO_Port 	GPIOE
#define LCD_D5_GPIO_Port 	GPIOE
#define LCD_D6_GPIO_Port 	GPIOE
#define LCD_D7_GPIO_Port 	GPIOE

#define LCD_BKL_GPIO_Port 	GPIOB
#define LCD_E_GPIO_Port 	GPIOD
#define LCD_RW_GPIO_Port 	GPIOD
#define LCD_RS_GPIO_Port 	GPIOF

#define LCD_DATA_GPIOD_MSK	(LCD_D0_Pin | LCD_D1_Pin | LCD_D2_Pin | LCD_D3_Pin)
#define LCD_DATA_GPIOE_MSK	(LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LCD_IO_TypeDef LCD_Defs[8] = { { LCD_D0_GPIO_Port, LCD_D0_Pin }, { LCD_D1_GPIO_Port, LCD_D1_Pin }, { LCD_D2_GPIO_Port,
LCD_D2_Pin }, { LCD_D3_GPIO_Port, LCD_D3_Pin }, { LCD_D4_GPIO_Port, LCD_D4_Pin }, { LCD_D5_GPIO_Port, LCD_D5_Pin }, {
		LCD_D6_GPIO_Port, LCD_D6_Pin }, { LCD_D7_GPIO_Port, LCD_D7_Pin } };

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
static void LCD_Write(uint8_t data) {
	LL_GPIO_ResetOutputPin(LCD_RW_GPIO_Port, LCD_RW_Pin);

	for (uint32_t i = 0; i < 8; ++i)
		if ((data >> i) & 1)
			LL_GPIO_SetOutputPin(LCD_Defs[i].port, LCD_Defs[i].pin);
		else
			LL_GPIO_ResetOutputPin(LCD_Defs[i].port, LCD_Defs[i].pin);

	LL_GPIO_SetOutputPin(LCD_E_GPIO_Port, LCD_E_Pin);
	HAL_Delay(2);
	LL_GPIO_ResetOutputPin(LCD_E_GPIO_Port, LCD_E_Pin);
	HAL_Delay(2);
}

static uint8_t LCD_Read(void) {
	uint8_t data;

	LL_GPIO_SetOutputPin(LCD_RW_GPIO_Port, LCD_RW_Pin);
	HAL_Delay(2);
	LL_GPIO_SetOutputPin(LCD_E_GPIO_Port, LCD_E_Pin);

	for (uint32_t i = 0; i < 8; ++i) {
		data |= ((uint8_t) HAL_GPIO_ReadPin(LCD_Defs[i].port, LCD_Defs[i].pin) << i);
	}
	LL_GPIO_ResetOutputPin(LCD_E_GPIO_Port, LCD_E_Pin);
	return data;
}

static uint8_t LCD_ReadStatus(void) {
	unsigned char status = 0;
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;

	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;

	GPIO_InitStruct.Pin = LCD_D0_Pin | LCD_D1_Pin | LCD_D2_Pin | LCD_D3_Pin;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	LL_GPIO_SetOutputPin(LCD_RW_GPIO_Port, LCD_RW_Pin);
	LL_GPIO_ResetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);

	status = LCD_Read();

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	GPIO_InitStruct.Pin = LCD_D0_Pin | LCD_D1_Pin | LCD_D2_Pin | LCD_D3_Pin;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	return status;
}
/* Send command to LCD module */
static void LCD_Cmd(unsigned char cmd) {
	LL_GPIO_ResetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);
	LCD_Write(cmd);
}

/* Send data to LCD module */
static void LCD_Data(uint8_t data) {
	LL_GPIO_SetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);
	LCD_Write(data);
}

static void GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	LL_GPIO_ResetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);
	/*Configure GPIO pin Output Level */
	LL_GPIO_ResetOutputPin(GPIOE, LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin);
	/*Configure GPIO pin Output Level */
	LL_GPIO_ResetOutputPin(LCD_BKL_GPIO_Port, LCD_BKL_Pin);
	/*Configure GPIO pin Output Level */
	LL_GPIO_ResetOutputPin(GPIOD, LCD_D0_Pin | LCD_D1_Pin | LCD_D2_Pin | LCD_D3_Pin | LCD_RW_Pin | LCD_E_Pin);

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;

	/*Configure GPIO pin : LCD_RS_Pin */
	GPIO_InitStruct.Pin = LCD_RS_Pin;
	HAL_GPIO_Init(LCD_RS_GPIO_Port, &GPIO_InitStruct);
	/*Configure GPIO pins : LCD_D4_Pin LCD_D5_Pin LCD_D6_Pin LCD_D7_Pin */
	GPIO_InitStruct.Pin = LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	/*Configure GPIO pin : LCD_BKL_Pin */
	GPIO_InitStruct.Pin = LCD_BKL_Pin;
	HAL_GPIO_Init(LCD_BKL_GPIO_Port, &GPIO_InitStruct);
	/*Configure GPIO pins : LCD_D0_Pin LCD_D1_Pin LCD_D2_Pin LCD_D3_Pin LCD_RW_Pin LCD_E_Pin */
	GPIO_InitStruct.Pin = LCD_D0_Pin | LCD_D1_Pin | LCD_D2_Pin | LCD_D3_Pin | LCD_RW_Pin | LCD_E_Pin;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/* Activate Power Pin that supplies LCD module */
void BSP_LCD_BacklightOn(void) {
	LL_GPIO_SetOutputPin(LCD_BKL_GPIO_Port, LCD_BKL_Pin);
}

/* Disable Power Pin that supplies LCD module */
void BSP_LCD_BacklightOff(void) {
	LL_GPIO_ResetOutputPin(LCD_BKL_GPIO_Port, LCD_BKL_Pin);
}

/* Clear LCD module display */
void BSP_LCD_Clear(void) {
	LCD_Cmd(HD44780_CLEAR);
}

/* Initializes HD44780 LCD module in 4-bit mode */
void BSP_LCD_Init(void) {
	GPIO_Init();

	LL_GPIO_SetOutputPin(LCD_E_GPIO_Port, LCD_E_Pin);
	LL_GPIO_ResetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);
	HAL_Delay(40); 		//
	LCD_Cmd(HD44780_FUNCTION_SET | HD44780_8_BIT);
	HAL_Delay(5); 		//
	LCD_Cmd(HD44780_FUNCTION_SET | HD44780_8_BIT);
	HAL_Delay(1); 		//
	LCD_Cmd(HD44780_FUNCTION_SET | HD44780_8_BIT);

	LCD_Cmd(HD44780_FUNCTION_SET | HD44780_FONT5x7 | HD44780_TWO_LINE | HD44780_8_BIT);
	LCD_Cmd(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_OFF);
	LCD_Cmd(HD44780_CLEAR);
	HAL_Delay(5);
	LCD_Cmd(HD44780_ENTRY_MODE | HD44780_EM_SHIFT_CURSOR | HD44780_EM_INCREMENT);
	LCD_Cmd(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON);
	BSP_LCD_BacklightOn();
}

/* Set Cursor to a specified location given by row and column information
 * @param  Row Number (1 to 2)
 * @param  Column Number (1 to 16) Assuming a 2 X 16 characters display */
void BSP_LCD_GoTo(uint8_t row, uint8_t column) {
	column--;
	switch (row) {
	case 1:
		/* Set cursor to 1st row address and add index*/
		LCD_Cmd(column |= 0x80);
		break;
	case 2:
		/* Set cursor to 2nd row address and add index*/
		LCD_Cmd(column |= 0x40 | 0x80);
		break;
	default:
		break;
	}
}

void BSP_LCD_WriteText(uint8_t *text) {
	while (*text)
		LCD_Data(*text++);
}

/* Display a characters string */
void BSP_LCD_WriteTextXY(uint8_t *text, uint8_t row, uint8_t col) {
	BSP_LCD_GoTo(row, col);
	while (*text) {
		LCD_Data(*text++);
	}
}

void BSP_LCD_WriteNumber(uint32_t number){
	uint8_t buffer[16];
	uint8_t * text = buffer;
	sprintf((char*)text, (const char*)"%d", (int)number);
	while (*text){
			LCD_Data(*text++);
	}
}

void BSP_LCD_WriteNumberXY(uint32_t number, uint8_t row, uint8_t col){
	uint8_t buffer[16];
	uint8_t * text = buffer;
	sprintf((char*)text, (const char*)"%d", (int)number);
	BSP_LCD_GoTo(row, col);
	while (*text){
			LCD_Data(*text++);
	}
}

void BSP_LCD_WriteBinary(uint32_t var, uint8_t bitCount) {
	signed char i;

	for (i = (bitCount - 1); i >= 0; i--) {
		LCD_Data((var & (1 << i)) ? '1' : '0');
	}
}

void BSP_LCD_SetUserChar(int8_t charNum, uint8_t n, const int8_t *p) {
	LCD_Cmd(HD44780_CGRAM_SET | charNum * 8);
	n *= 8;
	do {
		LCD_Data(*p++);
	} while (--n);
}


void BSP_LCD_ShiftLeft(void) {
	LCD_Cmd(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_LEFT | HD44780_SHIFT_DISPLAY);
}

void BSP_LCD_ShiftRight(void) {
	LCD_Cmd(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_RIGHT | HD44780_SHIFT_DISPLAY);
}

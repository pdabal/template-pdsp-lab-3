#ifndef __KAMELEON_SENSOR_H_
#define __KAMELEON_SENSOR_H_

#include "lsm303c.h"

// Possible datarates of accelerometer i.e. possible values for acc_datarate parameter of mems_init function.
#define MEMS_ACC_DATARATE_10HZ		1
#define MEMS_ACC_DATARATE_50HZ		2
#define MEMS_ACC_DATARATE_100HZ		3
#define MEMS_ACC_DATARATE_200HZ		4
#define MEMS_ACC_DATARATE_400HZ		5
#define MEMS_ACC_DATARATE_800HZ		6

// Possible fullscale range of accelerometer i.e. possible values for acc_fullscale parameter of mems_init function.
// Fullscale range of x means the values range from -x to +x.
#define MEMS_ACC_FULLSCALE_2G		0
#define MEMS_ACC_FULLSCALE_4G		2
#define MEMS_ACC_FULLSCALE_8G		3

#define MEMS_ACC_MAXVAL				0x7fff

/* Stucture for results read from MEMs sensor.
 * The result is comprised of 3 signed 16-bit numbers - one for each axis. */
typedef struct{
	int16_t x;
	int16_t y;
	int16_t z;
} MEMS_XYZ_TypeDef;

/* Initialize LSM303C MEMS accelerometer.
 * The function should be called before using any other accelerometer function.
 * It initializes GPIO lines and I2C interface used to communicate with the sensor.
 * @return 0 on succes, 1 if the function was called with incorrect parameters or
 * interfaceing with sensor failed. */
uint8_t BSP_MEMS_Init(uint8_t acc_datarate, uint8_t acc_fullscale);

/* Read accelerometer measurement of all 3 axes. The results are put in a mems_xyz_res structure.
 * @param res: pointer to mems_xyz_res structure to put results in. */
void BSP_MEMS_AccReadXYZ(MEMS_XYZ_TypeDef * res);

/* Read x-axis acceleration.
 * @return x-axis acceleration */
int16_t BSP_MEMS_AccReadX(void);

/* Read y-axis acceleration.
 * @return y-axis acceleration */
int16_t BSP_MEMS_AccReadY(void);

/* Read z-axis acceleration.
 * @return z-axis acceleration */
int16_t BSP_MEMS_AccReadZ(void);

/*----------------------------------------------------------------------------------*/
/* Initialize LM75 temperature sensor.
 * The function should be called before using any other LM75 temperature sensor function.
 * It initializes GPIO lines and I2C interface used to communicate with the sensor. */
void BSP_TEMP_LM75_Init(void);

/* Read the temperature in degrees Celsius from LM75 temperature sensor.
 * @retval Temperature in degrees Celsius */
int8_t BSP_TEMP_LM75_Read(void);

/*----------------------------------------------------------------------------------*/
#define BSP_POT_Bres               4096.0f

/* Initialize potentiometer.
 * The function should be called before using any other potentiometer function.
 * It initializes GPIO line and ADC (ADC3 is used for potentiometer). */
void BSP_POT_Init(void);

/* Get the value read from potentiometer as a 12-bit number.
 * The function is blocking. It starts the measurement and waits for it to end.
 * @retval Potentiometer value in the range 0-0xfff. */
uint16_t BSP_POT_Read(void);

#endif /* __KAMELEON_SENSOR_H_ */

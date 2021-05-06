#ifndef _KAMELEON_MOTOR_H_
#define _KAMELEON_MOTOR_H_

#include <stdint.h>

#define MOTOR_DIR_CLOCKWISE				0
#define MOTOR_DIR_COUNTERCLOCKWISE		1

/* Initialize motor control.
 * The function should be called before using any other motor function.
 * It initializes GPIO lines and timer used for PWM generation. */
void BSP_MOTOR_Init(void);

/* Turn the motor on (sets ENABLE line to 1). */
void BSP_MOTOR_On(void);

/* Turn the motor off (sets ENABLE line to 0). */
void BSP_MOTOR_Off(void);

/* Set motor rotation direction.
 * @param dir: motor rotation direction - should be one of the values: MOTOR_DIR_CLOCKWISE and MOTOR_DIR_COUNTERCLOCKWISE. */
void BSP_MOTOR_SetDir(uint8_t dir);

/* Set motor rotation speed.
 * @param speed: motor rotation speed in range 0 (no rotation) to 100 (full speed). */
void BSP_MOTOR_SetSpeed(uint8_t speed);

#endif /* _KAMELEON_MOTOR_H_ */

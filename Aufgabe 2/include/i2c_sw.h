#ifndef I2C_SW_H
#define I2C_SW_H

#include <stdint.h>


#define SCL_LOW  GPIOB->ODR &= ~(GPIO_ODR_OD9)
#define SCL_HIGH GPIOB->ODR |= GPIO_ODR_OD9
#define SDA_LOW  GPIOB->ODR &= ~(GPIO_ODR_OD8)
#define SDA_HIGH GPIOB->ODR |= GPIO_ODR_OD8


void i2c_sw_init(void);
void i2c_start_communication(void);
void i2c_stop_communication(void);




#endif /* I2C_SW_H */
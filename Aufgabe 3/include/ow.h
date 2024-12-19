#ifndef OW_H
#define OW_H

 #define pull_down GPIOB->ODR &= ~(GPIO_ODR_OD13) 
 #define pull_up GPIOB->ODR |= GPIO_ODR_OD13


void ow_tim_init(void);

void ow_init(void);

void ow_timer(uint16_t time);

void ow_reset(void);

void ow_send_byte(uint8_t buffer);

uint8_t ow_read_byte(void);

void ow_strong_pullup(void);



#endif /* OW_H */
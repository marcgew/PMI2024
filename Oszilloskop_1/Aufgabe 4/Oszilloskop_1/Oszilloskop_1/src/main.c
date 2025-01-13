#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ili9341.h>
#include <ow.h>

volatile uint32_t ring_buffer[240];
volatile uint8_t ring_counter = 0;
volatile uint8_t falling_detect = 0;
volatile uint8_t draw_variable = 0;
volatile uint8_t hysteresis_true = 0;

void OSC_gpio_init(void)
{
  RCC->IOPENR |= RCC_IOPENR_IOPCEN; // open GPIOC Channel

  GPIOC->MODER &= ~(GPIO_MODER_MODE4_1); // Set PC4 pins as output
  GPIOC->MODER |= GPIO_MODER_MODE4_0;

  GPIOC->MODER &= ~(GPIO_MODER_MODE8_1); // Set PC8 pins as output (LED D1)
  GPIOC->MODER |= GPIO_MODER_MODE8_0;

  GPIOC->MODER &= ~(GPIO_MODER_MODE6_1); // Set PC8 pins as output (LED D1)
  GPIOC->MODER |= GPIO_MODER_MODE6_0;
}

void OSC_ADC_init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  ADC1->CR &= ~(ADC_CR_ADEN);

  ADC1->CFGR1 &= ~(ADC_CFGR1_CONT); //
  ADC1->CFGR2 = 0;
  ADC1->SMPR |= (ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1); //

  ADC1->CHSELR |= ADC_CHSELR_CHSEL15; // ADC_IN15 -> PC5

  ADC1->CR |= ADC_CR_ADEN; // Enable ADC
  while ((ADC1->ISR & ADC_ISR_ADRDY) == 0)
  {
    /* For robust implementation, add here time-out management */
  }
}

void OSC_tim6_init(void)
{
  // Reset TIM6 peripheral
  RCC->APB1RSTR |= RCC_APB1RSTR_TIM6RST;
  RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM6RST;

  // Enable the TIM6 clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

  TIM6->PSC = 16000 - 1;      // Set prescaler for 1 kHz timer clock
  TIM6->ARR = 59 - 1;         //  auf 60ms stellen (16.6 Hz) also 17 Hz
  TIM6->DIER |= TIM_DIER_UIE; // Enable update interrupt

  // Configure NVIC for TIM6 interrupts
  NVIC_ClearPendingIRQ(TIM6_IRQn);
  NVIC_SetPriority(TIM6_IRQn, 1); // Set interrupt priority
  NVIC_EnableIRQ(TIM6_IRQn);

  // Enable the timer
  TIM6->CR1 |= TIM_CR1_CEN;
}

void OSC_tim2_init(void)
{
  // Reset TIM2 peripheral
  RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
  RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;

  // Enable the TIM2 clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // Configure TIM2
  TIM2->PSC = 15;             // Set prescaler for 100000 HZ
  TIM2->ARR = 400;            // auf 400 microseconds stellen 2500HZ
  TIM2->DIER |= TIM_DIER_UIE; // Enable update interrupt

  // Configure NVIC for TIM2 interrupts
  NVIC_ClearPendingIRQ(TIM2_IRQn);
  NVIC_SetPriority(TIM2_IRQn, 2); // Set interrupt priority
  NVIC_EnableIRQ(TIM2_IRQn);

  // Enable the timer
  TIM2->CR1 |= TIM_CR1_CEN;
}

uint8_t TIM6_EN = 0;

void TIM6_DAC_IRQHandler(void)
{
  if (TIM6->SR & TIM_SR_UIF) // Check if update interrupt flag is set
  {
    GPIOC->ODR ^= GPIO_ODR_OD4;
    GPIOC->ODR ^= GPIO_ODR_OD8;
  }
  TIM6->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
}

uint8_t TIM2_EN = 0;
void TIM2_IRQHandler(void)
{
  if (TIM2->SR & TIM_SR_UIF) // Check if update interrupt flag is set
  {
    if (draw_variable == 0)
    {
      GPIOC->ODR ^= GPIO_ODR_OD6;
      ADC1->CR |= ADC_CR_ADSTART; // Start Measuring

      while ((ADC1->ISR & ADC_ISR_EOC) == 0) /* wait end of conversion */
      {
        /* For robust implementation, add here time-out management */
      }

      ring_buffer[ring_counter] = ADC1->DR;

    }

      if (ring_counter > 100)
      {
         if (ring_counter < 119)
      {
        if (ring_buffer[ring_counter] >= 4060)
        {
          hysteresis_true = 1;
        }
      }
      }

      if (ring_counter > 119 && ring_counter < 140)
      {

        if (ring_buffer[ring_counter] <= 2589)
        {
          if (ring_buffer[ring_counter] >= 2530)
          {
            if (hysteresis_true == 1)
            {
              falling_detect = 1;
            }
          }
        }
      }

      if (ring_counter == 240)
      {
        if (falling_detect == 1)
        {
          draw_variable = 1;          
        }

        ring_counter = 0;
      }
      else
      {
        ring_counter += 1;
      }
  
  }

  TIM2->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
}

int main(void)
{
  clocks_init_pmi();
  uart_init_nucusb(115200);

  OSC_gpio_init();
  OSC_ADC_init();
  OSC_tim6_init();
  OSC_tim2_init();
  ili9341_init(0);

  ili9341_rect_fill(0, 0, 240, 320, ILI9341_COLOR_WHITE);

  while (1)
  {

    if (draw_variable == 1)
    {   
      ili9341_rect_fill(0, 0, 240, 320, ILI9341_COLOR_WHITE);
      for (uint8_t i = 0; i < 240; i++)
      {
        ili9341_pixel_set(i, 220 - ((ring_buffer[i]) / 64), ILI9341_COLOR_BLACK);
        ili9341_pixel_set(i, 219 - ((ring_buffer[i]) / 64), ILI9341_COLOR_BLACK);
      }
      draw_variable = 0;
      falling_detect = 0;
      hysteresis_true = 0;
    }
  }
}

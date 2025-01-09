#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ow.h>

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
  TIM6->ARR = 59 - 1;        //  auf 60ms stellen (16.6 Hz) also 17 Hz
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
  TIM2->PSC = 16;                 // Set prescaler for 100000 HZ
  TIM2->ARR = 40;                  // auf 400 microseconds stellen 2500HZ
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
    if (TIM6_EN == 0)
    {
      TIM6_EN = 1;
    }

    else
    {
      TIM6_EN = 0;
    }
  }
  TIM6->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
}

uint8_t TIM2_EN = 0;
void TIM2_IRQHandler(void)
{
  if (TIM2->SR & TIM_SR_UIF) // Check if update interrupt flag is set
  {
    if (TIM2_EN == 0)
    {
      TIM2_EN = 1;
    }

    else
    {
      TIM2_EN = 0;
    }
  }
  TIM2->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
}

void Signal_handler(void)
{
  if (TIM6_EN == 1)
  {
    GPIOC->ODR |= GPIO_ODR_OD4;
    GPIOC->ODR |= GPIO_ODR_OD8;
  }

  else
  {
    GPIOC->ODR &= ~(GPIO_ODR_OD4);
    GPIOC->ODR &= ~(GPIO_ODR_OD8);
  }
}

uint32_t ring_buffer[240];
uint8_t ring_counter = 0;

void signal_scanner(void)
{
  if (TIM2_EN == 1)
  { 
    GPIOC->ODR ^= GPIO_ODR_OD6;
    ADC1->CR |= ADC_CR_ADSTART;// Start Measuring

    while ((ADC1->ISR & ADC_ISR_EOC) == 0) /* wait end of conversion */
    {
      /* For robust implementation, add here time-out management */
    }

    ring_buffer[ring_counter] = ADC1->DR;

    if (ring_counter == 240)
    {
      ring_counter = 0;
    }
    else
    {
      ring_counter += 1;
    }
  }
}

int main(void)
{
  clocks_init_pmi();
  uart_init_nucusb(115200);

  OSC_gpio_init();
  OSC_ADC_init();
  OSC_tim6_init();
  OSC_tim2_init();

  while (1)
  {

    Signal_handler();
    signal_scanner();

    
  }
}

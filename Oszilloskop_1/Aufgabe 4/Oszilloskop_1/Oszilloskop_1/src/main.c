#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ili9341.h>
#include <ow.h>

volatile uint32_t ring_buffer[240];
volatile uint8_t ring_counter = 0;
volatile uint8_t falling_detect = 0;
volatile uint8_t max_value = 0;
volatile uint8_t middle_detect = 0; 
volatile uint8_t draw_variable = 0;
volatile uint8_t start_countdown = 0;
volatile uint8_t middle_id = 0;
volatile uint8_t treshhold_counter = 120;
volatile uint8_t zoom_pos = 1;
uint32_t measure_count = 0;

void OSC_gpio_init(void)
{
  RCC->IOPENR |= RCC_IOPENR_IOPCEN;  // open GPIOC Channel
  RCC->IOPENR |= RCC_IOPENR_GPIOBEN; // Enable SysClock for Port B

  GPIOC->MODER &= ~(GPIO_MODER_MODE4_1); // Set PC4 pins as output
  GPIOC->MODER |= GPIO_MODER_MODE4_0;

  GPIOC->MODER &= ~(GPIO_MODER_MODE8_1); // Set PC8 pins as output (LED D1)
  GPIOC->MODER |= GPIO_MODER_MODE8_0;

  GPIOC->MODER &= ~(GPIO_MODER_MODE6_1); // Set PC8 pins as output (LED D1)
  GPIOC->MODER |= GPIO_MODER_MODE6_0;

  // Configure PB1/PB2 as input for button with pull-up resistor
  GPIOB->MODER &= ~GPIO_MODER_MODE1;
  GPIOB->PUPDR |= GPIO_PUPDR_PUPD1_0;
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD1_1);

  GPIOB->MODER &= ~GPIO_MODER_MODE2;
  GPIOB->PUPDR |= GPIO_PUPDR_PUPD2_0;
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD2_1);
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

/**
 * @brief  Configures external interrupt (EXTI) for button on PB1 & PB2
 */
void EXTI_init()
{
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Enable SysClock for SYSCFG

  SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI1_PB + SYSCFG_EXTICR1_EXTI2_PB; // Map EXTI to PB1 & PB2

  EXTI->IMR |= EXTI_IMR_IM2; // Enable interrupt mask on line 1&2
  EXTI->IMR |= EXTI_IMR_IM1;

  EXTI->FTSR |= EXTI_FTSR_TR2; // Configure falling edge trigger for line 1&2
  EXTI->FTSR |= EXTI_FTSR_TR1;

  // Set interrupt priority & enable EXTI interrupt in NVIC
  NVIC_SetPriority(EXTI0_1_IRQn, 1);
  NVIC_EnableIRQ(EXTI0_1_IRQn);
  NVIC_SetPriority(EXTI2_3_IRQn, 2);
  NVIC_EnableIRQ(EXTI2_3_IRQn);
}

/**
 * @brief Handles external interrupt for EXTI lines 0-1
 *        Toggles the car_loop flag when button on PB1 is pressed
 */
void EXTI0_1_IRQHandler(void)
{
  if (EXTI->PR & EXTI_PR_PIF1)
  { // Check if interrupt was triggered on line 1
    if (zoom_pos < 1)
    {
      systick_delay_ms(70); // Debounce delay
    }
    else
    {
      zoom_pos -= 1;
      systick_delay_ms(70); // Debounce delay
    }
  }
  EXTI->PR = EXTI_PR_PIF1; // Clear pending interrupt flag on line 1
}

/**
 * @brief Handles external interrupt for EXTI lines 2-3
 *        Toggles the pedestrian_loop flag when button on PB2 is pressed
 */
void EXTI2_3_IRQHandler(void)
{
  if (EXTI->PR & EXTI_PR_PIF2)
  { // Check if interrupt was triggered on line 2
    if (zoom_pos >= 4)
    {
      systick_delay_ms(70); // Debounce delay
    }
    else
    {
      zoom_pos += 1;
      systick_delay_ms(70); // Debounce delay
    }
  }

  EXTI->PR = EXTI_PR_PIF2; // Clear pending interrupt flag on line 2
}

void TIM6_DAC_IRQHandler(void)
{
  if (TIM6->SR & TIM_SR_UIF) // Check if update interrupt flag is set
  {
    GPIOC->ODR ^= GPIO_ODR_OD4;
    GPIOC->ODR ^= GPIO_ODR_OD8;
  }
  TIM6->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
}

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
      measure_count += 1;

      if (measure_count>=120)
      {
        if (((ring_buffer[ring_counter-1])-(ring_buffer[ring_counter]))>=50 && falling_detect==0)
        {
          falling_detect = 1;
          max_value = (ring_counter-1);
        }

        if(falling_detect == 1 && (ring_buffer[ring_counter] < 2048) && (ring_buffer[ring_counter-1] >= 2048) && start_countdown==0)
        {
          start_countdown = 1;
          middle_id = ring_counter;
        }

        
      }

      if(start_countdown == 1)
      {
        if(treshhold_counter<=0)
        {
          draw_variable = 1;
        }
        else
        {
          treshhold_counter -=1;
        }
      }

     ring_counter = (ring_counter +=1) % 240; 
      
    
      /*if (ring_counter >= 120 && (ring_buffer[ring_counter] < 2048) && (ring_buffer[ring_counter-1] >= 2048))
      {

        falling_detect = 1;
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
      }*/

    
      
    }
  }

  TIM2->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
}

void draw_funciton(void)
{

  if (draw_variable == 1)
  {
    ili9341_rect_fill(0, 0, 240, 320, ILI9341_COLOR_WHITE);
    if ((middle_id-120)<0)
    {
      iterator = middle_id+120;
      upper_limit = 240- middle_id+120;
    } 
    else
    {
      iterator = middle_id-120;
      upper_limit =
    }
    
    for (uint8_t i = 0; i < upper_limit; i++)
    { 
      ili9341_pixel_set(i, 150 - ((ring_buffer[iterator]) / 64), ILI9341_COLOR_BLACK);
      ili9341_pixel_set(i, 149 - ((ring_buffer[iterator]) / 64), ILI9341_COLOR_BLACK);
      iterator +=1;
    }
      zoom_function();
      draw_variable = 0;
      falling_detect = 0;
      start_countdown = 0;
      measure_count = 0;
      treshhold_counter = 120;

      
  }
  
}

void zoom_function(void)
{
      if (zoom_pos == 4)
    {
      TIM2->ARR = 1600;
    }

    if (zoom_pos == 3)
    {
      TIM2->ARR = 800;
    }

    if (zoom_pos == 2)
    {
      TIM2->ARR = 600;
    }

    if (zoom_pos == 1)
    {
      TIM2->ARR = 400;
    }

    if (zoom_pos == 0)
    {
      TIM2->ARR = 200;
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
  ili9341_init(0);
  EXTI_init();

  ili9341_rect_fill(0, 0, 240, 320, ILI9341_COLOR_WHITE);

  while (1)
  {
    draw_funciton();
   

  
}

}

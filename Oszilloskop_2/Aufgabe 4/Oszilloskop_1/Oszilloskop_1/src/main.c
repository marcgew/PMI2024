#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ili9341.h>
#include <pmi_string.h>
#include <ow.h>

volatile uint32_t ring_buffer[240];
volatile uint8_t ring_counter = 0;
volatile uint8_t falling_detect = 0;
volatile uint8_t max_value = 0;
volatile uint8_t middle_detect = 0;
volatile uint8_t draw_variable = 0;
volatile uint8_t start_countdown = 0;
volatile uint8_t middle_id = 0;
volatile uint8_t RC_timing = 0;
volatile uint8_t RC_Id = 0;
volatile uint8_t treshhold_counter = 120;
volatile uint8_t zoom_pos = 2;
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
  NVIC_SetPriority(EXTI0_1_IRQn, 3);
  NVIC_EnableIRQ(EXTI0_1_IRQn);
  NVIC_SetPriority(EXTI2_3_IRQn, 3);
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
      systick_delay_ms(120); // Debounce delay
    }
    else
    {
      zoom_pos -= 1;
      systick_delay_ms(120); // Debounce delay
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
      systick_delay_ms(120); // Debounce delay
    }
    else
    {
      zoom_pos += 1;
      systick_delay_ms(120); // Debounce delay
    }
  }

  EXTI->PR = EXTI_PR_PIF2; // Clear pending interrupt flag on line 2
}

void TIM2_IRQHandler(void)
{
  if (TIM2->SR & TIM_SR_UIF) // Check if update interrupt flag is set
  {
    if (draw_variable == 0)
    {

      ADC1->CR |= ADC_CR_ADSTART; // Start Measuring

      while ((ADC1->ISR & ADC_ISR_EOC) == 0) /* wait end of conversion */
      {
        /* For robust implementation, add here time-out management */
      }

      ring_buffer[ring_counter] = ADC1->DR;
      measure_count += 1;

      if (ring_buffer[ring_counter] <= 1024)
      {
        GPIOC->ODR |= GPIO_ODR_OD4;
        GPIOC->ODR |= GPIO_ODR_OD8;
      }

      if (ring_buffer[ring_counter] >= 3071)
      {
        GPIOC->ODR &= ~GPIO_ODR_OD4;
        GPIOC->ODR &= ~GPIO_ODR_OD8;
      }

      if (measure_count >= 120)
      {
        if (ring_counter == 0) // from here
        {
          if ((ring_buffer[0] >= 3071) && (ring_buffer[239] < 3071) && falling_detect == 0)
          {
            falling_detect = 1;
            max_value = 240;
          }
        }
        else
        {
          if ((ring_buffer[ring_counter] < 3071) && (ring_buffer[ring_counter - 1] >= 3071) && falling_detect == 0)
          {
            falling_detect = 1;
            max_value = (ring_counter - 1);
          }
        }

        if (ring_counter == 0)
        {
          if (falling_detect == 1 && (ring_buffer[0] >= 2048) && (ring_buffer[239] < 2048) && start_countdown == 0)
          {
            start_countdown = 1;
            middle_id = 0;
          }
        }
        else
        {
          if (falling_detect == 1 && (ring_buffer[ring_counter] < 2048) && (ring_buffer[ring_counter - 1] >= 2048) && start_countdown == 0)
          {
            start_countdown = 1;
            middle_id = ring_counter;
          }
        }
      }

      if (start_countdown == 1)
      {
        if (treshhold_counter <= 0)
        {
          draw_variable = 1;
        }
        else
        {
          treshhold_counter -= 1;
        }
      }

      ring_counter = ((ring_counter += 1) % 240);
    }
  }

  TIM2->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
}

uint8_t iterator = 0;
uint8_t upper_limit = 0;
uint8_t rising_marker1 = 0;
uint8_t rising_marker2 = 0;
uint8_t rising_counter = 0;
uint16_t rising_value = 0;
uint16_t period_count = 0;

void average_period(void)
{
  rising_marker2 = rising_marker1;
  rising_marker1 = iterator;
  rising_counter += 1;

  if (rising_counter >= 2)
  {
    if (rising_marker1 - rising_marker2 < 0)
    {
      rising_value += (rising_marker1 + 240 - rising_marker2);
      period_count += 1;
    }
    else
    {

      rising_value += (rising_marker1 - rising_marker2);
      period_count += 1;
    }
  }
}

void draw_funciton(void)
{

  if (draw_variable == 1 && ring_buffer[max_value] > ring_buffer[middle_id])
  {
    GPIOC->ODR |= GPIO_ODR_OD6;

    ili9341_rect_fill(0, 55, 240, 100, ILI9341_COLOR_WHITE);

    if ((middle_id - 120) < 0)
    {
      iterator = middle_id + 120;
      upper_limit = 240 - iterator;
    }
    else
    {
      iterator = middle_id - 120;
      upper_limit = 240 - iterator;
    }

    for (uint8_t i = 0; i < upper_limit; i++)
    {
      if (iterator == 0)
      {
        if (ring_buffer[0] >= 2048 && ring_buffer[239] < 2048)
        {
          ili9341_line_draw(0, 80, 0, 140, ILI9341_COLOR_RED);
          average_period();
        }
      }
      else
      {
        if (ring_buffer[iterator] >= 2048 && ring_buffer[iterator - 1] < 2048)
        {
          ili9341_line_draw(i, 80, i, 140, ILI9341_COLOR_RED);
          average_period();
        }
      }

      ili9341_pixel_set(i, 150 - ((ring_buffer[iterator]) / 65), ILI9341_COLOR_BLACK);
      ili9341_pixel_set(i, 149 - ((ring_buffer[iterator]) / 65), ILI9341_COLOR_BLACK);
      iterator += 1;
    }
    iterator = 0;

    for (uint8_t i = upper_limit; i < 240; i++)
    {

      if (iterator == 0)
      {
        if (ring_buffer[0] >= 2048 && ring_buffer[239] < 2048)
        {
          ili9341_line_draw(0, 80, 0, 140, ILI9341_COLOR_RED);
          average_period();
        }
      }
      else
      {
        if (ring_buffer[iterator] >= 2048 && ring_buffer[iterator - 1] < 2048)
        {
          ili9341_line_draw(i, 80, i, 140, ILI9341_COLOR_RED);
          average_period();
        }
      }

      ili9341_pixel_set(i, 150 - ((ring_buffer[iterator]) / 65), ILI9341_COLOR_BLACK);
      ili9341_pixel_set(i, 149 - ((ring_buffer[iterator]) / 65), ILI9341_COLOR_BLACK);
      iterator += 1;
    }

    GPIOC->ODR &= ~(GPIO_ODR_OD6);

    display_logic();
    zoom_function();

    draw_variable = 0;
    falling_detect = 0;
    start_countdown = 0;
    RC_timing = 0;
    measure_count = 0;
    rising_counter = 0;
    rising_value = 0;
    period_count = 0;
    treshhold_counter = 120;
  }
  else if (draw_variable == 1)
  {
    draw_variable = 0;
    falling_detect = 0;
    start_countdown = 0;
    RC_timing = 0;
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
    TIM2->ARR = 400;
  }

  if (zoom_pos == 1)
  {
    TIM2->ARR = 200;
  }

  if (zoom_pos == 0)
  {
    TIM2->ARR = 100;
  }
}

void calc_avg(void)
{
  float avg = 0;
  char avg_str[7];

  for (uint8_t j = 0; j < 240; j++)
  {
    avg += (float)ring_buffer[j];
  }
  avg = avg / 240;
  avg = avg * 3300 / 4095;

  pmi_string_float2str(avg_str, 7, avg, 7);

  ili9341_text_pos_set(6, 7);
  ili9341_str_clear(6, ILI9341_COLOR_WHITE);
  ili9341_text_pos_set(6, 7);
  ili9341_str_print(avg_str, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

void calc_peaktopeak(void)
{
  float max_peak = 0;
  float min_peak = 4095;
  float ptp = 0;
  char ptp_str[7];

  for (uint8_t u = 0; u < 240; u++)
  {
    if ((float)ring_buffer[u] > max_peak)
    {
      max_peak = (float)ring_buffer[u];
    }

    if ((float)ring_buffer[u] < min_peak)
    {
      min_peak = (float)ring_buffer[u];
    }
  }
  ptp = max_peak - min_peak;
  ptp = ptp * 3300 / 4095;

  pmi_string_float2str(ptp_str, 7, ptp, 7);

  ili9341_text_pos_set(6, 8);
  ili9341_str_clear(6, ILI9341_COLOR_WHITE);
  ili9341_text_pos_set(6, 8);
  ili9341_str_print(ptp_str, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

void period_avg(void)
{
  float period_value = 0;
  float C_value = 0;

  char period_str[5];
  char C_str[5];


  period_value = (float)(0.1 * (1 << zoom_pos) * (rising_value / period_count));

  C_value = (float)(period_value/27.72);

  pmi_string_float2str(period_str, 5, period_value, 5);
  pmi_string_float2str(C_str, 5, C_value, 5);

  if (period_value == 0)
  {
    ili9341_text_pos_set(8, 9);
    ili9341_str_clear(5, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(8, 9);
    ili9341_str_print("ZOOM!", ILI9341_COLOR_RED, ILI9341_COLOR_WHITE);
  }
  else
  { 

    ili9341_text_pos_set(8, 9);
    ili9341_str_clear(5, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(8, 9);
    ili9341_str_print(period_str, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

    ili9341_text_pos_set(8, 10);
    ili9341_str_clear(5, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(8, 10);
    ili9341_str_print(C_str, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
  }
}

void display_logic(void)
{
  calc_avg();
  calc_peaktopeak();
  period_avg();

  if (zoom_pos == 0)
  {
    ili9341_text_pos_set(6, 11);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 11);
    ili9341_str_print("24 ms", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

    ili9341_text_pos_set(6, 12);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 12);
    ili9341_str_print("4x", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
  }

  if (zoom_pos == 1)
  {
    ili9341_text_pos_set(6, 11);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 11);
    ili9341_str_print("48 ms", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

    ili9341_text_pos_set(6, 12);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 12);
    ili9341_str_print("2x", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
  }

  if (zoom_pos == 2)
  {
    ili9341_text_pos_set(6, 11);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 11);
    ili9341_str_print("96 ms", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

    ili9341_text_pos_set(6, 12);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 12);
    ili9341_str_print("1x", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
  }

  if (zoom_pos == 3)
  {
    ili9341_text_pos_set(6, 11);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 11);
    ili9341_str_print("192 ms", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

    ili9341_text_pos_set(6, 12);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 12);
    ili9341_str_print("0.5x", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
  }
  if (zoom_pos == 4)
  {

    ili9341_text_pos_set(6, 11);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 11);
    ili9341_str_print("384 ms", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

    ili9341_text_pos_set(6, 12);
    ili9341_str_clear(6, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(6, 12);
    ili9341_str_print("0.25x", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
  }
}

void display_return(void)
{

  ili9341_text_pos_set(2, 1);
  ili9341_str_print("OSCILLOSCOPE", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(1, 7);
  ili9341_str_print("AVG:", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(13, 7);
  ili9341_str_print("mV", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(1, 8);
  ili9341_str_print("P-P:", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(13, 8);
  ili9341_str_print("mV", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(1, 9);

  ili9341_text_pos_set(1, 9);
  ili9341_str_print("Period:", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(13, 9);
  ili9341_str_print("ms", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(1, 10);
  ili9341_str_print("C:", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(13, 10);
  ili9341_str_print("nF", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(1, 11);
  ili9341_str_print("Span:", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(1, 12);
  ili9341_str_print("Zoom:", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

int main(void)
{
  clocks_init_pmi();
  uart_init_nucusb(115200);

  OSC_gpio_init();
  OSC_ADC_init();
  OSC_tim2_init();
  ili9341_init(0);
  EXTI_init();

  ili9341_rect_fill(0, 0, 240, 320, ILI9341_COLOR_WHITE);
  display_return();

  {

    while (1)
    {

      draw_funciton();
    }
  }
}
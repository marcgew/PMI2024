#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ili9341.h>
#include <pmi_string.h>
#include <systick.h>
#include <ow.h>

/** 
 * Global Variables
 * These variables are used for various states and calculations across the program.
 */
volatile uint32_t ring_buffer[240]; 
volatile uint8_t ring_counter = 0; // count current loc in Ring Buffer
volatile uint8_t falling_detect = 0; // check if in falling edge currently
volatile uint8_t max_value = 0; 
volatile uint8_t middle_detect = 0; // check if middle of discharge curve 
volatile uint8_t draw_variable = 0; // check if we can draw currently
volatile uint8_t start_countdown = 0; // check if we can count 120 after middle 
volatile uint8_t middle_id = 0; // current pos of middle
volatile uint8_t RC_timing = 0;
volatile uint8_t RC_Id = 0;
volatile uint8_t treshhold_counter = 120; // count down until 0 to check if we have over 240 sampels
volatile uint8_t zoom_pos = 2; // current zoom pos 
uint32_t measure_count = 0; 


void OSC_gpio_init(void)
{
  RCC->IOPENR |= RCC_IOPENR_IOPCEN;  // open GPIOC Channel
  RCC->IOPENR |= RCC_IOPENR_GPIOBEN; // Enable SysClock for Port B

  GPIOC->MODER &= ~(GPIO_MODER_MODE4_1); // Set PC4 pins as output
  GPIOC->MODER |= GPIO_MODER_MODE4_0;

  GPIOC->MODER &= ~(GPIO_MODER_MODE8_1); // Set PC8 pins as output (LED D1)
  GPIOC->MODER |= GPIO_MODER_MODE8_0;

  GPIOC->MODER &= ~(GPIO_MODER_MODE6_1); // Set PC6 pins as output (LED D2)
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
  TIM6->ARR = 59 - 1;         // set to 17hz
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
  NVIC_SetPriority(EXTI0_1_IRQn, 3);
  NVIC_EnableIRQ(EXTI0_1_IRQn);
  NVIC_SetPriority(EXTI2_3_IRQn, 3);
  NVIC_EnableIRQ(EXTI2_3_IRQn);
}

/**
 * @brief Handles external interrupt for EXTI lines 0-1
 *        Decrease zoom_pos by 1 when button on PB1 is pressed
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
 *        Increase zoom_pos by 1 when button on PB2 is pressed
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

/**
 * @brief toggles PC4 every interrupt 
 *        
 */
void TIM6_DAC_IRQHandler(void)
{
  if (TIM6->SR & TIM_SR_UIF) // Check if update interrupt flag is set
  {
    GPIOC->ODR ^= GPIO_ODR_OD4;
    GPIOC->ODR ^= GPIO_ODR_OD8;
  }
  TIM6->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
}


/**
 * @brief  Manages ring_buffer and marks critical values on the charge curve,
 *         Stops measurements when enough data has been collected
 */
void TIM2_IRQHandler(void)
{
  // Check if the update interrupt flag is set
  if (TIM2->SR & TIM_SR_UIF) 
  {
    // Check if no drawing is currently happening
    if (draw_variable == 0) 
    {
      // Start the ADC measurement
      ADC1->CR |= ADC_CR_ADSTART; 

      // Wait until the conversion is complete
      while ((ADC1->ISR & ADC_ISR_EOC) == 0) 
      {
        // Wait for the ADC conversion to finish
      }

      // Store the ADC result in the ring buffer
      ring_buffer[ring_counter] = ADC1->DR;
      // Increment the measurement counter
      measure_count += 1;

      // If 120 measurements have been collected, begin the analysis
      if (measure_count >= 120) 
      {
        // Handle the edge case when the ring buffer wraps around to 0
        if (ring_counter == 0) 
        {
          // Check if a peak is detected
          if ((ring_buffer[239] >= 4095) && (ring_buffer[0] < 4095) && falling_detect == 0) 
          {
            falling_detect = 1;  // Peak detected
            max_value = 240;     // Set the peak value
          }
        }
        else 
        {
          // Check if the peak is detected between adjacent values
          if ((ring_buffer[ring_counter] < 4095) && (ring_buffer[ring_counter - 1] >= 4095) && falling_detect == 0) 
          {
            falling_detect = 1;  // Peak detected
            max_value = (ring_counter - 1); // Set the peak value
          }
        }

        // Check if the midpoint detection should be activated
        if (ring_counter == 0)
        {
          if (falling_detect == 1 && (ring_buffer[239] >= 2048) && (ring_buffer[0] < 2048) && start_countdown == 0)
          {
            start_countdown = 1; // Start the countdown
            middle_id = 0;       // Set the middle value index
          }
        }
        else
        {
          if (falling_detect == 1 && (ring_buffer[ring_counter] < 2048) && (ring_buffer[ring_counter - 1] >= 2048) && start_countdown == 0)
          {
            start_countdown = 1; // Start the countdown
            middle_id = ring_counter; // Set the middle value index
          }
        }

        // Check if the RC timing value should be detected
        if (ring_counter == 0)
        {
          if (falling_detect == 1 && (ring_buffer[239] >= 1506) && (ring_buffer[0] < 1506) && RC_timing == 0)
          {
            RC_timing = 1;  // RC time detected
            RC_Id = 0;      // Set the RC value
          }
        }
        else
        {
          if (falling_detect == 1 && (ring_buffer[ring_counter] < 1506) && (ring_buffer[ring_counter - 1] >= 1506) && RC_timing == 0)
          {
            RC_timing = 1;  // RC time detected
            RC_Id = ring_counter; // Set the RC value
          }
        }
      }

      // If the countdown has started, check if the threshold has been reached
      if (start_countdown == 1)
      {
        if (treshhold_counter <= 0)
        {
          draw_variable = 1;  // Allow drawing the data
        }
        else
        {
          treshhold_counter -= 1;  // Decrease the countdown
        }
      }

      // Update the ring buffer index (wrap around after 240 values)
      ring_counter = (ring_counter += 1) % 240;
    }
  }

  // Clear the update interrupt flag
  TIM2->SR &= ~TIM_SR_UIF; 
}
uint8_t iterator = 0;
uint8_t upper_limit = 0;
uint16_t top_marker;
/**
 * @brief  Draws the graph with markers of important values
 */
void draw_function(void)
{

  if (draw_variable == 1 && ring_buffer[max_value] > ring_buffer[middle_id]) // check if drawing possible
  {
    GPIOC->ODR |= GPIO_ODR_OD6; //toggle led

    ili9341_rect_fill(0, 55, 240, 100, ILI9341_COLOR_WHITE); // clear old graph
    if ((middle_id - 120) < 0)  // determine which part of the ring_buffer should be printed on which part of the display
    {
      iterator = middle_id + 120;
      upper_limit = 240 - iterator;
    }
    else
    {
      iterator = middle_id - 120;
      upper_limit = 240 - iterator;
    }

    for (uint8_t i = 0; i < upper_limit; i++) //account for the continuous recording of measurements when printing
    { 
       //prints each measurement 
      ili9341_pixel_set(i, 150 - ((ring_buffer[iterator]) / 65), ILI9341_COLOR_BLACK);
      ili9341_pixel_set(i, 149 - ((ring_buffer[iterator]) / 65), ILI9341_COLOR_BLACK);
      iterator += 1;
    }
    iterator = 0;

    for (uint8_t i = upper_limit; i < 240; i++)
    {
       //prints each measurement 
      ili9341_pixel_set(i, 150 - ((ring_buffer[iterator]) / 65), ILI9341_COLOR_BLACK);
      ili9341_pixel_set(i, 149 - ((ring_buffer[iterator]) / 65), ILI9341_COLOR_BLACK);
      iterator += 1;
    }

    GPIOC->ODR &= ~(GPIO_ODR_OD6);
    draw_marker();
    display_logic();
    zoom_function();
    // reset all variables for next iteration
    draw_variable = 0;
    falling_detect = 0;
    start_countdown = 0;
    RC_timing = 0;
    measure_count = 0;
    treshhold_counter = 120;
  }
  else if (draw_variable == 1) // also reset if it could not draw 
  {
    draw_variable = 0;
    falling_detect = 0;
    start_countdown = 0;
    RC_timing = 0;
    measure_count = 0;
    treshhold_counter = 120;
  }
}

/**
 * @brief  Changes sample times based on the selected zoom level
 */
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
uint8_t max_marker_position;
uint8_t tau_marker_position;
void draw_marker(void)
{

  if (max_value >= iterator)
  {
    max_marker_position = max_value - iterator;
    tau_marker_position = RC_Id - iterator;
  }
  else
  {
    max_marker_position = 240 - iterator + max_value;
    tau_marker_position = 240 - iterator + RC_Id;
  }

  ili9341_line_draw(max_marker_position, 80, max_marker_position, 140, ILI9341_COLOR_RED);
  ili9341_line_draw(tau_marker_position, 80, tau_marker_position, 140, ILI9341_COLOR_BLUE);
  
}

/**
 * @brief  Calculates average voltage of the capacitor and prints the result on the display
 */
void calc_avg(void)
{
  float avg = 0;
  char avg_str[7];

  for (uint8_t j = 0; j < 240; j++)
  {
    avg += (float)ring_buffer[j]; //add up all values of the ringbuffer
  }
  avg = avg / 240; // divide by 240 for avg 
  avg = avg * 3300 / 4095; //calculate to mV

 // display of AVG on Screen
  pmi_string_float2str(avg_str, 7, avg, 7);
  ili9341_text_pos_set(6, 7);
  ili9341_str_clear(6, ILI9341_COLOR_WHITE);
  ili9341_text_pos_set(6, 7);
  ili9341_str_print(avg_str, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}


/**
 * @brief  Calculates the difference between the highest and the lowest values in the waveforms,
 *         Prints the result on the display
 */
void calc_peaktopeak(void)
{
  float max_peak = 0; // reset max peak for iteration
  float min_peak = 4095; // reset min_peak for iteration
  float ptp = 0; // reset ptp value
  char ptp_str[7];

  for (uint8_t u = 0; u < 240; u++)
  {
    if ((float)ring_buffer[u] > max_peak) // find the biggest value in buffer 
    {
      max_peak = (float)ring_buffer[u];
    }

    if ((float)ring_buffer[u] < min_peak) // find the smallest value in buffer
    {
      min_peak = (float)ring_buffer[u];
    }
  }
  ptp = max_peak - min_peak; //calc ptp value
  ptp = ptp * 3300 / 4095;  // calculate to mV 

   // display of ptp on Screen
  pmi_string_float2str(ptp_str, 7, ptp, 7);
  ili9341_text_pos_set(6, 8);
  ili9341_str_clear(7, ILI9341_COLOR_WHITE);
  ili9341_text_pos_set(6, 8);
  ili9341_str_print(ptp_str, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
}

float RC_value; //RC_value and C_value are the same

/**
 * @brief  Calculates the RC and C Value of the graph on the display,
 *         Prints the result on the display
 */
void calc_RC_C(void)
{
  RC_value = 0; //RC_value and C_value are the same
  
  char RC_str[8];
  if ((RC_Id-max_value)>0)
  {
    RC_value=(float)(100*(1<<zoom_pos)  * (RC_Id - max_value));
  }
  else
  {
    RC_value =(float)(100*(1<<zoom_pos)  *(RC_Id + 240 - max_value));
  }

  pmi_string_float2str(RC_str, 8, RC_value, 8);
  
  ili9341_text_pos_set(5, 9);
  ili9341_str_clear(6, ILI9341_COLOR_WHITE);
  ili9341_text_pos_set(5, 9);
  ili9341_str_print(RC_str, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(5, 10);
  ili9341_str_clear(6, ILI9341_COLOR_WHITE);
  ili9341_text_pos_set(5, 10);
  ili9341_str_print(RC_str, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  
}

/**
 * @brief  Calls the calculation funtions,
 *         Prints zoom values and span values
 */
void display_logic(void)
{ 
  // call all calculate functions
  calc_avg();
  calc_peaktopeak();
  calc_RC_C();

  //check current zoom pos to print Span and Zoom
  if (zoom_pos == 0)
  { ili9341_text_pos_set(6, 11);
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
  {  ili9341_text_pos_set(6, 11);
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
  uint16_t no_jumper = 0;
  no_jumper = (RC_value / (1 << zoom_pos));  // detect no jumper

  if (no_jumper == 100)
  {
    ili9341_rect_fill(0, 55, 240, 100, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(1, 3);
    ili9341_str_print("Put that Jumper ", ILI9341_COLOR_RED, ILI9341_COLOR_WHITE);
    ili9341_text_pos_set(3, 4);
    ili9341_str_print("back! -.-", ILI9341_COLOR_RED, ILI9341_COLOR_WHITE);
    systick_delay_ms(2000);
    
  }
}

/**
 * @brief  Prints static text on the display (no changes when the program runs)
 */
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
  ili9341_str_print("RC:", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);


  ili9341_text_pos_set(13,9);
  ili9341_str_print("us", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);

  ili9341_text_pos_set(1, 10);
  ili9341_str_print("C:", ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
  
  
    ili9341_text_pos_set(13,10);
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
  OSC_tim6_init();
  OSC_tim2_init();
  ili9341_init(0);
  EXTI_init();

  ili9341_rect_fill(0, 0, 240, 320, ILI9341_COLOR_WHITE);
  display_return();
  {

    while (1)
    {
      draw_function();
    }
  }
}
#include "stm32f0xx.h"
#include "cmsis_os.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void StartThread(void const * argument);
static void CheckButtonThread(void const * argument);

void EXTI0_1_IRQHandler() {
    // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9 | GPIO_PIN_8);
    // for (volatile int i = 0; i < 1500000; i++) {}
    // // HAL_Delay(15);
    // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9 | GPIO_PIN_8);
    // EXTI->PR |= EXTI_PR_PR0; 
}

void TIM2_IRQHandler() {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9 | GPIO_PIN_8);
    
    TIM2->SR &= ~TIM_SR_UIF;
}

int main(void) {
    
    // Reset of all peripherals, Initializes the Flash interface and the Systick
    HAL_Init();
    // SystemClock_Config();
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN;

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;
    TIM2->PSC =  8000;
    TIM2->ARR = 250;
    TIM2->DIER |= TIM_DIER_UIE;
    // TIM2->SMCR
    

    // Enable peri clock for syscfg
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA; 

    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_SetPriority(EXTI0_1_IRQn, 3);
    NVIC_SetPriority(SysTick_IRQn, 2);

    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 3);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_8| GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    // TODO
    //
    GPIOC->AFR[0] &= ~GPIO_AFRL_AFRL6;
    GPIOC->AFR[0] &= ~GPIO_AFRL_AFRL7;
    
    TIM3->PSC = 99;
    TIM3->ARR = 100;
    TIM3->CCMR1 &= ~TIM_CCMR1_CC1S;
    TIM3->CCMR1 &= ~TIM_CCMR1_CC2S;
    TIM3->CCMR1 |= (TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);
    TIM3->CCMR1 |= (TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1);
    TIM3->CCMR1 &= ~(TIM_CCMR1_OC2M_0);
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
    TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIM3->CCR1 |= 10;
    TIM3->CCR2 |= 10;
    TIM3->CR1 |= TIM_CR1_CEN;

    TIM2->CR1 |= TIM_CR1_CEN;

    // PA0
    GPIOA->MODER |= (GPIO_MODE_INPUT);  
    GPIOA->PUPDR |= GPIO_PULLDOWN;
    GPIOA->OSPEEDR |= GPIO_SPEED_LOW;

    EXTI->IMR |= EXTI_PR_PR0;
    EXTI->RTSR |= EXTI_PR_PR0;

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);

    while (1) {
        HAL_Delay(400);
        // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
    }
}

/**
 *System Clock Configuration
 */
void SystemClock_Config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __SYSCFG_CLK_ENABLE();
}

/**
 * Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 */
  /**
   * PC8 - LD4 [blue led]
   * PC9 - LD3 [green led]
   */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

static int ledState = 0;

static void StartThread(void const * argument) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, 1);
    for(;;) {
        vTaskDelay(200);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, ledState);
    }
}

static void CheckButtonThread(void const * argument) {
    uint8_t newstate, state = 1;
    for (;;) {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != state) {
            vTaskDelay(20);
            newstate = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
            if ( newstate != state ) {
                state = newstate;
                if (newstate) {
                    ledState = !ledState;
                }
            }
        }
        vTaskDelay(20);
    }
}

extern void xPortSysTickHandler(void);

void SysTick_Handler(void) {
    // if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    //     xPortSysTickHandler();
    // }
    uint32_t ticks = HAL_GetTick();
    if (ticks % 200 == 0) {
        // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
    }
    HAL_IncTick();
}

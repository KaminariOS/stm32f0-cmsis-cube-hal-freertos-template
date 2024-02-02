#include "stm32f0xx.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_rcc.h"
#include "cmsis_os.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void StartThread(void const * argument);
static void CheckButtonThread(void const * argument);

/// Main function.  Called by the startup code.
int main(void)
{

    /* GPIOC Periph clock enable */
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN;
    HAL_Init();
    uint16_t pin_mask = GPIO_PIN_8 | GPIO_PIN_9;

    GPIOA->MODER |= (GPIO_MODE_INPUT);
    GPIOA->OSPEEDR |= (GPIO_SPEED_LOW);
    GPIOA->PUPDR |= (GPIO_PULLDOWN);

    for (uint32_t pinpos = 0x00; pinpos < 0x10; pinpos++) {
        uint32_t pos = ((uint32_t)0x01) << pinpos;
        uint32_t currentpin = pin_mask & pos;
        if (currentpin == pos) {
            GPIOC->MODER |= (GPIO_MODE_OUTPUT_PP << (pinpos * 2));
            GPIOC->OTYPER |= (0 << pinpos)   ;
            GPIOC->OSPEEDR |= (GPIO_SPEED_LOW << (pinpos * 2)); 
            GPIOC->PUPDR |= (GPIO_NOPULL << (pinpos * 2));
        }
    }

    int flip = 0;
    for (;;)
    {
      flip = ~flip; 
      if (flip) {
          // Turn off the first led
          GPIOC->BRR = GPIO_PIN_8;
          GPIOC->BSRRL = GPIO_PIN_9;
      } else {
          GPIOC->BRR = GPIO_PIN_9;
          GPIOC->BSRRL = GPIO_PIN_8;
      }
      // Turn on both LEDs
      // GPIOC->BSRR = GPIO_Pin_6 | GPIO_Pin_7;

      // Turn off the second LED and the on the first

      // delay(2400000);

      // Turn off first
      // GPIOC->BRR = 0x0100;

      HAL_Delay(240);
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
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
    HAL_IncTick();
}

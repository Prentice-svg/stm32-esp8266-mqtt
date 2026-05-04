#include "main.h"
#include "cmsis_os.h"
#include "dwt/bsp_dwt.h"
#include "led/bsp_led.h"
#include "usart/bsp_usart.h"
#include "key/bsp_key.h"
#include "i2c/bsp_i2c.h"
#include "oled/bsp_i2c_oled.h"
#include "light/bsp_light.h"
#include "dht11/bsp_dht11.h"
#include "aht20/bsp_aht20.h"
#include "bmp280/bsp_bmp280.h"
#include "esp8266/bsp_esp8266.h"
#include "esp8266/app_esp8266.h"

void SystemClock_Config(void);
void FREERTOS_Init(void);

static void I2C1_ScanDevices(void)
{
	uint8_t found_count = 0;

	printf(">>> I2C1 scan start on PB8=SCL PB9=SDA.\r\n");

	for (uint8_t addr = 1; addr < 0x7F; addr++)
	{
		if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 2, 10) == HAL_OK)
		{
			printf(">>> I2C device found: 0x%02X\r\n", addr);
			found_count++;
		}
	}

	printf(">>> I2C1 scan done, found %d device(s).\r\n", found_count);
}

int main(void)
{
  HAL_Init();
	NVIC_SetPriorityGrouping(0);

  /* Configure the system clock to 72 MHz */
  SystemClock_Config();

	DWT_Init();

	LED_Init();

	Debug_UART_Init();

	Key_Init();

	OLED_I2C1_Init();

	OLED_Init();

	I2C1_ScanDevices();

	printf(">>> AHT20 init: %s\r\n", AHT20_Init() ? "OK" : "FAIL");
	printf(">>> BMP280 init: %s\r\n", BMP280_Init() ? "OK" : "FAIL");

	DHT11_Init();

	Light_Init();

	uint16_t retry = 0;
	while (ESP8266_Preparation() == false)
	{
		retry++;
		OLED_ClearArea(0, 48, 128, 16);
		OLED_Printf(0, 48, OLED_8X16, "(Retry * %d)", retry);
		OLED_Update();
	}

	/* Filter out the data read for the first time */
	DHT11_RawData dht11_raw_data = {0};
	DHT11_ReadRawData(&dht11_raw_data);
	HAL_Delay(1000);

	/* Init scheduler */
	printf(">>> FreeRTOS kernel initialize...\r\n");
  osKernelInitialize();

	/* Call init function for freertos objects (in cmsis_os2.c) */
	printf(">>> FreeRTOS objects initialize...\r\n");
  FREERTOS_Init();

	/* Start scheduler */
	printf(">>> FreeRTOS scheduler start...\r\n");
  osKernelStart();
	printf(">>> FreeRTOS scheduler returned unexpectedly.\r\n");

	/* We should never get here as control is now taken by the scheduler */

  while (1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV1                    = 1
  *            PLLMUL                         = 9
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

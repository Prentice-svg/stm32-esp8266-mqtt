#include "dht11/bsp_dht11.h"
#include "dwt/bsp_dwt.h"

#define DHT11_DATA_PIN				GPIO_PIN_12
#define DHT11_DATA_GPIO_PORT	GPIOB

void DHT11_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = DHT11_DATA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_DATA_GPIO_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN, GPIO_PIN_SET);
}

static void DHT11_SetGPIOMode(uint32_t mode, uint32_t pull)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = DHT11_DATA_PIN;
  GPIO_InitStruct.Mode = mode;
  GPIO_InitStruct.Pull = pull;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_DATA_GPIO_PORT, &GPIO_InitStruct);
}

static bool DHT11_WaitPinState(GPIO_PinState state, uint16_t timeout_us)
{
	while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN) != state)
	{
		if (timeout_us-- == 0)
		{
			return false;
		}

		DWT_DelayUs(1);
	}

	return true;
}

static bool DHT11_ReadByte(uint8_t *byte)
{
	*byte = 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		if (DHT11_WaitPinState(GPIO_PIN_SET, 100) == false)
		{
			return false;
		}

		DWT_DelayUs(40);

		if (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN) == GPIO_PIN_SET)
		{
			*byte |= (0x80 >> i);

			if (DHT11_WaitPinState(GPIO_PIN_RESET, 100) == false)
			{
				return false;
			}
		}
	}

	return true;
}

HAL_StatusTypeDef DHT11_ReadRawData(DHT11_RawData *data)
{
	uint8_t retry, sum;

	DHT11_SetGPIOMode(GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
	HAL_GPIO_WritePin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN, GPIO_PIN_RESET);
	DWT_DelayMs(20);
	HAL_GPIO_WritePin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN, GPIO_PIN_SET);
	DWT_DelayUs(30);

	DHT11_SetGPIOMode(GPIO_MODE_INPUT, GPIO_PULLUP);

	retry = 0;
	while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN) == GPIO_PIN_SET)
	{
		retry++;
		if (retry > 100)
		{
			return HAL_ERROR;
		}
		DWT_DelayUs(1);
	}

	retry = 0;
	while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN) == GPIO_PIN_RESET)
	{
		retry++;
		if (retry > 100)
		{
			return HAL_ERROR;
		}
		DWT_DelayUs(1);
	}

	retry = 0;
	while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_PORT, DHT11_DATA_PIN) == GPIO_PIN_SET)
	{
		retry++;
		if (retry > 100)
		{
			return HAL_ERROR;
		}
		DWT_DelayUs(1);
	}

	if (DHT11_ReadByte(&data->humi_int) == false ||
			DHT11_ReadByte(&data->humi_deci) == false ||
			DHT11_ReadByte(&data->temp_int) == false ||
			DHT11_ReadByte(&data->temp_deci) == false ||
			DHT11_ReadByte(&data->check_sum) == false)
	{
		return HAL_ERROR;
	}

	sum = data->humi_int + data->humi_deci + data->temp_int + data->temp_deci;

	return (sum == data->check_sum) ? HAL_OK : HAL_ERROR;
}

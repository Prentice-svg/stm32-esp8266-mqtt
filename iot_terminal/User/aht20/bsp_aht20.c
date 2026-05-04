#include "aht20/bsp_aht20.h"
#include "i2c/bsp_i2c.h"

#define AHT20_ADDR				0x38
#define AHT20_TIMEOUT		100

static bool AHT20_WriteCommand(uint8_t cmd, uint8_t arg1, uint8_t arg2)
{
	uint8_t data[3] = {cmd, arg1, arg2};
	return HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDR << 1, data, sizeof(data), AHT20_TIMEOUT) == HAL_OK;
}

bool AHT20_Init(void)
{
	uint8_t status = 0;

	HAL_Delay(40);

	if (HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDR << 1, &status, 1, AHT20_TIMEOUT) != HAL_OK)
	{
		return false;
	}

	if ((status & 0x08) == 0)
	{
		if (AHT20_WriteCommand(0xBE, 0x08, 0x00) == false)
		{
			return false;
		}

		HAL_Delay(10);
	}

	return true;
}

bool AHT20_Read(float *temperature, float *humidity)
{
	uint8_t data[6];
	uint32_t raw_humidity;
	uint32_t raw_temperature;

	if (AHT20_WriteCommand(0xAC, 0x33, 0x00) == false)
	{
		return false;
	}

	HAL_Delay(80);

	if (HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDR << 1, data, sizeof(data), AHT20_TIMEOUT) != HAL_OK)
	{
		return false;
	}

	if ((data[0] & 0x80) != 0)
	{
		return false;
	}

	raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((uint32_t)data[3] >> 4);
	raw_temperature = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

	*humidity = (float)raw_humidity * 100.0f / 1048576.0f;
	*temperature = (float)raw_temperature * 200.0f / 1048576.0f - 50.0f;

	return true;
}

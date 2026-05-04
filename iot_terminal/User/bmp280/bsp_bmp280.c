#include "bmp280/bsp_bmp280.h"
#include "i2c/bsp_i2c.h"

#define BMP280_ADDR					0x77
#define BMP280_TIMEOUT			100

typedef struct
{
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
	int32_t t_fine;
} BMP280_CalibData;

static BMP280_CalibData bmp280_calib;

static bool BMP280_ReadRegs(uint8_t reg, uint8_t *data, uint16_t len)
{
	if (HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR << 1, &reg, 1, BMP280_TIMEOUT) != HAL_OK)
	{
		return false;
	}

	return HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR << 1, data, len, BMP280_TIMEOUT) == HAL_OK;
}

static bool BMP280_WriteReg(uint8_t reg, uint8_t value)
{
	uint8_t data[2] = {reg, value};
	return HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR << 1, data, sizeof(data), BMP280_TIMEOUT) == HAL_OK;
}

static uint16_t BMP280_U16(const uint8_t *data)
{
	return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static int16_t BMP280_S16(const uint8_t *data)
{
	return (int16_t)BMP280_U16(data);
}

bool BMP280_Init(void)
{
	uint8_t chip_id;
	uint8_t calib[24];

	if (BMP280_ReadRegs(0xD0, &chip_id, 1) == false || chip_id != 0x58)
	{
		return false;
	}

	if (BMP280_ReadRegs(0x88, calib, sizeof(calib)) == false)
	{
		return false;
	}

	bmp280_calib.dig_T1 = BMP280_U16(&calib[0]);
	bmp280_calib.dig_T2 = BMP280_S16(&calib[2]);
	bmp280_calib.dig_T3 = BMP280_S16(&calib[4]);
	bmp280_calib.dig_P1 = BMP280_U16(&calib[6]);
	bmp280_calib.dig_P2 = BMP280_S16(&calib[8]);
	bmp280_calib.dig_P3 = BMP280_S16(&calib[10]);
	bmp280_calib.dig_P4 = BMP280_S16(&calib[12]);
	bmp280_calib.dig_P5 = BMP280_S16(&calib[14]);
	bmp280_calib.dig_P6 = BMP280_S16(&calib[16]);
	bmp280_calib.dig_P7 = BMP280_S16(&calib[18]);
	bmp280_calib.dig_P8 = BMP280_S16(&calib[20]);
	bmp280_calib.dig_P9 = BMP280_S16(&calib[22]);

	if (BMP280_WriteReg(0xF5, 0xA0) == false)
	{
		return false;
	}

	if (BMP280_WriteReg(0xF4, 0x27) == false)
	{
		return false;
	}

	HAL_Delay(100);
	return true;
}

bool BMP280_Read(float *temperature, float *pressure_hpa)
{
	uint8_t data[6];
	int32_t adc_T;
	int32_t adc_P;
	int32_t var1;
	int32_t var2;
	int64_t p;

	if (BMP280_ReadRegs(0xF7, data, sizeof(data)) == false)
	{
		return false;
	}

	adc_P = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | ((int32_t)data[2] >> 4);
	adc_T = ((int32_t)data[3] << 12) | ((int32_t)data[4] << 4) | ((int32_t)data[5] >> 4);

	var1 = ((((adc_T >> 3) - ((int32_t)bmp280_calib.dig_T1 << 1))) * ((int32_t)bmp280_calib.dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1))) >> 12) *
		((int32_t)bmp280_calib.dig_T3)) >> 14;
	bmp280_calib.t_fine = var1 + var2;
	*temperature = (float)((bmp280_calib.t_fine * 5 + 128) >> 8) / 100.0f;

	var1 = ((int64_t)bmp280_calib.t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)bmp280_calib.dig_P6;
	var2 = var2 + ((var1 * (int64_t)bmp280_calib.dig_P5) << 17);
	var2 = var2 + (((int64_t)bmp280_calib.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)bmp280_calib.dig_P3) >> 8) + ((var1 * (int64_t)bmp280_calib.dig_P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)bmp280_calib.dig_P1) >> 33;

	if (var1 == 0)
	{
		return false;
	}

	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)bmp280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)bmp280_calib.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)bmp280_calib.dig_P7) << 4);

	*pressure_hpa = (float)p / 25600.0f;
	return true;
}

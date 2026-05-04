#include "aht20/bsp_aht20.h"
#include "bmp280/bsp_bmp280.h"
#include "light/bsp_light.h"
#include "types/app_sensor_data.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include <stdio.h>

extern osMessageQueueId_t UpdateDataQueueHandle;
extern osMessageQueueId_t ESP8266PackQueueHandle;
extern osMutexId_t I2C1BusMutexHandle;

void StartSensorTask(void *argument)
{
	float bmp_temperature;

	while (1)
	{
		SensorData *update_data = pvPortMalloc(sizeof(SensorData));
		if (update_data == NULL)
		{
			Error_Handler();
		}

		SensorData *ESP8266_data = pvPortMalloc(sizeof(SensorData));
		if (ESP8266_data == NULL)
		{
			Error_Handler();
		}

		bool aht20_ok = false;
		bool bmp280_ok = false;

		if (osMutexAcquire(I2C1BusMutexHandle, 500) == osOK)
		{
			aht20_ok = AHT20_Read(&update_data->temp_value, &update_data->humi_value);
			bmp280_ok = BMP280_Read(&bmp_temperature, &update_data->pressure_hpa);
			osMutexRelease(I2C1BusMutexHandle);
		}

		/* Get AHT20 temperature and humidity data */
		if (aht20_ok == true)
		{
			update_data->dht11_check_result = DHT11_DATA_OK;
		}
		else
		{
			update_data->dht11_check_result = DHT11_DATA_ERROR;
			update_data->temp_value = 0.0f;
			update_data->humi_value = 0.0f;
		}

		if (bmp280_ok == false)
		{
			update_data->pressure_hpa = 0.0f;
		}

		/* Get light data */
		update_data->light_percentage_value = Light_ReadPercentageValue();

		if (update_data->dht11_check_result == DHT11_DATA_OK)
		{
			printf(">>> Sensor OK: temp=%.1f humi=%.1f pressure=%.2fhPa light=%.2f%%\r\n",
				update_data->temp_value, update_data->humi_value, update_data->pressure_hpa, update_data->light_percentage_value);
		}
		else
		{
			printf(">>> Sensor ERROR: AHT20 read failed, pressure=%.2fhPa light=%.2f%%\r\n",
				update_data->pressure_hpa, update_data->light_percentage_value);
		}

		*ESP8266_data = *update_data;
		osMessageQueuePut(UpdateDataQueueHandle, &update_data, 0, osWaitForever);
		osMessageQueuePut(ESP8266PackQueueHandle, &ESP8266_data, 0, osWaitForever);

		osDelay(5000);
	}
}

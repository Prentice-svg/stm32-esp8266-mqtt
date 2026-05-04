#include "oled/bsp_i2c_oled.h"
#include "types/app_screen_type.h"
#include "types/app_sensor_data.h"
#include "cmsis_os2.h"

extern ScreenType global_screen_type;
extern DisplayData global_display_data;
extern osMutexId_t I2C1BusMutexHandle;

static void OLED_DisplayOverview(void)
{
	OLED_ShowString(0, 0, "[P1]Overview", OLED_8X16);

	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_Printf(0, 16, OLED_6X8, "T:%05.1fC  H:%05.1f%%", global_display_data.temp_value, global_display_data.humi_value);
		OLED_Printf(0, 24, OLED_6X8, "P:%07.2fhPa", global_display_data.pressure_hpa);
	}
	else
	{
		OLED_ShowString(0, 16, "AHT20/BMP ERR", OLED_8X16);
	}

	OLED_Printf(0, 40, OLED_6X8, "Light:%05.1f%%", global_display_data.light_percentage_value);
	OLED_ShowString(0, 56, "K1:Next K2:LED", OLED_6X8);
}

static void OLED_DisplayTempHumi(void)
{
	OLED_ShowString(0, 0, "[P2]Temp/Humi", OLED_8X16);

	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_Printf(0, 16, OLED_8X16, "T:%+06.2f C", global_display_data.temp_value);
		OLED_Printf(0, 32, OLED_8X16, "H:%06.2f %%", global_display_data.humi_value);
	}
	else
	{
		OLED_ShowString(0, 16, "DATA ERROR", OLED_8X16);
	}
}

static void OLED_DisplayPressure(void)
{
	OLED_ShowString(0, 0, "[P3]Pressure", OLED_8X16);

	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_Printf(0, 16, OLED_8X16, "%07.2f hPa", global_display_data.pressure_hpa);
		OLED_Printf(0, 32, OLED_6X8, "Max:%07.2f", global_display_data.pressure_hpa_max);
		OLED_Printf(0, 40, OLED_6X8, "Min:%07.2f", global_display_data.pressure_hpa_min);
	}
	else
	{
		OLED_ShowString(0, 16, "DATA ERROR", OLED_8X16);
	}
}

static void OLED_DisplayLight(void)
{
	OLED_ShowString(0, 0, "[P4]Light", OLED_8X16);
	OLED_Printf(0, 16, OLED_8X16, "Now: %06.2f %%", global_display_data.light_percentage_value);
	OLED_Printf(0, 32, OLED_8X16, "Max: %06.2f %%", global_display_data.light_percentage_max);
	OLED_Printf(0, 48, OLED_8X16, "Min: %06.2f %%", global_display_data.light_percentage_min);
}

typedef void (*OLEDScreenHandler)(void);

static const OLEDScreenHandler screen_handlers[SCREEN_TYPE_COUNT] =
{
	OLED_DisplayOverview,
	OLED_DisplayTempHumi,
	OLED_DisplayPressure,
	OLED_DisplayLight
};

void StartOLEDTask(void *argument)
{
	while (1)
	{
		if (osMutexAcquire(I2C1BusMutexHandle, 100) == osOK)
		{
			OLED_Clear();
			screen_handlers[global_screen_type]();
			OLED_Update();
			osMutexRelease(I2C1BusMutexHandle);
		}

		osDelay(300);
	}
}

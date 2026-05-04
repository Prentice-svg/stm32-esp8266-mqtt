#include "oled/bsp_i2c_oled.h"
#include "types/app_screen_type.h"
#include "types/app_sensor_data.h"
#include "cmsis_os2.h"

extern ScreenType global_screen_type;
extern DisplayData global_display_data;
extern osMutexId_t I2C1BusMutexHandle;

static void OLED_DrawHeader(const char *title)
{
	OLED_ShowString(0, 0, (char *)title, OLED_8X16);
	OLED_DrawLine(0, 15, 127, 15);
}

static void OLED_DrawFooter(const char *text)
{
	OLED_DrawLine(0, 55, 127, 55);
	OLED_ShowString(0, 56, (char *)text, OLED_6X8);
}

static uint8_t OLED_ClampPercent(float value)
{
	if (value < 0.0f)
	{
		return 0;
	}

	if (value > 100.0f)
	{
		return 100;
	}

	return (uint8_t)(value + 0.5f);
}

static void OLED_DisplayOverview(void)
{
	OLED_DrawHeader("P1 OVERVIEW");

	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_Printf(0, 19, OLED_6X8, "TEMP  %5.1f C", global_display_data.temp_value);
		OLED_Printf(0, 29, OLED_6X8, "HUMI  %5.1f %%", global_display_data.humi_value);
		OLED_Printf(0, 39, OLED_6X8, "PRES %7.2f", global_display_data.pressure_hpa);
	}
	else
	{
		OLED_ShowString(0, 22, "AHT20/BMP ERROR", OLED_6X8);
	}

	OLED_Printf(0, 49, OLED_6X8, "LIGHT %5.1f %%", global_display_data.light_percentage_value);
	OLED_DrawFooter("K1 NEXT   K2 LED");
}

static void OLED_DisplayTempHumi(void)
{
	OLED_DrawHeader("P2 TEMP HUMI");

	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_ShowString(0, 19, "TEMP", OLED_6X8);
		OLED_Printf(0, 27, OLED_8X16, "%5.1f C", global_display_data.temp_value);
		OLED_ShowString(0, 45, "HUMI", OLED_6X8);
		OLED_Printf(0, 53, OLED_6X8, "%5.1f %%RH", global_display_data.humi_value);
	}
	else
	{
		OLED_ShowString(0, 28, "SENSOR DATA ERROR", OLED_6X8);
	}
}

static void OLED_DisplayPressure(void)
{
	OLED_DrawHeader("P3 PRESSURE");

	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_ShowString(0, 19, "CURRENT", OLED_6X8);
		OLED_Printf(0, 27, OLED_8X16, "%7.2f", global_display_data.pressure_hpa);
		OLED_ShowString(88, 35, "hPa", OLED_6X8);
		OLED_Printf(0, 46, OLED_6X8, "MAX %7.2f", global_display_data.pressure_hpa_max);
		OLED_Printf(0, 56, OLED_6X8, "MIN %7.2f", global_display_data.pressure_hpa_min);
	}
	else
	{
		OLED_ShowString(0, 28, "SENSOR DATA ERROR", OLED_6X8);
	}
}

static void OLED_DisplayLight(void)
{
	uint8_t bar_width = OLED_ClampPercent(global_display_data.light_percentage_value);

	OLED_DrawHeader("P4 LIGHT");
	OLED_ShowString(0, 19, "LEVEL", OLED_6X8);
	OLED_Printf(0, 27, OLED_8X16, "%5.1f %%", global_display_data.light_percentage_value);
	OLED_DrawRectangle(0, 46, 102, 10, OLED_UNFILLED);
	OLED_DrawRectangle(2, 48, bar_width, 6, OLED_FILLED);
	OLED_Printf(106, 47, OLED_6X8, "%3d", bar_width);
	OLED_Printf(0, 57, OLED_6X8, "MAX %5.1f  MIN %5.1f", global_display_data.light_percentage_max, global_display_data.light_percentage_min);
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

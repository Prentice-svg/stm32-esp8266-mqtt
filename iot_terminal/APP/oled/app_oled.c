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

static void OLED_ShowLabelValue(int16_t label_x, int16_t y, char *label, int16_t value_x, char *format, float value)
{
	OLED_ShowChinese(label_x, y, label);
	OLED_Printf(value_x, y + 4, OLED_6X8, format, value);
}

static void OLED_DisplayOverview(void)
{
	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_ShowLabelValue(0, 0, "温度", 40, "%5.1fC", global_display_data.temp_value);
		OLED_ShowLabelValue(0, 16, "湿度", 40, "%5.1f%%", global_display_data.humi_value);
		OLED_ShowLabelValue(0, 32, "大气压", 56, "%6.1fhPa", global_display_data.pressure_hpa);
	}
	else
	{
		OLED_ShowString(0, 8, "AHT20/BMP ERROR", OLED_6X8);
	}

	OLED_ShowLabelValue(0, 48, "光强", 40, "%5.1f%%", global_display_data.light_percentage_value);
}

static void OLED_DisplayTempHumi(void)
{
	OLED_DrawHeader("P2");

	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_ShowChinese(0, 19, "温度");
		OLED_Printf(40, 19, OLED_8X16, "%5.1f C", global_display_data.temp_value);
		OLED_ShowChinese(0, 43, "湿度");
		OLED_Printf(40, 47, OLED_6X8, "%5.1f %%RH", global_display_data.humi_value);
	}
	else
	{
		OLED_ShowString(0, 28, "SENSOR DATA ERROR", OLED_6X8);
	}
}

static void OLED_DisplayPressure(void)
{
	OLED_DrawHeader("P3");

	if (global_display_data.dht11_check_result == DHT11_DATA_OK)
	{
		OLED_ShowChinese(0, 19, "大气压");
		OLED_Printf(50, 19, OLED_8X16, "%7.2f", global_display_data.pressure_hpa);
		OLED_ShowString(108, 35, "hPa", OLED_6X8);
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

	OLED_DrawHeader("P4");
	OLED_ShowChinese(0, 19, "光强");
	OLED_Printf(40, 19, OLED_8X16, "%5.1f %%", global_display_data.light_percentage_value);
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

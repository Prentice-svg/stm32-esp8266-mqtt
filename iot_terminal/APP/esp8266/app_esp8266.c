#include "esp8266/app_esp8266.h"
#include "esp8266/bsp_esp8266.h"
#include "esp8266/bsp_esp_at_port.h"
#include "types/app_sensor_data.h"
#include "types/app_publish_data.h"
#include "oled/bsp_i2c_oled.h"
#include "led/bsp_led.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"

#define ESP_AT_AP_SSID					"CMCC-etru"
#define ESP_AT_AP_PWD					"4us46hfx"

#define CONNECT_ID							0

#define MQTT_SERVER_IP					"mqtts.heclouds.com"
#define MQTT_SERVER_PORT				1883

#define DEVICE_NAME							"stm32-esp32c3"
#define PRODUCT_ID							"45Z51DlJn3"
#define TOKEN										"version=2018-10-31&res=products%2F45Z51DlJn3%2Fdevices%2Fstm32-esp32c3&et=1809399480&method=md5&sign=QAfZy6%2F2oq090K%2B043BVrA%3D%3D"

#define MQTT_TOPIC_POST					"$sys/45Z51DlJn3/stm32-esp32c3/thing/property/post"
#define MQTT_TOPIC_POST_REPLY		"$sys/45Z51DlJn3/stm32-esp32c3/thing/property/post/reply"
#define MQTT_TOPIC_SET					"$sys/45Z51DlJn3/stm32-esp32c3/thing/property/set"
#define MQTT_TOPIC_SET_REPLY		"$sys/45Z51DlJn3/stm32-esp32c3/thing/property/set_reply"

extern osSemaphoreId_t ESP8266ReceiveSemaphoreHandle;
extern osMessageQueueId_t ESP8266PackQueueHandle;
extern osMessageQueueId_t ESP8266RespondQueueHandle;
extern osMessageQueueId_t ESP8266SendQueueHandle;

extern uint8_t ESP8266_RxBuff[];
extern uint16_t ESP8266_RxBuff_Size;

static const char *ESP8266_GetWifiStateText(ConnWifiStateTypeDef state)
{
	switch (state)
	{
		case CONNECT_TIMEOUT:
			return "TIMEOUT";
		case PASSWORD_ERROR:
			return "PWD ERR";
		case TARGET_AP_NOT_FOUND:
			return "AP MISS";
		case CONNECT_FAIL:
			return "JOIN FAIL";
		default:
			return "UNKNOWN";
	}
}

bool ESP8266_Preparation(void)
{
	ESP8266_EnablePrintResponse();
	printf(">>> %s is powered on for initialization...\r\n", ESP_AT_MODULE_NAME);

	ESP8266_Init();
	printf(">>> %s has been initialized and is now ready to receive...\r\n", ESP_AT_MODULE_NAME);
	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[1/9] OK", OLED_8X16);
	OLED_ShowString(0, 16, "ESP32-C3 Init", OLED_8X16);
	OLED_Update();
	HAL_Delay(300);

	printf(">>> %s is undergoing software reset...\r\n", ESP_AT_MODULE_NAME);
	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[2/9]", OLED_8X16);
	OLED_ShowString(0, 16, "ESP32-C3 Reset", OLED_8X16);
	OLED_Update();
	if (ESP8266_SoftReset_WaitReady() == true)
	{
		printf(">>> %s has been reset.\r\n", ESP_AT_MODULE_NAME);
		OLED_ShowString(48, 0, "OK", OLED_8X16);
		OLED_Update();
		HAL_Delay(300);
	}
	else
	{
		printf(">>> %s reset timeout!\r\n", ESP_AT_MODULE_NAME);
		return false;
	}

	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[3/9]", OLED_8X16);
	OLED_ShowString(0, 16, "ESP32-C3 Test", OLED_8X16);
	OLED_Update();
	if (ESP8266_Test() == true)
	{
		printf(">>> %s has passed the test.\r\n", ESP_AT_MODULE_NAME);
		OLED_ShowString(48, 0, "OK", OLED_8X16);
		OLED_Update();
		HAL_Delay(300);
	}
	else
	{
		printf(">>> %s is unavailable!\r\n", ESP_AT_MODULE_NAME);
		return false;
	}

	printf(">>> %s is disabling UART hardware flow control...\r\n", ESP_AT_MODULE_NAME);
	if (ESP8266_ConfigUARTNoFlow() == false)
	{
		printf(">>> %s failed to disable UART hardware flow control!\r\n", ESP_AT_MODULE_NAME);
		OLED_ClearArea(0, 0, 128, 48);
		OLED_ShowString(0, 0, "UART Flow Fail", OLED_8X16);
		OLED_Update();
		return false;
	}

	printf(">>> %s is configuring Wi-Fi defaults...\r\n", ESP_AT_MODULE_NAME);
	if (ESP8266_ConfigWiFiDefaults() == false)
	{
		printf(">>> %s failed to configure Wi-Fi defaults!\r\n", ESP_AT_MODULE_NAME);
		OLED_ClearArea(0, 0, 128, 48);
		OLED_ShowString(0, 0, "WiFi Cfg Fail", OLED_8X16);
		OLED_Update();
		return false;
	}

	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[4/9]", OLED_8X16);
	OLED_ShowString(0, 16, "ESP32-C3 Set", OLED_8X16);
	OLED_ShowString(0, 32, "STA Mode", OLED_8X16);
	OLED_Update();
	if (ESP8266_SetNetMode(STA) == true)
	{
		printf(">>> %s has been set to STA mode.\r\n", ESP_AT_MODULE_NAME);
		OLED_ShowString(48, 0, "OK", OLED_8X16);
		OLED_Update();
		HAL_Delay(300);
	}
	else
	{
		printf(">>> %s failed to set the working mode!\r\n", ESP_AT_MODULE_NAME);
		return false;
	}

	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[5/9]", OLED_8X16);
	OLED_ShowString(0, 16, "Scan Wi-Fi", OLED_8X16);
	OLED_Update();
	if (ESP8266_ScanWifi(ESP_AT_AP_SSID) == false)
	{
		printf(">>> Wi-Fi AP not found: %s.\r\n", ESP_AT_AP_SSID);
		OLED_ClearArea(0, 0, 128, 48);
		OLED_ShowString(0, 0, "[5/9] FAIL", OLED_8X16);
		OLED_ShowString(0, 16, "AP NOT FOUND", OLED_8X16);
		OLED_Update();
		HAL_Delay(2000);
		return false;
	}

	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[5/9]", OLED_8X16);
	OLED_ShowString(0, 16, "ESP32-C3 Join", OLED_8X16);
	OLED_ShowString(0, 32, "Wi-Fi", OLED_8X16);
	OLED_Update();
	ConnWifiStateTypeDef wifi_state = ESP8266_ConnectWifi(ESP_AT_AP_SSID, ESP_AT_AP_PWD);
	if (wifi_state == CONNECT_OK)
	{
		printf(">>> Wi-Fi connected.\r\n");
		OLED_ShowString(48, 0, "OK", OLED_8X16);
		OLED_Update();
		HAL_Delay(300);
	}
	else
	{
		printf(">>> Wi-Fi failed to connect: %s.\r\n", ESP8266_GetWifiStateText(wifi_state));
		OLED_ClearArea(0, 0, 128, 48);
		OLED_ShowString(0, 0, "[5/9] FAIL", OLED_8X16);
		OLED_ShowString(0, 16, (char *)ESP8266_GetWifiStateText(wifi_state), OLED_8X16);
		OLED_Update();
		HAL_Delay(2000);
		return false;
	}

	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[6/9]", OLED_8X16);
	OLED_ShowString(0, 16, "Config MQTT User", OLED_8X16);
	OLED_ShowString(0, 32, "Information", OLED_8X16);
	OLED_Update();
	if (ESP8266_ConfigMQTTUser(CONNECT_ID, true, DEVICE_NAME, PRODUCT_ID, TOKEN) == true)
	{
		printf(">>> MQTT user information configured successfully.\r\n");
		OLED_ShowString(48, 0, "OK", OLED_8X16);
		OLED_Update();
		HAL_Delay(300);
	}
	else
	{
		printf(">>> Failed to configure MQTT user information!\r\n");
		return false;
	}

	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[7/9]", OLED_8X16);
	OLED_ShowString(0, 16, "ESP32-C3 MQTT", OLED_8X16);
	OLED_ShowString(0, 32, "MQTT Server", OLED_8X16);
	OLED_Update();
	if (ESP8266_ConnectMQTTServer(MQTT_SERVER_IP, MQTT_SERVER_PORT, CONNECT_ID, true) == true)
	{
		printf(">>> Connected to the MQTT server successfully.\r\n");
		OLED_ShowString(48, 0, "OK", OLED_8X16);
		OLED_Update();
		HAL_Delay(300);
	}
	else
	{
		printf(">>> Failed to connect to the MQTT server!\r\n");
		return false;
	}

	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[8/9]", OLED_8X16);
	OLED_ShowString(0, 16, "Subscribe Topic", OLED_8X16);
	OLED_ShowString(0, 32, "\"post/reply\"", OLED_8X16);
	OLED_Update();
	if (ESP8266_MQTTSubscribe(CONNECT_ID, MQTT_TOPIC_POST_REPLY, MQTT_QOS_0) == true)
	{
		printf(">>> Subscribed to MQTT topic \"post/reply\" successfully.\r\n");
		OLED_ShowString(48, 0, "OK", OLED_8X16);
		OLED_Update();
		HAL_Delay(300);
	}
	else
	{
		printf(">>> Failed to subscribe to MQTT topic \"post/reply\"!\r\n");
		return false;
	}

	OLED_ClearArea(0, 0, 128, 48);
	OLED_ShowString(0, 0, "[9/9]", OLED_8X16);
	OLED_ShowString(0, 16, "Subscribe Topic", OLED_8X16);
	OLED_ShowString(0, 32, "\"set\"", OLED_8X16);
	OLED_Update();
	if (ESP8266_MQTTSubscribe(CONNECT_ID, MQTT_TOPIC_SET, MQTT_QOS_0) == true)
	{
		printf(">>> Subscribed to MQTT topic \"set\" successfully.\r\n\r\n");
		OLED_ShowString(48, 0, "OK", OLED_8X16);
		OLED_Update();
		HAL_Delay(300);
	}
	else
	{
		printf(">>> Failed to subscribe to MQTT topic \"set\"!\r\n");
		return false;
	}

	ESP8266_EnablePrintResponse();

	return true;
}

void StartESP8266PackTask(void *argument)
{
	uint32_t payload_id = 1;

	while (1)
	{
		SensorData *sensor_data;

		osMessageQueueGet(ESP8266PackQueueHandle, &sensor_data, 0, osWaitForever);

		MQTTPublishData *publish_data = pvPortMalloc(sizeof(MQTTPublishData));
		if (publish_data == NULL)
		{
			Error_Handler();
		}

		memcpy(publish_data->topic, MQTT_TOPIC_POST, sizeof(MQTT_TOPIC_POST));

		if (sensor_data->dht11_check_result == DHT11_DATA_OK)
		{
			snprintf(publish_data->payload, sizeof(publish_data->payload),
				"{\\\"id\\\":\\\"%d\\\"\\,\\\"version\\\":\\\"1.0\\\"\\,\\\"params\\\":{\\\"Humi\\\":{\\\"value\\\":%.1f}\\,\\\"Temp\\\":{\\\"value\\\":%.1f}\\,\\\"led\\\":{\\\"value\\\":%s}\\,\\\"Light\\\":{\\\"value\\\":%.1f}\\,\\\"Press\\\":{\\\"value\\\":%.2f}}}",
			payload_id++, sensor_data->humi_value, sensor_data->temp_value, LED_IsOn(LED_ONBOARD) ? "true" : "false", sensor_data->light_percentage_value, sensor_data->pressure_hpa);
		}
		else
		{
			printf(">>> DHT11 invalid: post light and led only.\r\n");
			snprintf(publish_data->payload, sizeof(publish_data->payload),
				"{\\\"id\\\":\\\"%d\\\"\\,\\\"version\\\":\\\"1.0\\\"\\,\\\"params\\\":{\\\"Light\\\":{\\\"value\\\":%.1f}\\,\\\"led\\\":{\\\"value\\\":%s}}}",
			payload_id++, sensor_data->light_percentage_value, LED_IsOn(LED_ONBOARD) ? "true" : "false");
		}

		osMessageQueuePut(ESP8266SendQueueHandle, &publish_data, 0, osWaitForever);

		vPortFree(sensor_data);
		sensor_data = NULL;
	}
}

void StartESP8266ReceiveTask(void *argument)
{
	while (1)
	{
		osSemaphoreAcquire(ESP8266ReceiveSemaphoreHandle, osWaitForever);

		if (strstr((char *)ESP8266_RxBuff, "+MQTTSUBRECV:") != NULL &&
				strstr((char *)ESP8266_RxBuff, "thing/property/set") != NULL)
		{
			printf(">>> MQTT set message received.\r\n");

			char *message = pvPortMalloc(sizeof(char) * (ESP8266_RxBuff_Size + 1));
			if (message == NULL)
			{
				Error_Handler();
			}

			memcpy(message, ESP8266_RxBuff, ESP8266_RxBuff_Size);
			message[ESP8266_RxBuff_Size] = '\0';
			osMessageQueuePut(ESP8266RespondQueueHandle, &message, 0, osWaitForever);
		}
	}
}

void StartESP8266RespondTask(void *argument)
{
	while (1)
	{
		char *message;
		char payload_id[64] = {0};

		osMessageQueueGet(ESP8266RespondQueueHandle, &message, 0, osWaitForever);
		char *json_start = strstr(message, "{");

		if (json_start != NULL)
		{
			char *id_start = strstr(json_start, "\"id\"");
			if (id_start != NULL)
			{
				char *colon = strchr(id_start, ':');
				if (colon != NULL)
				{
					char *value = colon + 1;
					while (*value == ' ' || *value == '\"')
					{
						value++;
					}

					uint8_t index = 0;
					while (*value != '\0' && *value != '\"' && *value != ',' && *value != '}' && index < sizeof(payload_id) - 1)
					{
						payload_id[index++] = *value++;
					}
					payload_id[index] = '\0';
				}
			}

			if (strstr(json_start, "\"led\":true") != NULL || strstr(json_start, "\"led\":{\"value\":true") != NULL)
			{
				LED_On(LED_ONBOARD);
				printf(">>> Property set: led=true.\r\n");
			}
			else if (strstr(json_start, "\"led\":false") != NULL || strstr(json_start, "\"led\":{\"value\":false") != NULL)
			{
				LED_Off(LED_ONBOARD);
				printf(">>> Property set: led=false.\r\n");
			}

			if (payload_id[0] != '\0')
			{
				MQTTPublishData *publish_data = pvPortMalloc(sizeof(MQTTPublishData));
				if (publish_data == NULL)
				{
					Error_Handler();
				}

				memcpy(publish_data->topic, MQTT_TOPIC_SET_REPLY, sizeof(MQTT_TOPIC_SET_REPLY));
				snprintf(publish_data->payload, sizeof(publish_data->payload), "{\\\"id\\\":\\\"%s\\\"\\,\\\"code\\\":200\\,\\\"msg\\\":\\\"success\\\"}", payload_id);
				printf(">>> Property set reply id=%s.\r\n", payload_id);
				osMessageQueuePut(ESP8266SendQueueHandle, &publish_data, 0, osWaitForever);
			}
			else
			{
				printf(">>> Property set: missing id, cannot reply.\r\n");
			}
		}

		vPortFree(message);
		message = NULL;
	}
}

void StartESP8266SendTask(void *argument)
{
	while (1)
	{
		MQTTPublishData *publish_data;

		osMessageQueueGet(ESP8266SendQueueHandle, &publish_data, 0, osWaitForever);

		printf(">>> MQTT post: topic=%s payload_len=%d\r\n", publish_data->topic, strlen(publish_data->payload));
		if (ESP8266_MQTTPublish(CONNECT_ID, publish_data->topic, publish_data->payload, MQTT_QOS_0, false) == true)
		{
			printf(">>> MQTT post OK.\r\n");
		}
		else
		{
			printf(">>> MQTT post command failed.\r\n");
		}

		vPortFree(publish_data);
		publish_data = NULL;
		osDelay(500);
	}
}

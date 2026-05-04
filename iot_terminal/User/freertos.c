#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "types/app_screen_type.h"
#include "types/app_sensor_data.h"
#include "types/app_publish_data.h"

const osPriority_t SENSOR_TASK_PRIORITY						= osPriorityRealtime;
const osPriority_t ESP8266_RECEIVE_TASK_PRIORITY	= osPriorityHigh2;
const osPriority_t ESP8266_RESPOND_TASK_PRIORITY	= osPriorityHigh1;
const osPriority_t ESP8266_SEND_TASK_PRIORITY			= osPriorityHigh;
const osPriority_t UPDATE_DATA_TASK_PRIORITY			= osPriorityAboveNormal;
const osPriority_t ESP8266_PACK_TASK_PRIORITY			= osPriorityAboveNormal;
const osPriority_t OLED_TASK_PRIORITY							= osPriorityNormal;
const osPriority_t LED_TASK_PRIORITY							= osPriorityNormal;
const osPriority_t KEY_TASK_PRIORITY							= osPriorityLow;

osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes =
{
  .name = "KeyTask",
  .stack_size = 128 * 4,
  .priority = KEY_TASK_PRIORITY,
};

osThreadId_t OLEDTaskHandle;
const osThreadAttr_t OLEDTask_attributes =
{
  .name = "OLEDTask",
  .stack_size = 256 * 4,
  .priority = OLED_TASK_PRIORITY,
};

osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes =
{
  .name = "SensorTask",
  .stack_size = 256 * 4,
  .priority = SENSOR_TASK_PRIORITY,
};

osThreadId_t UpdateDataTaskHandle;
const osThreadAttr_t UpdateDataTask_attributes =
{
  .name = "UpdateDataTask",
  .stack_size = 128 * 4,
  .priority = UPDATE_DATA_TASK_PRIORITY,
};

osThreadId_t LEDTaskHandle;
const osThreadAttr_t LEDTask_attributes =
{
  .name = "LEDTask",
  .stack_size = 128 * 4,
  .priority = LED_TASK_PRIORITY,
};

osThreadId_t ESP8266PackTaskHandle;
const osThreadAttr_t ESP8266PackTask_attributes =
{
  .name = "ESP8266PackTask",
  .stack_size = 256 * 4,
  .priority = ESP8266_PACK_TASK_PRIORITY,
};

osThreadId_t ESP8266ReceiveTaskHandle;
const osThreadAttr_t ESP8266ReceiveTask_attributes =
{
  .name = "ESP8266ReceiveTask",
  .stack_size = 256 * 4,
  .priority = ESP8266_RECEIVE_TASK_PRIORITY,
};

osThreadId_t ESP8266RespondTaskHandle;
const osThreadAttr_t ESP8266RespondTask_attributes =
{
  .name = "ESP8266RespondTask",
  .stack_size = 256 * 4,
  .priority = ESP8266_RESPOND_TASK_PRIORITY,
};

osThreadId_t ESP8266SendTaskHandle;
const osThreadAttr_t ESP8266SendTask_attributes =
{
  .name = "ESP8266SendTask",
  .stack_size = 384 * 4,
  .priority = ESP8266_SEND_TASK_PRIORITY,
};

osMessageQueueId_t UpdateDataQueueHandle;
const osMessageQueueAttr_t UpdateDataQueue_attributes =
{
  .name = "UpdateDataQueue"
};

osMessageQueueId_t ESP8266PackQueueHandle;
const osMessageQueueAttr_t ESP8266PackQueue_attributes =
{
  .name = "ESP8266PackQueue"
};

osMessageQueueId_t ESP8266RespondQueueHandle;
const osMessageQueueAttr_t ESP8266RespondQueue_attributes =
{
  .name = "ESP8266RespondQueue"
};

osMessageQueueId_t ESP8266SendQueueHandle;
const osMessageQueueAttr_t ESP8266SendQueue_attributes =
{
  .name = "ESP8266SendQueue"
};

osSemaphoreId_t ESP8266ReceiveSemaphoreHandle;
const osSemaphoreAttr_t ESP8266ReceiveSemaphore_attributes =
{
  .name = "ESP8266ReceiveSemaphore"
};

osMutexId_t I2C1BusMutexHandle;
const osMutexAttr_t I2C1BusMutex_attributes =
{
  .name = "I2C1BusMutex"
};

extern void StartKeyTask(void *argument);
extern void StartOLEDTask(void *argument);
extern void StartSensorTask(void *argument);
extern void StartUpdateDataTask(void *argument);
extern void StartLEDTask(void *argument);
extern void StartESP8266PackTask(void *argument);
extern void StartESP8266ReceiveTask(void *argument);
extern void StartESP8266RespondTask(void *argument);
extern void StartESP8266SendTask(void *argument);

void FREERTOS_Init(void)
{
	/* Create the semaphores(s) */
  ESP8266ReceiveSemaphoreHandle = osSemaphoreNew(1, 0, &ESP8266ReceiveSemaphore_attributes);
  printf(">>> RTOS semaphore: %s\r\n", ESP8266ReceiveSemaphoreHandle == NULL ? "FAIL" : "OK");

	I2C1BusMutexHandle = osMutexNew(&I2C1BusMutex_attributes);
	printf(">>> RTOS mutex I2C1: %s\r\n", I2C1BusMutexHandle == NULL ? "FAIL" : "OK");
	if (I2C1BusMutexHandle == NULL)
	{
		Error_Handler();
	}

	/* Create the queue(s) */
  UpdateDataQueueHandle = osMessageQueueNew (16, sizeof(SensorData*), &UpdateDataQueue_attributes);
	ESP8266PackQueueHandle = osMessageQueueNew (16, sizeof(SensorData*), &ESP8266PackQueue_attributes);
	ESP8266RespondQueueHandle = osMessageQueueNew (16, sizeof(char*), &ESP8266RespondQueue_attributes);
	ESP8266SendQueueHandle = osMessageQueueNew (16, sizeof(MQTTPublishData*), &ESP8266SendQueue_attributes);
	printf(">>> RTOS queues: %s\r\n",
		(UpdateDataQueueHandle == NULL || ESP8266PackQueueHandle == NULL ||
		 ESP8266RespondQueueHandle == NULL || ESP8266SendQueueHandle == NULL) ? "FAIL" : "OK");

	/* Create the thread(s) */
  KeyTaskHandle = osThreadNew(StartKeyTask, NULL, &KeyTask_attributes);
	printf(">>> RTOS task Key: %s\r\n", KeyTaskHandle == NULL ? "FAIL" : "OK");
	if (KeyTaskHandle == NULL)
	{
		Error_Handler();
	}

	OLEDTaskHandle = osThreadNew(StartOLEDTask, NULL, &OLEDTask_attributes);
	printf(">>> RTOS task OLED: %s\r\n", OLEDTaskHandle == NULL ? "FAIL" : "OK");
	if (OLEDTaskHandle == NULL)
	{
		Error_Handler();
	}

	SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);
	printf(">>> RTOS task Sensor: %s\r\n", SensorTaskHandle == NULL ? "FAIL" : "OK");
	if (SensorTaskHandle == NULL)
	{
		Error_Handler();
	}

	UpdateDataTaskHandle = osThreadNew(StartUpdateDataTask, NULL, &UpdateDataTask_attributes);
	printf(">>> RTOS task UpdateData: %s\r\n", UpdateDataTaskHandle == NULL ? "FAIL" : "OK");
	if (UpdateDataTaskHandle == NULL)
	{
		Error_Handler();
	}

	LEDTaskHandle = osThreadNew(StartLEDTask, NULL, &LEDTask_attributes);
	printf(">>> RTOS task LED: %s\r\n", LEDTaskHandle == NULL ? "FAIL" : "OK");
	if (LEDTaskHandle == NULL)
	{
		Error_Handler();
	}

	ESP8266PackTaskHandle = osThreadNew(StartESP8266PackTask, NULL, &ESP8266PackTask_attributes);
	printf(">>> RTOS task ESP Pack: %s\r\n", ESP8266PackTaskHandle == NULL ? "FAIL" : "OK");
	if (ESP8266PackTaskHandle == NULL)
	{
		Error_Handler();
	}

	ESP8266ReceiveTaskHandle = osThreadNew(StartESP8266ReceiveTask, NULL, &ESP8266ReceiveTask_attributes);
	printf(">>> RTOS task ESP Receive: %s\r\n", ESP8266ReceiveTaskHandle == NULL ? "FAIL" : "OK");
	if (ESP8266ReceiveTaskHandle == NULL)
	{
		Error_Handler();
	}

	ESP8266RespondTaskHandle = osThreadNew(StartESP8266RespondTask, NULL, &ESP8266RespondTask_attributes);
	printf(">>> RTOS task ESP Respond: %s\r\n", ESP8266RespondTaskHandle == NULL ? "FAIL" : "OK");
	if (ESP8266RespondTaskHandle == NULL)
	{
		Error_Handler();
	}

	ESP8266SendTaskHandle = osThreadNew(StartESP8266SendTask, NULL, &ESP8266SendTask_attributes);
	printf(">>> RTOS task ESP Send: %s\r\n", ESP8266SendTaskHandle == NULL ? "FAIL" : "OK");
	if (ESP8266SendTaskHandle == NULL)
	{
		Error_Handler();
	}
}

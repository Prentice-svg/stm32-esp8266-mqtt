#ifndef __APP_PUBLISH_DATA_H
#define __APP_PUBLISH_DATA_H

typedef struct
{
	char payload[384];
	char topic[64];
} MQTTPublishData;

#endif

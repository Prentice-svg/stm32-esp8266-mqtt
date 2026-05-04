#ifndef __BSP_AHT20_H
#define __BSP_AHT20_H

#include "main.h"
#include <stdbool.h>

bool AHT20_Init(void);
bool AHT20_Read(float *temperature, float *humidity);

#endif

#ifndef __BSP_BMP280_H
#define __BSP_BMP280_H

#include "main.h"
#include <stdbool.h>

bool BMP280_Init(void);
bool BMP280_Read(float *temperature, float *pressure_hpa);

#endif

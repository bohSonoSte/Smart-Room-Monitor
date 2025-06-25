#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include "msp.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//init the temperature sensor
void _temperatureSensorInit();

//return the temperature in Celsius
float getTemperature();

#endif

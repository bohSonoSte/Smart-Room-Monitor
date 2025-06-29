#ifndef __LIGHTSENSOR_H__
#define __LIGHTSENSOR_H__

#include "msp.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//init the lux sensor
void  _lightSensorInit();

//return the lux value
float getLux();

#endif

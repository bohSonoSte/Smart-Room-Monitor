#include "temperature.h"
#include "../HAL_I2C.h" //I2C communication drivers
#include "../HAL_TMP006.h" //Sensor specific driver TMP006

void _temperatureSensorInit()
{
    /* Temperature Sensor initialization */
    /* Initialize I2C communication */
    Init_I2C_GPIO();
    I2C_init();
    /* Initialize TMP006 temperature sensor */
    TMP006_init();
    __delay_cycles(100000);

}

float getTemperature(){
    float temp;
    temp = TMP006_getTemp();
    return temp = (temp - 32.0) * (5.0/9.0) - 6.3;
}

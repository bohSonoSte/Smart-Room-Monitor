#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "pir.h"
#include <stdio.h>

#define PIR_PIN         GPIO_PORT_P2, GPIO_PIN3
#define PIR_LED         GPIO_PORT_P1, GPIO_PIN0

void initPir(){
    // Configures the pin P6.4 as input (PIR sensor)
        GPIO_setAsInputPin(PIR_PIN);

        // Configures the pin P1.0 as output (LED)
        GPIO_setAsOutputPin(PIR_LED);
        GPIO_setOutputLowOnPin(PIR_LED);

}

bool PIR_detect(){
    if(GPIO_getInputPinValue(PIR_PIN) == GPIO_INPUT_PIN_HIGH)
            {
                // Turn on the LED
                GPIO_setOutputHighOnPin(PIR_LED);
                return true;
            }
            else
            {
                // Shut off the LED
                GPIO_setOutputLowOnPin(PIR_LED);
                return false;
            }
}

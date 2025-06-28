#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "pir.h"
#include <stdio.h>

#define PIR_PIN         GPIO_PORT_P2, GPIO_PIN3
#define PIR_LED         GPIO_PORT_P1, GPIO_PIN0

void initPir(){
    // Configura il pin P6.4 come input (sensore PIR)
        GPIO_setAsInputPin(PIR_PIN);

        // Configura il pin P1.0 come output (LED)
        GPIO_setAsOutputPin(PIR_LED);
        GPIO_setOutputLowOnPin(PIR_LED);

}

bool PIR_detect(){
    if(GPIO_getInputPinValue(PIR_PIN) == GPIO_INPUT_PIN_HIGH)
            {
                // Accendi il LED
                GPIO_setOutputHighOnPin(PIR_LED);
                return true;
            }
            else
            {
                // Spegni il LED
                GPIO_setOutputLowOnPin(PIR_LED);
                return false;
            }
}

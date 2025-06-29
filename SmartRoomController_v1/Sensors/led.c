#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include "lightsensor.h"
#include <stdio.h>

#define PWM_PERIOD      1000

typedef enum {
    LED_OFF,
    LED_DIM,
    LED_COOL_WHITE,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
} LEDColor;

static LEDColor currentColor = LED_OFF;
static bool ledActive = false;
float lux;

uint16_t red, green, blue = PWM_PERIOD;


void _initPWM(void) {
    // Red - P2.6 (TA0.3)
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    // Green - P2.4 (TA0.1)
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    // Blue - P5.6 (TA2.1)
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);

    Timer_A_generatePWM(TIMER_A0_BASE, &(Timer_A_PWMConfig){
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_64,
        PWM_PERIOD,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        0
    });
    Timer_A_generatePWM(TIMER_A0_BASE, &(Timer_A_PWMConfig){
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_64,
        PWM_PERIOD,
        TIMER_A_CAPTURECOMPARE_REGISTER_3,
        TIMER_A_OUTPUTMODE_RESET_SET,
        0
    });
    Timer_A_generatePWM(TIMER_A2_BASE, &(Timer_A_PWMConfig){
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_64,
        PWM_PERIOD,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        0
    });
}

// Modify setColor function to handle different color modes
void setColor(LEDColor color) {
    //uint16_t red, green, blue;

    switch(color) {
        case LED_DIM:
            // Usa il valore lux già letto (da aggiornare separatamente)
                        if (lux >= 1000.0) {
                            red = green = blue = PWM_PERIOD;
                        } else if (lux <= 10.0) {
                            red = PWM_PERIOD;
                            green = (uint16_t)(PWM_PERIOD * 0.6);
                            blue = (uint16_t)(PWM_PERIOD * 0.2);
                        } else {
                            float t = (lux - 10.0) / (1000.0 - 10.0);
                            red = PWM_PERIOD;
                            green = (uint16_t)(PWM_PERIOD * (0.6 + 0.4 * t));
                            blue = (uint16_t)(PWM_PERIOD * t);
                        }
                        break;
        case LED_COOL_WHITE:
            red = PWM_PERIOD;
            green = PWM_PERIOD;
            blue = PWM_PERIOD;
            break;
        case LED_RED:
            red = PWM_PERIOD;
            green = 0;
            blue = 0;
            break;
        case LED_GREEN:
            red = 0;
            green = PWM_PERIOD;
            blue = 0;
            break;
        case LED_BLUE:
            red = 0;
            green = 0;
            blue = PWM_PERIOD;
            break;
        case LED_OFF:
        default:
            red = green = blue = 0;
            break;
    }

    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1, green);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, red);
    Timer_A_setCompareValue(TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1, blue);
}

// Modify updateLED function
void updateLED(uint8_t option) {
    switch(option) {
        case 0: // Toggle LED
            ledActive = !ledActive;
            if (ledActive) {
                setColor(currentColor);
            } else {
                setColor(LED_OFF);
            }
            break;
        case 1: // Led Dim
        case 2: // Cool White
        case 3: // Red
        case 4: // Green
        case 5: // Blue
            {
                LEDColor newColor = option; // Options match enum values
                //if (newColor != currentColor) {
                    currentColor = newColor;
                    //if (ledActive) {
                        setColor(currentColor);
                        ledActive = 1;
                    //}
                //}
            }
            break;

        default:
            break;
    }
}

void updateWarmWhite() {
    if(currentColor == LED_DIM && ledActive) {
        lux = getLux(); // Leggi il valore attuale della luce
        setColor(LED_DIM); // Ricalcola il colore
    }
}

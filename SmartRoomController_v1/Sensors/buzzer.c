#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "led.h"


const Timer_A_UpModeConfig upConfigBuzzer = {
    TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK
    TIMER_A_CLOCKSOURCE_DIVIDER_12,         // Divisore
    20000,                                  // Periodo
    TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disabilita interrupt timer
    TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,    // Disabilita interrupt CCR0
    TIMER_A_DO_CLEAR                        // Clear timer
};

Timer_A_CompareModeConfig compareConfigBuzzer = {
    TIMER_A_CAPTURECOMPARE_REGISTER_4,      // Usa CCR4
    TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,
    TIMER_A_OUTPUTMODE_TOGGLE_SET,          // Modalità PWM
    10000                                    // Duty cycle iniziale (25%)
};

void _buzzerInit() {
    setColor(LED_OFF);
    /* Configura P2.7 come TA1.4 (secondary function) */
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P2,
        GPIO_PIN7,
        GPIO_PRIMARY_MODULE_FUNCTION);  // TA1.4

    /* Configura Timer_A1 */
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfigBuzzer);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    /* Inizializza CCR4 su TA1 */
    Timer_A_initCompare(TIMER_A0_BASE, &compareConfigBuzzer);
}

void stopBuzzer() {
    // Ferma solo il PWM del buzzer senza influenzare i LED
    Timer_A_stop(TIMER_A0_BASE);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
}

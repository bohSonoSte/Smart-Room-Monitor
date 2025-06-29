#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "accellerometer.h"

// Buffer per i risultati ADC (X, Y, Z)
static volatile uint16_t resultsBuffer[3] = {0};

// Valori di riferimento a riposo (con filtraggio)
static float baseline[3] = {0};

// Fattore di smoothing per il baseline (0.1 = 10% di ogni nuova lettura)
#define SMOOTHING_FACTOR 0.1f

// Soglia dinamica per la rilevazione del movimento
#define MOVEMENT_THRESHOLD_PERCENT 5.0f  // 5% di variazione

// Conteggio letture per stabilizzazione
#define STABILIZATION_READINGS 50

// Stato di calibrazione
static bool isCalibrated = false;

// Macro per il valore assoluto
#define ABS(x) ((x) < 0 ? -(x) : (x))

void Accelerometer_init(void) {
    // Configurazione pin come ingressi analogici
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                              GPIO_PIN0 | GPIO_PIN2,
                                              GPIO_TERTIARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN1,
                                              GPIO_TERTIARY_MODULE_FUNCTION);

    // Inizializzazione ADC
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC,
                    ADC_PREDIVIDER_64,
                    ADC_DIVIDER_8,
                    0);

    // Configurazione memoria ADC
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM2, true);
    ADC14_configureConversionMemory(ADC_MEM0,
                                   ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                   ADC_INPUT_A14,
                                   ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM1,
                                   ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                   ADC_INPUT_A13,
                                   ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM2,
                                   ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                   ADC_INPUT_A11,
                                   ADC_NONDIFFERENTIAL_INPUTS);

    // Abilitazione interrupt
    ADC14_enableInterrupt(ADC_INT2);
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    // Configurazione timer automatico
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    // Reset calibrazione
    isCalibrated = false;
}

bool isMovementDetected(void) {
    if (!isCalibrated) {
        return false; // Non restituire movimento durante calibrazione
    }

    // Calcola la variazione percentuale per ogni asse
    float deltaX = ABS(100.0f * (resultsBuffer[0] - baseline[0]) / baseline[0]);
    float deltaY = ABS(100.0f * (resultsBuffer[1] - baseline[1]) / baseline[1]);
    float deltaZ = ABS(100.0f * (resultsBuffer[2] - baseline[2]) / baseline[2]);

    // Verifica se almeno un asse supera la soglia
    return (deltaX > MOVEMENT_THRESHOLD_PERCENT) ||
           (deltaY > MOVEMENT_THRESHOLD_PERCENT) ||
           (deltaZ > MOVEMENT_THRESHOLD_PERCENT);
}

// Funzione per la calibrazione continua
void updateCalibration(void) {
    static uint8_t readingCount = 0;

    if (readingCount < STABILIZATION_READINGS) {
        // Fase iniziale: calibrazione aggressiva
        baseline[0] = (baseline[0] * readingCount + resultsBuffer[0]) / (readingCount + 1);
        baseline[1] = (baseline[1] * readingCount + resultsBuffer[1]) / (readingCount + 1);
        baseline[2] = (baseline[2] * readingCount + resultsBuffer[2]) / (readingCount + 1);
        readingCount++;
    } else {
        // Fase operativa: aggiornamento continuo con smoothing
        baseline[0] = (1.0f - SMOOTHING_FACTOR) * baseline[0] + SMOOTHING_FACTOR * resultsBuffer[0];
        baseline[1] = (1.0f - SMOOTHING_FACTOR) * baseline[1] + SMOOTHING_FACTOR * resultsBuffer[1];
        baseline[2] = (1.0f - SMOOTHING_FACTOR) * baseline[2] + SMOOTHING_FACTOR * resultsBuffer[2];
    }

    if (readingCount == STABILIZATION_READINGS && !isCalibrated) {
        isCalibrated = true; // Marca la fine della calibrazione iniziale
    }
}

// Interrupt Handler per l'ADC
void ADC14_IRQHandler(void) {
    uint64_t status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if (status & ADC_INT2) {
        // Salva i nuovi valori
        resultsBuffer[0] = ADC14_getResult(ADC_MEM0); // X
        resultsBuffer[1] = ADC14_getResult(ADC_MEM1); // Y
        resultsBuffer[2] = ADC14_getResult(ADC_MEM2); // Z

        // Aggiorna la calibrazione
        updateCalibration();
    }
}

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <math.h>
#include "accellerometer.h"

// Indirizzo I2C dell'MMA8452Q
#define MMA8452Q_I2C_ADDRESS 0x1D

// Registri accelerometro
#define MMA8452Q_STATUS 0x00
#define MMA8452Q_OUT_X_MSB 0x01
#define MMA8452Q_CTRL_REG1 0x2A
#define MMA8452Q_XYZ_DATA_CFG 0x0E

// Soglie per il rilevamento
#define EARTHQUAKE_THRESHOLD 0.3f  // 0.3g (circa 3 m/s²)
#define SHAKE_DURATION 10           // Campioni consecutivi sopra soglia

void initAccelerometer(void) {
    // Inizializza I2C
    I2C_initMaster(EUSCI_B0_BASE, &(eUSCI_I2C_MasterConfig){
        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,
        3000000,  // SMCLK a 3MHz (modificare secondo la configurazione)
        EUSCI_B_I2C_SET_DATA_RATE_400KBPS,
        0,
        EUSCI_B_I2C_NO_AUTO_STOP
    });

    I2C_setSlaveAddress(EUSCI_B0_BASE, MMA8452Q_I2C_ADDRESS);
    I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    I2C_enableModule(EUSCI_B0_BASE);

    // Configura accelerometro
    uint8_t config[2];

    // 1. Metti in standby per configurare
    config[0] = MMA8452Q_CTRL_REG1;
    config[1] = 0x00; // Disabilita
    I2C_masterSendMultiByteStart(EUSCI_B0_BASE, config[0]);
    I2C_masterSendMultiByteFinish(EUSCI_B0_BASE, config[1]);

    // 2. Configura fondo scala ±4g
    config[0] = MMA8452Q_XYZ_DATA_CFG;
    config[1] = 0x01; // ±4g
    I2C_masterSendMultiByteStart(EUSCI_B0_BASE, config[0]);
    I2C_masterSendMultiByteFinish(EUSCI_B0_BASE, config[1]);

    // 3. Riattiva a 100Hz
    config[0] = MMA8452Q_CTRL_REG1;
    config[1] = 0x01; // Active mode + 100Hz
    I2C_masterSendMultiByteStart(EUSCI_B0_BASE, config[0]);
    I2C_masterSendMultiByteFinish(EUSCI_B0_BASE, config[1]);
}

void readAccelerometer(float *x, float *y, float *z) {
    uint8_t txData = MMA8452Q_OUT_X_MSB;
    uint8_t rxData[6];

    // Avvia lettura
    I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    I2C_masterSendSingleByte(EUSCI_B0_BASE, txData);

    // Leggi 6 byte (X, Y, Z)
    I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_MODE);
    uint8_t i = 0;
    for(i = 0; i < 6; i++) {
        rxData[i] = I2C_masterReceiveSingleByte(EUSCI_B0_BASE);
    }

    // Converti in valori g (14-bit, ±4g)
    *x = ((int16_t)((rxData[0] << 8) | rxData[1])) / 4096.0f;
    *y = ((int16_t)((rxData[2] << 8) | rxData[3])) / 4096.0f;
    *z = ((int16_t)((rxData[4] << 8) | rxData[5])) / 4096.0f;
}

bool isEarthquakeDetected() {
    static uint8_t shakeCount = 0;
    float x, y, z;

    readAccelerometer(&x, &y, &z);

    // Calcola l'accelerazione risultante (rimuovendo 1g per la gravità)
    float resultant = sqrtf(x*x + y*y + (z-1.0f)*(z-1.0f));

    if(resultant > EARTHQUAKE_THRESHOLD) {
        shakeCount++;
        if(shakeCount >= SHAKE_DURATION) {
            shakeCount = 0;
            return true;
        }
    } else {
        if(shakeCount > 0) shakeCount--;
    }

    return false;
}

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "HAL_I2C.h"
#include "HAL_OPT3001.h"
#include "HAL_TMP006.h"
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"

#include "Sensors/temperature.h"
#include "Sensors/lightSensor.h"
#include "Sensors/display.h"
#include "Sensors/led.h"
#include "Sensors/rtc.h"
#include "Sensors/buzzer.h"
#include "Sensors/accellerometer.h"
#include "Sensors/pir.h"

#include <stdio.h>

// Definizioni pin
#define BUTTON_DOWN       GPIO_PORT_P3, GPIO_PIN5   // S2 (DOWN button)
#define BUTTON_UP     GPIO_PORT_P5, GPIO_PIN1   // S1 (UP button)
#define BUTTON_SELECT   GPIO_PORT_P4, GPIO_PIN1
#define Btn GPIO_PORT_P1, GPIO_PIN1
#define PWM_PERIOD      1000

#define LED_PIN GPIO_PORT_P2, GPIO_PIN6  // LED rosso del BoosterPack
#define ALARM_LED_PIN GPIO_PORT_P1, GPIO_PIN0  // LED rosso del BoosterPack

#define IDLE_TIMEOUT_SECONDS 30
#define TIMER_FREQUENCY 750000  // SMCLK / 16


// Stati del menu
typedef enum {
    MENU_MAIN,
    MENU_TEMPERATURE,
    MENU_LIGHT,
    MENU_MOTION,
    MENU_CLOCK,
    MENU_LED,
    TEMP_ALERT
} MenuState;

// Struttura per gli elementi del menu
typedef struct {
    const char* name;
    MenuState targetState;
} MenuItem;

// Variabili globali
Graphics_Context g_sContext;
float lux, temp;
bool motionDetected;
bool sameDataDisplay;
bool showedAlarm = false;
MenuState currentState = MENU_MAIN;
MenuState lastState = MENU_MAIN;
uint8_t currentSelection = 0;

uint8_t ledMenuSelection = 0;
const uint8_t ledMenuSize = 7;

// Elementi del menu principale
MenuItem mainMenu[] = {
    {"Temperature", MENU_TEMPERATURE},
    {"Light Sensor", MENU_LIGHT},
    {"Motion Sensor", MENU_MOTION},
    {"Clock", MENU_CLOCK},
    {"Led", MENU_LED}
};
uint8_t mainMenuSize = sizeof(mainMenu) / sizeof(MenuItem);



// Inizializzazioni hardware (come nei tuoi esempi)
void _hwInit(void) {
    WDT_A_holdTimer();
    Interrupt_disableMaster();
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    // Configurazione clock
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // Configurazione pulsanti
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_UP);
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_DOWN);
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_SELECT);
    GPIO_setAsInputPinWithPullUpResistor(Btn);

    // Configurazione led alert
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);  // Imposta P1.0 come output
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);  // Inizialmente spento


    // Altre inizializzazioni
    _graphicsInit();
    initPir();
    _lightSensorInit();
    _temperatureSensorInit();
    _initPWM();
    initRTC();

}

// Gestione input utente
void handleInput() {
    static uint8_t lastUpState = 1;
    static uint8_t lastDownState = 1;
    static uint8_t lastSelectState = 1;
    static uint8_t ledMenuSelection = 0; // Track selection in LED menu
    const uint8_t ledMenuSize = 7; // Number of options in LED menu

    uint8_t currentUpState = GPIO_getInputPinValue(BUTTON_UP);
    uint8_t currentDownState = GPIO_getInputPinValue(BUTTON_DOWN);
    uint8_t currentSelectState = GPIO_getInputPinValue(BUTTON_SELECT);

    // Pulsante UP
    if (currentUpState == 0 && lastUpState == 1) {
        __delay_cycles(10000); // Debounce
        Timer_A_stopTimer(TIMER_A1_BASE);
        Timer_A_clearTimer(TIMER_A1_BASE);
        Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

        if (currentState == MENU_MAIN) {
            if (currentSelection > 0) currentSelection--;
            else currentSelection = mainMenuSize - 1;
            drawMainMenu(currentSelection);
        }
        else if (currentState == MENU_LED) {
            if (ledMenuSelection > 0) ledMenuSelection--;
            else ledMenuSelection = ledMenuSize - 1;
            drawLedScreen(false, ledMenuSelection);
            __delay_cycles(50000);
        }
    }

    // Pulsante DOWN
    if (currentDownState == 0 && lastDownState == 1) {
        __delay_cycles(10000); // Debounce
        Timer_A_stopTimer(TIMER_A1_BASE);
        Timer_A_clearTimer(TIMER_A1_BASE);
        Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

        if (currentState == MENU_MAIN) {
            if (currentSelection < mainMenuSize - 1) currentSelection++;
            else currentSelection = 0;
            drawMainMenu(currentSelection);
        }
        else if (currentState == MENU_LED) {
            if (ledMenuSelection < ledMenuSize - 1) ledMenuSelection++;
            else ledMenuSelection = 0;
            drawLedScreen(false, ledMenuSelection);
            __delay_cycles(50000);
        }
    }

    // Pulsante SELECT
    if (currentSelectState == 0 && lastSelectState == 1) {
        __delay_cycles(10000); // Debounce
        Timer_A_stopTimer(TIMER_A1_BASE);
        Timer_A_clearTimer(TIMER_A1_BASE);
        Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

        if (currentState == MENU_MAIN) {
            currentState = mainMenu[currentSelection].targetState;
            switch(currentState) {
                case MENU_TEMPERATURE: drawTemperatureScreen(0); updateWarmWhite(); break;
                case MENU_LIGHT: drawLightScreen(0); updateWarmWhite(); break;
                case MENU_MOTION: drawMotionScreen(0); updateWarmWhite(); break;
                case MENU_CLOCK: drawClockScreen(sameDataDisplay); updateWarmWhite(); break;
                case MENU_LED:
                    ledMenuSelection = 0; // Reset to first option when entering
                    updateWarmWhite();
                    drawLedScreen(0, ledMenuSelection);
                    break;
                default: break;
            }
        }else if (currentState == TEMP_ALERT) {
            // Solo nasconde la schermata, ma il LED resta acceso
            stopBuzzer();
            currentState = MENU_MAIN;
            drawMainMenu(currentSelection);
        }
        else if (currentState == MENU_LED) {
            if (ledMenuSelection == ledMenuSize - 1) { // "Back" option
                currentState = MENU_MAIN;
                updateWarmWhite();
                drawMainMenu(currentSelection);
            } else {
                updateLED(ledMenuSelection); // Handle LED control selection
                updateWarmWhite();
                drawLedScreen(0, ledMenuSelection);
                __delay_cycles(50000);
            }
        }
        else {
            currentState = MENU_MAIN;
            drawMainMenu(currentSelection);
        }
    }

    lastUpState = currentUpState;
    lastDownState = currentDownState;
    lastSelectState = currentSelectState;
}

void toggleAlarmLed(bool stato){
    if (stato) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);  // Accende il LED
    } else {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);    // Spegne il LED
    }
}


int main(void) {
    MAP_WDT_A_holdTimer(); // Disabilita il Watchdog

    _hwInit();

    drawMainMenu(currentSelection);

    // Modifica nel main loop
    while (1) {
        lastState = currentState;
        PIR_detect();

        // Gestione allarme temperatura
        float currentTemp = getTemperature();

        if (currentTemp > 40) {
            //toggleAlarmLed(true);  // Accendi il LED se temp > 25
            if (!showedAlarm) {
                currentState = TEMP_ALERT;
                _buzzerInit();
                showedAlarm = true;
                sameDataDisplay = false; // Forza l'aggiornamento della schermata
            }
        } else {
            //toggleAlarmLed(false);  // Spegni il LED
            if (showedAlarm) {
                // Se stiamo uscendo dallo stato di allarme
                if (currentState == TEMP_ALERT) {
                    stopBuzzer();
                    currentState = MENU_MAIN;
                    drawMainMenu(currentSelection);
                }
                showedAlarm = false;
            }
        }

        handleInput();

        // Aggiorna il flag sameDataDisplay
        sameDataDisplay = (lastState == currentState);

        updateWarmWhite();
        updateTimeFromRTC();

        // Aggiorna gli schermi
        if(currentState == TEMP_ALERT){
            drawTemperatureScreen_Alert(sameDataDisplay);
        }
        if (currentState != MENU_MAIN) {
            switch(currentState) {
                case MENU_TEMPERATURE:
                    //drawTemperatureScreen(sameDataDisplay);
                    drawEnhancedTemperatureScreen(sameDataDisplay, getTemperature());
                    break;
                case MENU_LIGHT:
                    drawLightScreen(sameDataDisplay);
                    break;
                case MENU_MOTION:
                    drawMotionScreen(sameDataDisplay);
                    break;
                case MENU_CLOCK:
                    drawClockScreen(sameDataDisplay);
                    break;
                case MENU_LED:
                    // Nessun disegno specifico qui
                    break;
                default: break;
            }
            updateWarmWhite();
        }
        __delay_cycles(200000);
    }
}

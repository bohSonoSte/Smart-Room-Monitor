#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "HAL_I2C.h"
#include "HAL_OPT3001.h"
#include "HAL_TMP006.h"
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"

#include "images/termometro_8bit.h"
#include "images/logoSRM.h"
#include "images/lampadina.h"
#include "images/rgbimage.h"

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


#define PIR_LED         GPIO_PORT_P1, GPIO_PIN0
#define LED_PIN GPIO_PORT_P2, GPIO_PIN6  // LED rosso del BoosterPack
#define LED_TERREMOTO GPIO_PORT_P2, GPIO_PIN0
#define LED_TEMP_ALTA GPIO_PORT_P2, GPIO_PIN1
#define LED_TEMP_BASSA GPIO_PORT_P2, GPIO_PIN2

#define ALARM_LED_PIN GPIO_PORT_P1, GPIO_PIN0  // LED rosso del BoosterPack

#define IDLE_TIMEOUT_SECONDS 30
#define TIMER_FREQUENCY 750000  // SMCLK / 16

#define EDIT_MODE_NONE 0
#define EDIT_MODE_HOUR 1
#define EDIT_MODE_MINUTE 2
#define EDIT_MODE_DAY 3
#define EDIT_MODE_MONTH 4
#define EDIT_MODE_YEAR 5

uint8_t editMode = EDIT_MODE_NONE;


// Stati del menu
typedef enum {
    MENU_SPLASH,
    MENU_QUADRANT,
    MENU_MAIN,
    MENU_TEMPERATURE,
    MENU_LIGHT,
    MENU_RGB,
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
uint8_t quadrantSelection = 0;

uint8_t ledMenuSelection = 0;
const uint8_t ledMenuSize = 7;
extern const Graphics_Image  dropLogoPalette;


// Elementi del menu principale
MenuItem mainMenu[] = {
    {"Temperature", MENU_TEMPERATURE},
    {"Light Sensor", MENU_LIGHT},
    {"Led", MENU_LED},
    {"Clock", MENU_CLOCK}
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
    GPIO_setAsOutputPin(LED_TERREMOTO);
    GPIO_setAsOutputPin(LED_TEMP_ALTA);
    GPIO_setAsOutputPin(LED_TEMP_BASSA);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);  // Inizialmente spento
    GPIO_setOutputLowOnPin(LED_TERREMOTO);
    GPIO_setOutputLowOnPin(LED_TEMP_ALTA);
    GPIO_setOutputLowOnPin(LED_TEMP_BASSA);


    // Altre inizializzazioni
    _graphicsInit();
    initPir();
    _lightSensorInit();
    _temperatureSensorInit();
    _initPWM();
    initRTC();
    Accelerometer_init();

}

// Gestione input utente
void handleInput() {
    static uint8_t lastUpState = 1;
    static uint8_t lastDownState = 1;
    static uint8_t lastSelectState = 1;
    static uint8_t ledMenuSelection = 0;
    const uint8_t ledMenuSize = 7;

    uint8_t currentUpState = GPIO_getInputPinValue(BUTTON_UP);
    uint8_t currentDownState = GPIO_getInputPinValue(BUTTON_DOWN);
    uint8_t currentSelectState = GPIO_getInputPinValue(BUTTON_SELECT);

    // Pulsante UP
    if (currentUpState == 0 && lastUpState == 1) {
        __delay_cycles(10000); // Debounce

        if (currentState == MENU_QUADRANT)
        {
            if (quadrantSelection > 0)
                quadrantSelection--;
            else
                quadrantSelection = 3;  // da 0 a 3 (4 quadranti)
            drawQuadrantMenuWithSelection(quadrantSelection);
        }
        else if (currentState == MENU_LED) {
            if (ledMenuSelection > 0) ledMenuSelection--;
            else ledMenuSelection = ledMenuSize - 1;
            drawLedScreen(false, ledMenuSelection);
            __delay_cycles(50000);
        }
        else if (currentState == MENU_CLOCK)
        {
            if (editMode == EDIT_MODE_NONE)
            {
                // Nella modalità normale, UP/DOWN cambiano il campo da modificare
                if (editMode > EDIT_MODE_HOUR)
                    editMode--;
                else
                    editMode = EDIT_MODE_YEAR;
                drawClockScreen(false, editMode);
            }
            else
            {
                // Nella modalità modifica, UP/DOWN cambiano il valore
                switch (editMode)
                {
                case EDIT_MODE_HOUR:
                    currentTime.hours = (currentTime.hours + 1) % 24;
                    break;
                case EDIT_MODE_MINUTE:
                    currentTime.minutes = (currentTime.minutes + 1) % 60;
                    break;
                case EDIT_MODE_DAY:
                    currentTime.day = (currentTime.day % 31) + 1; // Semplificato, senza controlli sul mese
                    break;
                case EDIT_MODE_MONTH:
                    currentTime.month = (currentTime.month % 12) + 1;
                    break;
                case EDIT_MODE_YEAR:
                    currentTime.year++;
                    if (currentTime.year > 2100)
                        currentTime.year = 2000;
                    break;
                }
                drawClockScreen(false, editMode);
            }
        }
    }

    // Pulsante DOWN
    if (currentDownState == 0 && lastDownState == 1) {
        __delay_cycles(10000); // Debounce

        if (currentState == MENU_QUADRANT)
        {
            if (quadrantSelection < 3)
                quadrantSelection++;
            else
                quadrantSelection = 0;
            drawQuadrantMenuWithSelection(quadrantSelection);
        }
        else if (currentState == MENU_LED) {
            if (ledMenuSelection < ledMenuSize - 1) ledMenuSelection++;
            else ledMenuSelection = 0;
            drawLedScreen(false, ledMenuSelection);
            __delay_cycles(50000);
        }
        else if (currentState == MENU_CLOCK)
        {
            if (editMode == EDIT_MODE_NONE)
            {
                // Nella modalità normale, UP/DOWN cambiano il campo da modificare
                if (editMode < EDIT_MODE_YEAR)
                    editMode++;
                else
                    editMode = EDIT_MODE_HOUR;
                drawClockScreen(false, editMode);
            }
            else
            {
                // Nella modalità modifica, UP/DOWN cambiano il valore
                switch (editMode)
                {
                case EDIT_MODE_HOUR:
                    currentTime.hours = (currentTime.hours - 1 + 24) % 24;
                    break;
                case EDIT_MODE_MINUTE:
                    currentTime.minutes = (currentTime.minutes - 1 + 60) % 60;
                    break;
                case EDIT_MODE_DAY:
                    currentTime.day = (currentTime.day - 2 + 31) % 31 + 1;
                    break;
                case EDIT_MODE_MONTH:
                    currentTime.month = (currentTime.month - 2 + 12) % 12 + 1;
                    break;
                case EDIT_MODE_YEAR:
                    currentTime.year--;
                    if (currentTime.year < 2000)
                        currentTime.year = 2100;
                    break;
                }
                drawClockScreen(false, editMode);
            }
        }
    }

    // Pulsante SELECT
    if (currentSelectState == 0 && lastSelectState == 1) {
        __delay_cycles(10000); // Debounce

        if (currentState == MENU_QUADRANT)
        {
            // Vai al menu corrispondente in base alla selezione
            switch (quadrantSelection)
            {
            case 0:
                currentState = MENU_TEMPERATURE;
                drawEnhancedTemperatureScreen(false, getTemperature());
                updateWarmWhite();
                break;
            case 1:
                currentState = MENU_LIGHT;
                drawLightScreen(0);
                updateWarmWhite();
                break;
            case 2:
                currentState = MENU_LED;
                ledMenuSelection = 0;
                updateWarmWhite();
                drawLedScreen(false, ledMenuSelection);
                break;
            case 3:
                currentState = MENU_CLOCK;
                editMode = EDIT_MODE_HOUR; // Inizia con la modifica dell'ora
                drawClockScreen(0, editMode);
                updateWarmWhite();
                break;
            }
        } else if (currentState == TEMP_ALERT) {
            stopBuzzer();
            currentState = MENU_QUADRANT;
            Graphics_clearDisplay(&g_sContext);
            drawQuadrantMenuWithSelection(quadrantSelection);
        }
        else if (currentState == MENU_LED) {
            if (ledMenuSelection == ledMenuSize - 1) {
                currentState = MENU_QUADRANT;
                updateWarmWhite();
                Graphics_clearDisplay(&g_sContext);
                drawQuadrantMenuWithSelection(quadrantSelection);
            } else {
                updateLED(ledMenuSelection);
                updateWarmWhite();
                drawLedScreen(0, ledMenuSelection);
                __delay_cycles(50000);
            }
        }
        else if (currentState == MENU_CLOCK) {
            if (editMode == EDIT_MODE_NONE) {
                // Torna al menu principale
                currentState = MENU_QUADRANT;
                Graphics_clearDisplay(&g_sContext);
                drawQuadrantMenuWithSelection(quadrantSelection);
            } else {
                // Passa al prossimo campo o conferma
                if (editMode == EDIT_MODE_YEAR) {
                    // Ultimo campo, conferma e torna alla visualizzazione
                    setTime(currentTime.hours, currentTime.minutes, currentTime.seconds,
                           currentTime.day, currentTime.month, currentTime.year);
                    editMode = EDIT_MODE_NONE;
                } else {
                    editMode++;
                }
                drawClockScreen(false, editMode);
            }
        }
        else {
            currentState = MENU_QUADRANT;
            Graphics_clearDisplay(&g_sContext);
            drawQuadrantMenuWithSelection(quadrantSelection);
        }
    }

    lastUpState = currentUpState;
    lastDownState = currentDownState;
    lastSelectState = currentSelectState;
}

void toggleEarthquakeLed(bool stato){
    if (stato) {
        GPIO_setOutputHighOnPin(LED_TERREMOTO);  // Accende il LED
        GPIO_setOutputHighOnPin(LED_TEMP_ALTA);
    } else {
        GPIO_setOutputLowOnPin(LED_TERREMOTO);    // Spegne il LED
        GPIO_setOutputLowOnPin(LED_TEMP_ALTA);
    }
}
void toggleHighTempLed(bool stato){
    if (stato) {
        GPIO_setOutputHighOnPin(LED_TERREMOTO);  // Accende il LED
    } else {
        GPIO_setOutputLowOnPin(LED_TERREMOTO);    // Spegne il LED
    }
}
void toggleLowTempLed(bool stato){
    if (stato) {
        GPIO_setOutputHighOnPin(LED_TEMP_BASSA);  // Accende il LED
    } else {
        GPIO_setOutputLowOnPin(LED_TEMP_BASSA);    // Spegne il LED
    }
}


int main(void) {
    MAP_WDT_A_holdTimer(); // Disabilita il Watchdog

    _hwInit();
    currentState = MENU_SPLASH;
    showSplashScreen();  // Mostra immagine iniziale per 5s
    Graphics_clearDisplay(&g_sContext);
    currentState = MENU_QUADRANT;
    updateTimeFromRTC();
    drawQuadrantMenuWithSelection(0);  // Poi mostra il menu a 4 quadranti

    // Modifica nel main loop
    while (1) {
        lastState = currentState;
        PIR_detect();

        if(isMovementDetected()){
            toggleEarthquakeLed(1);
        }else{
            toggleEarthquakeLed(0);
        }

        // Gestione allarme temperatura
        float currentTemp = getTemperature();

        if (currentTemp > 40) {
            toggleHighTempLed(true);  // Accendi il LED se temp > 40
            if (!showedAlarm) {
                currentState = TEMP_ALERT;
                //_buzzerInit();
                showedAlarm = true;
                sameDataDisplay = false; // Forza l'aggiornamento della schermata
            }
        }else if (currentTemp < 15) {
            toggleLowTempLed(true);  // Accendi il LED se temp > 25
            if (!showedAlarm) {
                currentState = TEMP_ALERT;
                //_buzzerInit();
                showedAlarm = true;
                sameDataDisplay = false; // Forza l'aggiornamento della schermata
            }
        }
        else {
            toggleHighTempLed(false);  // Spegni il LED
            toggleLowTempLed(false);
            if (showedAlarm) {
                // Se stiamo uscendo dallo stato di allarme
                if (currentState == TEMP_ALERT) {
                    //stopBuzzer();
                    currentState = MENU_QUADRANT;
                    Graphics_clearDisplay(&g_sContext);
                    drawQuadrantMenuWithSelection(quadrantSelection);
                }
                showedAlarm = false;
            }
        }

        handleInput();

        // Aggiorna il flag sameDataDisplay
        sameDataDisplay = (lastState == currentState);

        updateWarmWhite();
        // Aggiorna l'RTC SOLO se non siamo in modalità modifica
        if (currentState != MENU_CLOCK || editMode == EDIT_MODE_NONE)
        {
            updateTimeFromRTC();
        }

        // Aggiorna gli schermi
        if(currentState == TEMP_ALERT){
            drawTemperatureScreen_Alert(sameDataDisplay);
        }
        if (currentState != MENU_QUADRANT) {
            switch(currentState) {
                case MENU_TEMPERATURE:
                    //drawTemperatureScreen(sameDataDisplay);
                    drawEnhancedTemperatureScreen(sameDataDisplay, getTemperature());
                    break;
                case MENU_LIGHT:
                    drawLightScreen(sameDataDisplay);
                    break;
                case MENU_LED:
                    // Nessun disegno specifico qui
                    break;
                case MENU_CLOCK:
                    drawClockScreen(sameDataDisplay, editMode);
                    break;
                default: break;
            }
            updateWarmWhite();
        }
        __delay_cycles(200000);
    }
}

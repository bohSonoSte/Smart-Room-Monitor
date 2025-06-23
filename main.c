#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "HAL_I2C.h"
#include "HAL_OPT3001.h"
#include "HAL_TMP006.h"
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>

// Definizioni pin
#define PIR_PIN         GPIO_PORT_P6, GPIO_PIN4
#define BUTTON_DOWN       GPIO_PORT_P3, GPIO_PIN5   // S2 (DOWN button)
#define BUTTON_UP     GPIO_PORT_P5, GPIO_PIN1   // S1 (UP button)
#define BUTTON_SELECT   GPIO_PORT_P4, GPIO_PIN1
#define PWM_PERIOD      1000

#define LED_PIN GPIO_PORT_P2, GPIO_PIN6  // LED rosso del BoosterPack

#define IDLE_TIMEOUT_SECONDS 30
#define TIMER_FREQUENCY 750000  // SMCLK / 16


// Stati del menu
typedef enum {
    MENU_MAIN,
    MENU_TEMPERATURE,
    MENU_LIGHT,
    MENU_MOTION,
    MENU_CLOCK,
    MENU_LED
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
MenuState currentState = MENU_MAIN;
MenuState lastState = MENU_MAIN;
uint8_t currentSelection = 0;

// Elementi del menu principale
MenuItem mainMenu[] = {
    {"Temperature", MENU_TEMPERATURE},
    {"Light Sensor", MENU_LIGHT},
    {"Motion Sensor", MENU_MOTION},
    {"Clock", MENU_CLOCK},
    {"Led", MENU_LED}
};
uint8_t mainMenuSize = sizeof(mainMenu) / sizeof(MenuItem);

void initStandbyTimerA(void) {
    // ACLK = 32768 Hz
    Timer_A_configureUpMode(TIMER_A1_BASE, &(Timer_A_UpModeConfig){
        TIMER_A_CLOCKSOURCE_ACLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_1,
        32768 * 30,  // 30 secondi
        TIMER_A_TAIE_INTERRUPT_DISABLE,
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
        TIMER_A_DO_CLEAR
    });

    // Abilita l'interrupt del TimerA1 nella NVIC
    Interrupt_enableInterrupt(INT_TA1_0);

    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
}
void TA1_0_IRQHandler(void) {
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Standby Mode",
                                AUTO_STRING_LENGTH, 64, 64, OPAQUE_TEXT);

    // Abilita interrupt pulsanti
    GPIO_clearInterruptFlag(BUTTON_UP);
    GPIO_clearInterruptFlag(BUTTON_DOWN);
    GPIO_clearInterruptFlag(BUTTON_SELECT);
    GPIO_enableInterrupt(BUTTON_UP);
    GPIO_enableInterrupt(BUTTON_DOWN);
    GPIO_enableInterrupt(BUTTON_SELECT);

    // Entra in LPM3
    PCM_gotoLPM3();
}


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

    // COnfigurazione timer per spegnimento automatico
    MAP_GPIO_setAsInputPinWithPullUpResistor(BUTTON_UP);
    GPIO_clearInterruptFlag(BUTTON_UP);
    GPIO_enableInterrupt(BUTTON_UP);

    MAP_GPIO_setAsInputPinWithPullUpResistor(BUTTON_DOWN);
    GPIO_clearInterruptFlag(BUTTON_DOWN);
    GPIO_enableInterrupt(BUTTON_DOWN);

    MAP_GPIO_setAsInputPinWithPullUpResistor(BUTTON_SELECT);
    GPIO_clearInterruptFlag(BUTTON_SELECT);
    GPIO_enableInterrupt(BUTTON_SELECT);

    // Altre inizializzazioni
    _graphicsInit();
    _pirInit();
    _lightSensorInit();
    _temperatureSensorInit();
    _initPWM();

    initStandbyTimerA();
}

void _graphicsInit(void) {
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
}

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

void _lightSensorInit(void)
{
    Init_I2C_GPIO();
    I2C_init();
    OPT3001_init();
    __delay_cycles(100000);
}

void _pirInit(){
    // Configura il PIR come input con pull-down interno (evita falsi trigger)
    GPIO_setAsInputPinWithPullDownResistor(PIR_PIN);

    // Configura il LED come output
    GPIO_setAsOutputPin(LED_PIN);
    GPIO_setOutputLowOnPin(LED_PIN);  // Spegne il LED all'inizio
    __delay_cycles(10);
}

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

// Funzioni per gestire i vari stati del menu
void drawMainMenu() {
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"SmartRoom Controller",
                              AUTO_STRING_LENGTH, 64, 10, OPAQUE_TEXT);

    uint8_t i = 0;
    for (i = 0; i < mainMenuSize; i++) {
        if (i == currentSelection) {
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        } else {
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        }

        Graphics_drawStringCentered(&g_sContext, (int8_t *)mainMenu[i].name,
                                  AUTO_STRING_LENGTH, 64, 30 + i * 20, OPAQUE_TEXT);
    }
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
}

void drawTemperatureScreen(bool sameDataDisplay) {
    char string[20];
    if(!sameDataDisplay){
        Graphics_clearDisplay(&g_sContext);
    }

    temp = TMP006_getTemp();
    temp = (temp - 32.0) * (5.0/9.0) - 6.3;
    sprintf(string, "Temp: %.2f C", temp);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)string,
                              AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Press SELECT to return",
                              AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}

void drawLightScreen(sameDataDisplay) {
    char string[20];
    if(!sameDataDisplay){
        Graphics_clearDisplay(&g_sContext);
    }

    lux = OPT3001_getLux();
    sprintf(string, "Light: %.2f lux", lux);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)string,
                              AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Press SELECT to return",
                              AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}

void drawMotionScreen(sameDataDisplay) {
    if(!sameDataDisplay){
        Graphics_clearDisplay(&g_sContext);
    }

    motionDetected = (GPIO_getInputPinValue(PIR_PIN) == 1);
    if (motionDetected) {
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"Motion: DETECTED",
                                  AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    } else {
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"Motion: NOT detected",
                                  AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    }

    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Press SELECT to return",
                              AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}

void drawClockScreen(sameDataDisplay) {
    // Implementa la visualizzazione dell'orologio come nel tuo esempio RTC
    if(!sameDataDisplay){
        Graphics_clearDisplay(&g_sContext);
    }
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Clock Screen",
                              AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Press SELECT to return",
                              AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}

// Gestione input utente
void handleInput() {
    static uint8_t lastUpState = 1;
    static uint8_t lastDownState = 1;
    static uint8_t lastSelectState = 1;

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
            drawMainMenu();
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
            drawMainMenu();
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
                case MENU_TEMPERATURE: drawTemperatureScreen(0); break;
                case MENU_LIGHT: drawLightScreen(0); break;
                case MENU_MOTION: drawMotionScreen(0); break;
                case MENU_CLOCK: drawClockScreen(0); break;
                default: break;
            }
        } else {
            currentState = MENU_MAIN;
            drawMainMenu();
        }
    }

    lastUpState = currentUpState;
    lastDownState = currentDownState;
    lastSelectState = currentSelectState;
}

// Risveglio del sistema
void PORT1_IRQHandler(void) {
    GPIO_clearInterruptFlag(BUTTON_SELECT);
    currentState = MENU_MAIN;
    drawMainMenu();
}

void PORT3_IRQHandler(void) {
    GPIO_clearInterruptFlag(BUTTON_DOWN);
    currentState = MENU_MAIN;
    drawMainMenu();
}

void PORT5_IRQHandler(void) {
    GPIO_clearInterruptFlag(BUTTON_UP);
    currentState = MENU_MAIN;
    drawMainMenu();
}

int main(void) {
    MAP_WDT_A_holdTimer(); // Disabilita il Watchdog

    Interrupt_enableInterrupt(INT_TA1_0);
    Interrupt_enableInterrupt(INT_PORT1);
    Interrupt_enableInterrupt(INT_PORT3);
    Interrupt_enableInterrupt(INT_PORT5);

    _hwInit();
    drawMainMenu();


    while (1) {
        lastState = currentState;
        handleInput();
        if(lastState == currentState){
            sameDataDisplay = true;
        }else{
            sameDataDisplay = false;
        }

        // Aggiorna solo gli schermi che mostrano dati in tempo reale
        if (currentState != MENU_MAIN) {
            switch(currentState) {
                case MENU_TEMPERATURE: drawTemperatureScreen(sameDataDisplay); break;
                case MENU_LIGHT: drawLightScreen(sameDataDisplay); break;
                case MENU_MOTION: drawMotionScreen(sameDataDisplay); break;
                // MENU_CLOCK può essere aggiornato tramite interrupt
                default: break;
            }
        }

        __delay_cycles(200000); // Piccolo ritardo per ridurre il consumo energetico
    }
}

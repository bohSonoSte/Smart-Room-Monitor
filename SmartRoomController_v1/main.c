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

// Pin definitions
#define BUTTON_DOWN       GPIO_PORT_P3, GPIO_PIN5   // S2 (DOWN button)
#define BUTTON_UP     GPIO_PORT_P5, GPIO_PIN1   // S1 (UP button)
#define BUTTON_SELECT   GPIO_PORT_P4, GPIO_PIN1
#define Btn GPIO_PORT_P1, GPIO_PIN1
#define PWM_PERIOD      1000


#define PIR_LED         GPIO_PORT_P1, GPIO_PIN0
#define LED_PIN GPIO_PORT_P2, GPIO_PIN6  // Boosterpack red LED
#define LED_TERREMOTO GPIO_PORT_P2, GPIO_PIN0
#define LED_TEMP_ALTA GPIO_PORT_P2, GPIO_PIN1
#define LED_TEMP_BASSA GPIO_PORT_P2, GPIO_PIN2

#define ALARM_LED_PIN GPIO_PORT_P1, GPIO_PIN0  // Boosterpack red LED

#define IDLE_TIMEOUT_SECONDS 30
#define TIMER_FREQUENCY 750000  // SMCLK / 16

#define EDIT_MODE_NONE 0
#define EDIT_MODE_HOUR 1
#define EDIT_MODE_MINUTE 2
#define EDIT_MODE_DAY 3
#define EDIT_MODE_MONTH 4
#define EDIT_MODE_YEAR 5

uint8_t editMode = EDIT_MODE_NONE;


// menu states
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

// Struct for menu elements
typedef struct {
    const char* name;
    MenuState targetState;
} MenuItem;

// Global variables
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


// Main menu elements
MenuItem mainMenu[] = {
    {"Temperature", MENU_TEMPERATURE},
    {"Light Sensor", MENU_LIGHT},
    {"Led", MENU_LED},
    {"Clock", MENU_CLOCK}
};
uint8_t mainMenuSize = sizeof(mainMenu) / sizeof(MenuItem);



// Hardware initialization
void _hwInit(void) {
    WDT_A_holdTimer();
    Interrupt_disableMaster();
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    // Clock configuration
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // Button configurations
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_UP);
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_DOWN);
    GPIO_setAsInputPinWithPullUpResistor(BUTTON_SELECT);
    GPIO_setAsInputPinWithPullUpResistor(Btn);

    // LED alert configuration
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);  // Sets P1.0 as output
    GPIO_setAsOutputPin(LED_TERREMOTO);
    GPIO_setAsOutputPin(LED_TEMP_ALTA);
    GPIO_setAsOutputPin(LED_TEMP_BASSA);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(LED_TERREMOTO);
    GPIO_setOutputLowOnPin(LED_TEMP_ALTA);
    GPIO_setOutputLowOnPin(LED_TEMP_BASSA);


    // Other initializations
    _graphicsInit();
    initPir();
    _lightSensorInit();
    _temperatureSensorInit();
    _initPWM();
    initRTC();
    Accelerometer_init();

}

// User input management
void handleInput() {
    static uint8_t lastUpState = 1;
    static uint8_t lastDownState = 1;
    static uint8_t lastSelectState = 1;
    static uint8_t ledMenuSelection = 0;
    const uint8_t ledMenuSize = 7;

    uint8_t currentUpState = GPIO_getInputPinValue(BUTTON_UP);
    uint8_t currentDownState = GPIO_getInputPinValue(BUTTON_DOWN);
    uint8_t currentSelectState = GPIO_getInputPinValue(BUTTON_SELECT);

    // UP Button
    if (currentUpState == 0 && lastUpState == 1) {
        __delay_cycles(10000); // Debounce

        if (currentState == MENU_QUADRANT)
        {
            if (quadrantSelection > 0)
                quadrantSelection--;
            else
                quadrantSelection = 3;  // from 0 to 3 (4 quadrants)
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
                // In normal mode, UP/DOWN change the field to modify
                if (editMode > EDIT_MODE_HOUR)
                    editMode--;
                else
                    editMode = EDIT_MODE_YEAR;
                drawClockScreen(false, editMode);
            }
            else
            {
                // In modify mode, UP/DOWN change the value
                switch (editMode)
                {
                case EDIT_MODE_HOUR:
                    currentTime.hours = (currentTime.hours + 1) % 24;
                    break;
                case EDIT_MODE_MINUTE:
                    currentTime.minutes = (currentTime.minutes + 1) % 60;
                    break;
                case EDIT_MODE_DAY:
                    currentTime.day = (currentTime.day % 31) + 1; // simplified without checks on the month
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

    // DOWN button
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
                // In normal mode, UP/DOWN change the field to modify
                if (editMode < EDIT_MODE_YEAR)
                    editMode++;
                else
                    editMode = EDIT_MODE_HOUR;
                drawClockScreen(false, editMode);
            }
            else
            {
                //  In modify mode, UP/DOWN change the value
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

    // SELECT button
    if (currentSelectState == 0 && lastSelectState == 1) {
        __delay_cycles(10000); // Debounce

        if (currentState == MENU_QUADRANT)
        {
            // Go to the right menu based on selection
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
                editMode = EDIT_MODE_HOUR; // Starts with the time modifying mode
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
                // Back to main menu
                currentState = MENU_QUADRANT;
                Graphics_clearDisplay(&g_sContext);
                drawQuadrantMenuWithSelection(quadrantSelection);
            } else {
                // Goes to next field or confirm
                if (editMode == EDIT_MODE_YEAR) {
                    // Last field, confirm and go back to visualization
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
        GPIO_setOutputHighOnPin(LED_TERREMOTO);  // Turns on the LED
        GPIO_setOutputHighOnPin(LED_TEMP_ALTA);
    } else {
        GPIO_setOutputLowOnPin(LED_TERREMOTO);    // Shuts down the LED
        GPIO_setOutputLowOnPin(LED_TEMP_ALTA);
    }
}
void toggleHighTempLed(bool stato){
    if (stato) {
        GPIO_setOutputHighOnPin(LED_TERREMOTO);  // Turns on the LED
    } else {
        GPIO_setOutputLowOnPin(LED_TERREMOTO);    //  Shuts down the LED
    }
}
void toggleLowTempLed(bool stato){
    if (stato) {
        GPIO_setOutputHighOnPin(LED_TEMP_BASSA);  // Turns on the LED
    } else {
        GPIO_setOutputLowOnPin(LED_TEMP_BASSA);    //  Shuts down the LED
    }
}


int main(void) {
    MAP_WDT_A_holdTimer(); // Disables the  Watchdog

    _hwInit();
    currentState = MENU_SPLASH;
    showSplashScreen();  // Show startup image for 5s
    Graphics_clearDisplay(&g_sContext);
    currentState = MENU_QUADRANT;
    updateTimeFromRTC();
    drawQuadrantMenuWithSelection(0);  // the show the 4 quadrants menu

    while (1) {
        lastState = currentState;
        PIR_detect();

        if(isMovementDetected()){
            toggleEarthquakeLed(1);
        }else{
            toggleEarthquakeLed(0);
        }

        // temperature alarm management
        float currentTemp = getTemperature();

        if (currentTemp > 40) {
            toggleHighTempLed(true);  // Turns on LED if temp > 40
            if (!showedAlarm) {
                currentState = TEMP_ALERT;
                //_buzzerInit();
                showedAlarm = true;
                sameDataDisplay = false; // Forces screen update
            }
        }else if (currentTemp < 15) {
            toggleLowTempLed(true);  // Turns on LED if temp < 25
            if (!showedAlarm) {
                currentState = TEMP_ALERT;
                //_buzzerInit();
                showedAlarm = true;
                sameDataDisplay = false; // Forces screen update
            }
        }
        else {
            toggleHighTempLed(false);  // Shuts down the LED
            toggleLowTempLed(false);
            if (showedAlarm) {
                // If we are exiting from alarm state
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

        // Updates the flag sameDataDisplay
        sameDataDisplay = (lastState == currentState);

        updateWarmWhite();
        // Updates the RTC ONLY if we're not in modifying mode
        if (currentState != MENU_CLOCK || editMode == EDIT_MODE_NONE)
        {
            updateTimeFromRTC();
        }

        // Updates the screens
        if(currentState == TEMP_ALERT){
            drawTemperatureScreen_Alert(sameDataDisplay);
        }
        if (currentState != MENU_QUADRANT) {
            switch(currentState) {
                case MENU_TEMPERATURE:
                    drawEnhancedTemperatureScreen(sameDataDisplay, getTemperature());
                    break;
                case MENU_LIGHT:
                    drawLightScreen(sameDataDisplay);
                    break;
                case MENU_LED:
                    // No drawing specified here
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

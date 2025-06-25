#include "display.h"
#include "menu.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "../LcdDriver/Crystalfontz128x128_ST7735.h"
#include "../LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>

#include "temperature.h"
#include "lightSensor.h"
#include "led.h"

#define PIR_PIN         GPIO_PORT_P6, GPIO_PIN4


// Variabili globali
Graphics_Context g_sContext;
float lux, temp;
bool motionDetected;
char string[20];


void _graphicsInit(void) {
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
}

// Funzioni per gestire i vari stati del menu
void drawMainMenu(uint8_t currentSelection) {
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

    temp=getTemperature();
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

    lux = getLux();
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

void drawLedScreen(bool sameDataDisplay, uint8_t currentSelection) {
    if(!sameDataDisplay) {
        Graphics_clearDisplay(&g_sContext);
    }

    // Menu title
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"LED Control",
                              AUTO_STRING_LENGTH, 64, 10, OPAQUE_TEXT);

    // Menu options
    const char* menuOptions[] = {
        "Toggle LED",
        "Color: Warm White",
        "Color: Cool White",
        "Color: Red",
        "Color: Green",
        "Color: Blue",
        "Back"
    };
    uint8_t numOptions = sizeof(menuOptions)/sizeof(menuOptions[0]);

    // Draw menu items
    uint8_t i = 0;
    for (i = 0; i < numOptions; i++) {
        if (i == currentSelection) {
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        } else {
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        }

        Graphics_drawStringCentered(&g_sContext, (int8_t *)menuOptions[i],
                                  AUTO_STRING_LENGTH, 64, 30 + i * 15, OPAQUE_TEXT);
    }

}

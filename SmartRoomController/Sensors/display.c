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
#include "rtc.h"

#define PIR_PIN         GPIO_PORT_P2, GPIO_PIN3


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
        if(mainMenu[i].name == "Temperature" && getTemperature() > 40){
            sprintf(string, "Temperature ALTA" );
            Graphics_drawStringCentered(&g_sContext, (int8_t *)string,
                                      AUTO_STRING_LENGTH, 64, 30 + i * 20, OPAQUE_TEXT);
        }


        Graphics_drawStringCentered(&g_sContext, (int8_t *)mainMenu[i].name,
                                  AUTO_STRING_LENGTH, 64, 30 + i * 20, OPAQUE_TEXT);
    }
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
}

void drawTemperatureScreen(bool sameDataDisplay) {
    char string[20];
    setTime(1,1,1,26,11,2002);
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
        Graphics_clearDisplay(&g_sContext);
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
        "Dimmerable light",
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

void drawClockScreen(bool sameDataDisplay) {
    char timeString[20];
    char dateString[20];

    if(!sameDataDisplay) {
        Graphics_clearDisplay(&g_sContext);
    }

    // Formatta l'orario
    sprintf(timeString, "%02d:%02d:%02d",
           currentTime.hours,
           currentTime.minutes,
           currentTime.seconds);

    // Formatta la data
    sprintf(dateString, "%02d/%02d/%04d",
           currentTime.day,
           currentTime.month,
           currentTime.year);

    // Disegna l'orario
    Graphics_setFont(&g_sContext, &g_sFontCmss20b);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)timeString,
                              AUTO_STRING_LENGTH, 64, 40, OPAQUE_TEXT);

    // Disegna la data
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)dateString,
                              AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);

    // Istruzioni
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Press SELECT to return",
                              AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}

void drawTemperatureScreen_Alert(bool sameDataDisplay) {
    char string[20];
    if(!sameDataDisplay){
        Graphics_clearDisplay(&g_sContext);
    }

    temp=getTemperature();
    sprintf(string, "TEMPERATURA ELEVATA !", temp);
        Graphics_drawStringCentered(&g_sContext, (int8_t *)string,
                                  AUTO_STRING_LENGTH, 64, 20, OPAQUE_TEXT);

    sprintf(string, "Temp: %.2f C", temp);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)string,
                              AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Press SELECT to return",
                              AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}


void drawEnhancedTemperatureScreen(bool sameDataDisplay, float temperature) {
    // Icona termometro semplificata (8x8)
    static const uint8_t thermometerIcon[] = {
        0x18, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x3C,
        0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x24, 0x18
    };

    if(!sameDataDisplay) {
        Graphics_clearDisplay(&g_sContext);

        // Cornice sottile
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_drawRect(&g_sContext, 2, 2, 124, 124);
    }

    // Disegna icona termometro piccola (8x8)
    Graphics_drawImage(&g_sContext, thermometerIcon, 8, 8);

    // Formatta e centra la temperatura
    char tempString[10];
    sprintf(tempString, "%.1f°C", temperature);

    // Usa un font medio (anziché grande)
    Graphics_setFont(&g_sContext, &g_sFontCmss16);

    // Calcola la larghezza del testo per centrarlo perfettamente
    int16_t stringWidth = Graphics_getStringWidth(&g_sContext, (int8_t *)tempString, -1);
    int16_t xPos = (128 - stringWidth) / 2;

    Graphics_drawString(&g_sContext, (int8_t *)tempString, -1, xPos, 12, OPAQUE_TEXT);

    // Barra di temperatura centrata
    int16_t tempLevel = (int16_t)((temperature / 50.0) * 80); // Scala ridotta a 80px
    if(tempLevel > 80) tempLevel = 80;
    if(tempLevel < 0) tempLevel = 0;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_fillRect(&g_sContext, (128 - 80)/2, 40, tempLevel, 12);

    // Scala semplificata
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    uint8_t i = 0;
    for(i = 0; i <= 4; i++) {
        Graphics_drawLine(&g_sContext, 24 + (i*20), 55, 24 + (i*20), 58);
    }

    // Istruzioni di ritorno su due righe centrate
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Premi SELECT",
                              AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"per tornare indietro",
                              AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}



//Metodi per disegnare
void Graphics_drawRect(Graphics_Context *context, int16_t x, int16_t y,
                      int16_t width, int16_t height) {
    Graphics_drawLine(context, x, y, x + width, y); // Top
    Graphics_drawLine(context, x, y + height, x + width, y + height); // Bottom
    Graphics_drawLine(context, x, y, x, y + height); // Left
    Graphics_drawLine(context, x + width, y, x + width, y + height); // Right
}

void Graphics_fillRect(Graphics_Context *context, int16_t x, int16_t y,
                      int16_t width, int16_t height) {
    int16_t i = 0;
    for(i = 0; i < height; i++) {
        Graphics_drawLine(context, x, y + i, x + width, y + i);
    }
}

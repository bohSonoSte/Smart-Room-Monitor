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

#include "images/termometro_8bit.h"
#include "images/logoSRM.h"
#include "images/lampadina.h"
#include "images/rgbimage.h"

#define EDIT_MODE_NONE 0
#define EDIT_MODE_HOUR 1
#define EDIT_MODE_MINUTE 2
#define EDIT_MODE_DAY 3
#define EDIT_MODE_MONTH 4
#define EDIT_MODE_YEAR 5


#define PIR_PIN         GPIO_PORT_P5, GPIO_PIN5


// Variabili globali
Graphics_Context g_sContext;
float luxValue, temp;
bool motionDetected;
char string[20];

char luxString[10];
int16_t stringWidth;
int16_t xPos;

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

void drawLightScreen(bool sameDataDisplay) {
    // Pulisci lo schermo solo se necessario
    if(!sameDataDisplay) {
        Graphics_clearDisplay(&g_sContext);
    }else{
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_drawString(&g_sContext, (int8_t *)luxString, -1, xPos, 12, OPAQUE_TEXT);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    }


    // Formatta e centra il valore lux
    luxValue = getLux();
    sprintf(luxString, "%.1f lux", luxValue);

    // Usa lo stesso font della temperatura
    Graphics_setFont(&g_sContext, &g_sFontCmss16);

    // Calcola la larghezza del testo per centrarlo
    int16_t stringWidth = Graphics_getStringWidth(&g_sContext, (int8_t *)luxString, -1);
    int16_t xPos = (128 - stringWidth) / 2;

    Graphics_drawString(&g_sContext, (int8_t *)luxString, -1, xPos, 12, OPAQUE_TEXT);

    // Barra di intensità luminosa centrata
    int16_t lightLevel = (int16_t)((luxValue / 1000.0) * 80); // Scala fino a 1000 lux
    if(lightLevel > 80) lightLevel = 80;
    if(lightLevel < 0) lightLevel = 0;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_YELLOW);
    Graphics_fillRect(&g_sContext, (128 - 80)/2, 40, lightLevel, 12);

    // Scala semplificata
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    uint8_t i = 0;
    for(i = 0; i <= 4; i++) {
        Graphics_drawLine(&g_sContext, 24 + (i*20), 55, 24 + (i*20), 58);
    }

    // Istruzioni di ritorno (stesso stile temperatura)
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Premi SELECT",
                              AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"per tornare indietro",
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

void drawTemperatureScreen_Alert(bool sameDataDisplay) {
    temp = getTemperature();
    if(!sameDataDisplay && temp >40) {
        Graphics_clearDisplay(&g_sContext);
        // Sfondo rosso per l'allarme
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
        Graphics_fillRect(&g_sContext, 0, 0, 128, 128);
    }else if(!sameDataDisplay && temp < 15){
        Graphics_clearDisplay(&g_sContext);
        // Sfondo blu per l'allarme
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_fillRect(&g_sContext, 0, 0, 128, 128);
    }


    // Testo bianco per contrasto
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);

    // 1. Scritta "HIGH TEMPERATURE" in alto
    Graphics_setFont(&g_sContext, &g_sFontCmss12);
    if(temp > 40)
        sprintf(string, "HIGH TEMPERATURE !");
    else if (temp < 15)
        sprintf(string, "LOW TEMPERATURE !");
    Graphics_drawStringCentered(&g_sContext, (int8_t *)string,
                                  AUTO_STRING_LENGTH, 64, 20, OPAQUE_TEXT);

    // 2. Valore temperatura grande al centro
    char tempString[20];
    sprintf(tempString, "%.1f °C", temp);
    Graphics_setFont(&g_sContext, &g_sFontCmss24b); // Font più grande
    Graphics_drawStringCentered(&g_sContext, (int8_t*) tempString,
    AUTO_STRING_LENGTH,
                                64, 54, OPAQUE_TEXT);

    // 3. Messaggio per uscire in basso (stile coerente)
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Press SELECT to return",
    AUTO_STRING_LENGTH,
                                64, 100, OPAQUE_TEXT);

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
    //Graphics_drawImage(&g_sContext, thermometerIcon, 8, 8);

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

void drawClockScreen(bool sameDataDisplay, uint8_t editMode) {
    char timeString[20];
    char dateString[20];
    //char editString[30];

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

    // Titolo
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Clock Settings",
                              AUTO_STRING_LENGTH, 64, 10, OPAQUE_TEXT);

    // Disegna l'orario con evidenziazione se in modalità modifica
    Graphics_setFont(&g_sContext, &g_sFontCmss20b);

    if(editMode == EDIT_MODE_HOUR) {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"Edit HOUR",
                                  AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    } else if(editMode == EDIT_MODE_MINUTE) {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"Edit MINUTE",
                                  AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    } else {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    }

    Graphics_drawStringCentered(&g_sContext, (int8_t *)timeString,
                              AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);

    // Disegna la data con evidenziazione se in modalità modifica
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);

    if(editMode == EDIT_MODE_DAY) {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"Edit DAY",
                                  AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
    } else if(editMode == EDIT_MODE_MONTH) {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"Edit MONTH",
                                  AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
    } else if(editMode == EDIT_MODE_YEAR) {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"Edit YEAR",
                                  AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
    } else {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    }

    Graphics_drawStringCentered(&g_sContext, (int8_t *)dateString,
                              AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);

    // Istruzioni
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    if(editMode == EDIT_MODE_NONE) {
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"UP/DOWN: Select field",
                                  AUTO_STRING_LENGTH, 64, 110, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"SELECT: Edit",
                                  AUTO_STRING_LENGTH, 64, 120, OPAQUE_TEXT);
    } else {
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"UP/DOWN: Change value",
                                  AUTO_STRING_LENGTH, 64, 110, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, (int8_t *)"SELECT: Confirm",
                                  AUTO_STRING_LENGTH, 64, 120, OPAQUE_TEXT);
    }
}

//nuova grafica
void showSplashScreen(void) {
    GrImageDraw(&g_sContext, &dropLogoPalette, 0, 0);
    GrFlush(&g_sContext);
    __delay_cycles(48000000 * 1); // 5s delay
}
void drawQuadrantMenuWithSelection(uint8_t selected) {

    uint8_t quadWidth = 64;
    uint8_t quadHeight = 64;

    // Quadrante 0: in alto a sinistra (Termometro)
    Graphics_drawImage(&g_sContext, &termometroImage, 0, 0);
    if (selected == 0) {
        Graphics_setForegroundColor(&g_sContext, ClrRed);
        Graphics_drawRectangle(&g_sContext, &(Graphics_Rectangle){0, 0, quadWidth-1, quadHeight-1});
    }

    // Quadrante 1: in alto a destra (Lampadina)
    Graphics_drawImage(&g_sContext, &dropLampPalette, quadWidth, 0);
    if (selected == 1) {
        Graphics_setForegroundColor(&g_sContext, ClrRed);
        Graphics_drawRectangle(&g_sContext, &(Graphics_Rectangle){quadWidth, 0, 2*quadWidth-1, quadHeight-1});
    }

    // Quadrante 2: in basso a sinistra (RGB)
    Graphics_drawImage(&g_sContext, &dropRGBPalette, 0, quadHeight);
    if (selected == 2) {
        Graphics_setForegroundColor(&g_sContext, ClrRed);
        Graphics_drawRectangle(&g_sContext, &(Graphics_Rectangle){0, quadHeight, quadWidth-1, 2*quadHeight-1});
    }

    // Quadrante 3: in basso a destra (Orologio)
    char timeString[20];
    char dateString[20];

    // Formatta l'orario
    sprintf(timeString, "%02d:%02d", currentTime.hours, currentTime.minutes);

    // Formatta la data
    sprintf(dateString, "%02d/%02d/%04d", currentTime.day, currentTime.month, currentTime.year);

    // Disegna l'orario (grande)
    Graphics_setFont(&g_sContext, &g_sFontCmss20b); // Font grande
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)timeString,
                              AUTO_STRING_LENGTH,
                              quadWidth + quadWidth/2,
                              quadHeight + 15,
                              OPAQUE_TEXT);

    // Disegna la data (piccola)
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8); // Font piccolo
    Graphics_drawStringCentered(&g_sContext, (int8_t *)dateString,
                              AUTO_STRING_LENGTH,
                              quadWidth + quadWidth/2,
                              quadHeight + 45,
                              OPAQUE_TEXT);

    if (selected == 3) {
        Graphics_setForegroundColor(&g_sContext, ClrRed);
        Graphics_drawRectangle(&g_sContext, &(Graphics_Rectangle){quadWidth, quadHeight, 2*quadWidth-1, 2*quadHeight-1});
    }else{
        Graphics_setForegroundColor(&g_sContext, ClrWhite);
        Graphics_drawRectangle(&g_sContext, &(Graphics_Rectangle){quadWidth, quadHeight, 2*quadWidth-1, 2*quadHeight-1});
    }

    // Ripristina colore di default
    Graphics_setForegroundColor(&g_sContext, ClrWhite);

    Graphics_flushBuffer(&g_sContext);
}

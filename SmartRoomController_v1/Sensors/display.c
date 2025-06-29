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


// Global Variables
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
            sprintf(string, "HIGH temperature" );
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
    // Clean the screen if necessary
    if(!sameDataDisplay) {
        Graphics_clearDisplay(&g_sContext);
    }else{
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_drawString(&g_sContext, (int8_t *)luxString, -1, xPos, 12, OPAQUE_TEXT);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    }


    // Format and center lux value
    luxValue = getLux();
    sprintf(luxString, "%.1f lux", luxValue);

    // Use the same font as temperature
    Graphics_setFont(&g_sContext, &g_sFontCmss16);

    // Calculate the width of the text to center it
    int16_t stringWidth = Graphics_getStringWidth(&g_sContext, (int8_t *)luxString, -1);
    int16_t xPos = (128 - stringWidth) / 2;

    Graphics_drawString(&g_sContext, (int8_t *)luxString, -1, xPos, 12, OPAQUE_TEXT);

    // Light intensity bar centered
    int16_t lightLevel = (int16_t)((luxValue / 1000.0) * 80); // Scala fino a 1000 lux
    if(lightLevel > 80) lightLevel = 80;
    if(lightLevel < 0) lightLevel = 0;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_YELLOW);
    Graphics_fillRect(&g_sContext, (128 - 80)/2, 40, lightLevel, 12);

    // Draw light intensity bar
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    uint8_t i = 0;
    for(i = 0; i <= 4; i++) {
        Graphics_drawLine(&g_sContext, 24 + (i*20), 55, 24 + (i*20), 58);
    }

    // Draw instructions to go back
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Press SELECT",
                              AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"to go back",
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
        // Red background for the alarm
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
        Graphics_fillRect(&g_sContext, 0, 0, 128, 128);
    }else if(!sameDataDisplay && temp < 15){
        Graphics_clearDisplay(&g_sContext);
        // Blue background for the alarm
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_fillRect(&g_sContext, 0, 0, 128, 128);
    }


    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);

    // 1. String "HIGH TEMPERATURE" at the top
    Graphics_setFont(&g_sContext, &g_sFontCmss12);
    if(temp > 40)
        sprintf(string, "HIGH TEMPERATURE !");
    else if (temp < 15)
        sprintf(string, "LOW TEMPERATURE !");
    Graphics_drawStringCentered(&g_sContext, (int8_t *)string,
                                  AUTO_STRING_LENGTH, 64, 20, OPAQUE_TEXT);

    // 2. Temperature value in the middle
    char tempString[20];
    sprintf(tempString, "%.1f °C", temp);
    Graphics_setFont(&g_sContext, &g_sFontCmss24b);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) tempString,
    AUTO_STRING_LENGTH,
                                64, 54, OPAQUE_TEXT);

    // 3. Message to go back at the bottom
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Press SELECT to return",
    AUTO_STRING_LENGTH,
                                64, 100, OPAQUE_TEXT);

}

//Draw the temperature page
void drawEnhancedTemperatureScreen(bool sameDataDisplay, float temperature) {
    // thermometer icon (8x8)
    static const uint8_t thermometerIcon[] = {
        0x18, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x3C,
        0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x24, 0x18
    };

    if(!sameDataDisplay) {
        Graphics_clearDisplay(&g_sContext);

        // Light frame
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_drawRect(&g_sContext, 2, 2, 124, 124);
    }


    // Format and center the temperature
    char tempString[10];
    sprintf(tempString, "%.1f°C", temperature);

    // Use a medium font (instead of big)
    Graphics_setFont(&g_sContext, &g_sFontCmss16);

    // Calculate the width of the text to center it
    int16_t stringWidth = Graphics_getStringWidth(&g_sContext, (int8_t *)tempString, -1);
    int16_t xPos = (128 - stringWidth) / 2;

    Graphics_drawString(&g_sContext, (int8_t *)tempString, -1, xPos, 12, OPAQUE_TEXT);

    // Temperature bar centered
    int16_t tempLevel = (int16_t)((temperature / 50.0) * 80); // Scale reduced to 80px
    if(tempLevel > 80) tempLevel = 80;
    if(tempLevel < 0) tempLevel = 0;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_fillRect(&g_sContext, (128 - 80)/2, 40, tempLevel, 12);

    // Drawing thermometer
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    uint8_t i = 0;
    for(i = 0; i <= 4; i++) {
        Graphics_drawLine(&g_sContext, 24 + (i*20), 55, 24 + (i*20), 58);
    }

    // drawing instructions of return to main menu
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Press SELECT",
                              AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"to go back",
                              AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}



//Drawing methods
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

    if(!sameDataDisplay) {
        Graphics_clearDisplay(&g_sContext);
    }

    // Format the time
    sprintf(timeString, "%02d:%02d:%02d",
           currentTime.hours,
           currentTime.minutes,
           currentTime.seconds);

    // Format the date
    sprintf(dateString, "%02d/%02d/%04d",
           currentTime.day,
           currentTime.month,
           currentTime.year);

    // Title
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)"Clock Settings",
                              AUTO_STRING_LENGTH, 64, 10, OPAQUE_TEXT);

    // Draw the time highlighted if in modifying mode
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

    // Draw the date highlighted if in modifying mode
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

    // Instructions
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

void showSplashScreen(void) {
    GrImageDraw(&g_sContext, &dropLogoPalette, 0, 0);
    GrFlush(&g_sContext);
    __delay_cycles(48000000 * 1); // 5s delay
}
void drawQuadrantMenuWithSelection(uint8_t selected) {

    uint8_t quadWidth = 64;
    uint8_t quadHeight = 64;

    // Quadrant 0: Top Left (Thermometer)
    Graphics_drawImage(&g_sContext, &termometroImage, 0, 0);
    if (selected == 0) {
        Graphics_setForegroundColor(&g_sContext, ClrRed);
        Graphics_drawRectangle(&g_sContext, &(Graphics_Rectangle){0, 0, quadWidth-1, quadHeight-1});
    }

    // Quadrant 1: Top Right (Light bulb)
    Graphics_drawImage(&g_sContext, &dropLampPalette, quadWidth, 0);
    if (selected == 1) {
        Graphics_setForegroundColor(&g_sContext, ClrRed);
        Graphics_drawRectangle(&g_sContext, &(Graphics_Rectangle){quadWidth, 0, 2*quadWidth-1, quadHeight-1});
    }

    // Quadrant 2: Bottom Left (RGB)
    Graphics_drawImage(&g_sContext, &dropRGBPalette, 0, quadHeight);
    if (selected == 2) {
        Graphics_setForegroundColor(&g_sContext, ClrRed);
        Graphics_drawRectangle(&g_sContext, &(Graphics_Rectangle){0, quadHeight, quadWidth-1, 2*quadHeight-1});
    }

    // Quadrant 3: Bottom Right (Clock)
    char timeString[20];
    char dateString[20];

    // Format the time
    sprintf(timeString, "%02d:%02d", currentTime.hours, currentTime.minutes);

    // Format the date
    sprintf(dateString, "%02d/%02d/%04d", currentTime.day, currentTime.month, currentTime.year);

    // draw the time (big)
    Graphics_setFont(&g_sContext, &g_sFontCmss20b);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)timeString,
                              AUTO_STRING_LENGTH,
                              quadWidth + quadWidth/2,
                              quadHeight + 15,
                              OPAQUE_TEXT);

    // Draw the date (small)
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

    // Reset color to default
    Graphics_setForegroundColor(&g_sContext, ClrWhite);

    Graphics_flushBuffer(&g_sContext);
}

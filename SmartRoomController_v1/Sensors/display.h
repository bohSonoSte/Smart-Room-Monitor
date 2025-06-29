#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define TEMP_ICON_WIDTH 24
#define TEMP_ICON_HEIGHT 24

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>

void _graphicsInit(void);

//Metodi per disegnare menù
void drawMainMenu(uint8_t currentSelection);
void drawTemperatureScreen(bool sameDataDisplay);
void drawLightScreen(bool sameDataDisplay);
void drawMotionScreen(bool sameDataDisplay);
void drawLedScreen(bool sameDataDisplay, uint8_t currentSelection);
void drawClockScreen(bool sameDataDisplay, uint8_t editMode);
void drawTemperatureScreen_Alert(bool sameDataDisplay);
void drawEnhancedTemperatureScreen(bool sameDataDisplay, float temperature);
void Graphics_drawRect(Graphics_Context *context, int16_t x, int16_t y, int16_t width, int16_t height);
void Graphics_fillRect(Graphics_Context *context, int16_t x, int16_t y, int16_t width, int16_t height);
void showSplashScreen(void);
void drawQuadrantMenuWithSelection(uint8_t selected);

#endif

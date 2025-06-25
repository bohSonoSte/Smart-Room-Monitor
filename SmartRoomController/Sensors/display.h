#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>

void _graphicsInit(void);

//Metodi per disegnare menù
void drawMainMenu(uint8_t currentSelection);
void drawTemperatureScreen(bool sameDataDisplay);
void drawLightScreen(sameDataDisplay);
void drawMotionScreen(sameDataDisplay);
void drawLedScreen(bool sameDataDisplay, uint8_t currentSelection);

#endif

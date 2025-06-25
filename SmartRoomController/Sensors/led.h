#ifndef __LED_H__
#define __LED_H__

typedef enum {
    LED_OFF,
    LED_WARM_WHITE,
    LED_COOL_WHITE,
    LED_RED,
    LED_GREEN,
    LED_BLUE
} LEDColor;

void _initPWM(void);
void setColor(LEDColor color);
void updateLED(uint8_t option);


#endif

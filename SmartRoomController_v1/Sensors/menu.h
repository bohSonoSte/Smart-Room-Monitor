#ifndef MENU_H
#define MENU_H

#include <stdint.h>

typedef enum {
    MENU_MAIN,
    MENU_TEMPERATURE,
    MENU_LIGHT,
    MENU_MOTION,
    MENU_CLOCK,
    MENU_LED
} MenuState;

typedef struct {
    const char* name;
    MenuState targetState;
} MenuItem;

extern MenuItem mainMenu[];
extern uint8_t mainMenuSize;

#endif

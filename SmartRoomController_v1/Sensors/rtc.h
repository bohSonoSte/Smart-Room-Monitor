#ifndef __RTC_H__
#define __RTC_H__


#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>


typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} DateTime;

extern DateTime currentTime;  // Dichiarazione come variabile esterna

void initRTC(void);
void updateTimeFromRTC(void);
void setTime(uint8_t h, uint8_t m, uint8_t s, uint8_t day, uint8_t month, uint16_t year);


#endif

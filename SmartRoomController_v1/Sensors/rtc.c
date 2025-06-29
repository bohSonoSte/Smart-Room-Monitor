
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>


#include "rtc.h"

DateTime currentTime = {0, 0, 0, 1, 1, 2023}; // Definizione della variabile globale

void initRTC(void) {
    // Configura il RTC con l'oscillatore a 32.768 kHz
    RTC_C_initCalendar(&(RTC_C_Calendar){
        .seconds = 0,
        .minutes = 25,
        .hours = 16,
        .dayOfWeek = 0,
        .dayOfmonth = 26,
        .month = 6,
        .year = 2025
    }, RTC_C_FORMAT_BINARY);

    // Abilita l'oscillatore di bassa frequenza
    CS_startLFXT(CS_LFXT_DRIVE3);

    // Avvia il RTC
    RTC_C_startClock();
}

void updateTimeFromRTC(void) {
    RTC_C_Calendar calendarTime = RTC_C_getCalendarTime();
    currentTime.seconds = calendarTime.seconds;
    currentTime.minutes = calendarTime.minutes;
    currentTime.hours = calendarTime.hours;
    currentTime.day = calendarTime.dayOfmonth;
    currentTime.month = calendarTime.month;
    currentTime.year = calendarTime.year;
}

void setTime(uint8_t h, uint8_t m, uint8_t s, uint8_t day, uint8_t month, uint16_t year) {
    RTC_C_initCalendar(&(RTC_C_Calendar){
        .seconds = s,
        .minutes = m,
        .hours = h,
        .dayOfWeek = 0,
        .dayOfmonth = day,
        .month = month,
        .year = year
    }, RTC_C_FORMAT_BINARY);
    // Avvia il RTC
    RTC_C_startClock();
}


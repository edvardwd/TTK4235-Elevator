#ifndef TIMER_H
#define TIMER_H
#include <time.h>

typedef struct{
    time_t startTime;
    time_t stopTime;
} Timer;



void initTimer(Timer* timer);
void startTimer(Timer* timer);
void stopTimer(Timer* timer);
int getTimePassed(Timer* timer);
#endif
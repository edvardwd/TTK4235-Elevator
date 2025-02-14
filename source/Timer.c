#include "Timer.h"

void initTimer(Timer* timer){
    timer->startTime = time(0); //set the current time
}


void startTimer(Timer* timer){
    timer->startTime = time(0);
}

void stopTimer(Timer* timer){
    timer->stopTime = time(0);
}

int getTimePassed(Timer* timer){
    stopTimer(timer);
    return timer->stopTime - timer->startTime;
}

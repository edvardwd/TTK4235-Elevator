#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"
#include "Timer.h"
#include "Elevator.h"


int main() {
    Elevator elevator;
    initElevator(&elevator);

    elevatorMainLoop(&elevator);
    destroyElevator(&elevator);
    printf("Program finished.\n");

    return 0;
}


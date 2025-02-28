#include "ElevatorStateMachine.h"
#include "stdlib.h"
#include "stdio.h"

typedef struct{
    ElevatorStateMachine stateMachine;
    
    int* fixedFloors[N_FLOORS];
    int* queue[N_FLOORS];

    int orders[N_FLOORS][N_BUTTONS];
} Elevator;


void initElevator(Elevator* elevator);
void checkForOrders(Elevator* elevator);
void orderQueue(Elevator* elevator);
void updateQueue(Elevator* elevator);
void updateLights(Elevator* elevator);
void printOrders(Elevator* elevator);
void printQueue(Elevator* elevator);
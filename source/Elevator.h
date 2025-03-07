#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "driver/elevio.h"
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
void clearOrders(Elevator* elevator, int all);
void sortQueue(Elevator* elevator);
void updateQueue(Elevator* elevator);

int queueIsEmpty(Elevator* elevator);
int compareUp(int a, int b, int currentFloor, int moving);
int compareDown(int a, int b, int currentFloor, int moving);


void updateLights(Elevator* elevator);
void stopButtonLight(Elevator* elevator);
void doorLight(Elevator* elevator);
void buttonLights (Elevator* elevator);


void printOrders(Elevator* elevator);
void printQueue(Elevator* elevator);

void destroyElevator(Elevator* elevator); 
void updateState(Elevator* elevator);


void elevatorMainLoop(Elevator* elevator);



#endif
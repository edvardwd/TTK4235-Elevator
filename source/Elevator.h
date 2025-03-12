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
void destroyElevator(Elevator* elevator); 

void clearOrders(Elevator* elevator, int all);
void updateQueue(Elevator* elevator);

int hallOrderWrongDir(int floor, MotorDirection dir, Elevator* elevator);
int compareUp(int a, int b, Elevator* elevator);
int compareDown(int a, int b, Elevator* elevator);
void sortQueue(Elevator* elevator);

void stopButtonLight(Elevator* elevator);
void doorLight(Elevator* elevator);
void buttonLights (Elevator* elevator);
void updateLights(Elevator* elevator);

void printOrders(Elevator* elevator);
void printQueue(Elevator* elevator);


void elevatorMainLoop(Elevator* elevator);

#endif
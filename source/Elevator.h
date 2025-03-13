#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "driver/elevio.h"
#include "ElevatorStateMachine.h"
#include "stdlib.h"
#include "stdio.h"

typedef struct{
    ElevatorStateMachine stateMachine;
    
    int* fixedFloors[N_FLOORS]; //array holding the pointers of the different floors
    int* queue[N_FLOORS]; //array holding pointers to the floors values (-1 if no order and floor number if order)

    int orders[N_FLOORS][N_BUTTONS]; //2d array to monitor placed orders

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

void elevatorMainLoop(Elevator* elevator);

void printOrders(Elevator* elevator);
void printQueue(Elevator* elevator);


#endif
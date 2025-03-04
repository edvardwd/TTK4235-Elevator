#include "ElevatorStateMachine.h"


void initElevatorStateMachine(ElevatorStateMachine* stateMachine){
    stateMachine->state = IDLE;
    stateMachine->dir = DIRN_STOP;
    stateMachine->lastFloor = -1;
    stateMachine->doorOpen = 0;
}


void updateLastFloor(ElevatorStateMachine* stateMachine){
    int floor = elevio_floorSensor();
    stateMachine->lastFloor = (floor == -1) ? stateMachine->lastFloor : floor; //update floor when a new floor is reached
}

void setState(ElevatorStateMachine* stateMachine, State state){
    stateMachine->state = state;
}


void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir){
    stateMachine->dir = dir;
    elevio_motorDirection(dir);

    if (dir == DIRN_STOP) setState(stateMachine, IDLE);
    else setState(stateMachine, MOVING);
}

void setLastFloor(ElevatorStateMachine* stateMachine, int floor){
    stateMachine->lastFloor = floor;
}

void setDoorOpen(ElevatorStateMachine* stateMachine, int open){
    stateMachine->doorOpen = open;
}

State getState(ElevatorStateMachine* stateMachine){
    return stateMachine->state;
}
MotorDirection getDir(ElevatorStateMachine* stateMachine){
    return stateMachine->dir;
}
int getLastFloor(ElevatorStateMachine* stateMachine){
    return stateMachine->lastFloor;
}
int getDoorOpen(ElevatorStateMachine* stateMachine){
    return stateMachine->doorOpen;
}
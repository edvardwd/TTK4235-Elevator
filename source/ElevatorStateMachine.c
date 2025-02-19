#include "ElevatorStateMachine.h"



void setState(ElevatorStateMachine* stateMachine, State state){
    stateMachine->state = state;
}


void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir){
    stateMachine->dir = dir;
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
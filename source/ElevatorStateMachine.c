#include "ElevatorStateMachine.h"


void initElevatorStateMachine(ElevatorStateMachine* stateMachine){
    stateMachine->state = IDLE;
    stateMachine->dir = DIRN_STOP;
    stateMachine->lastFloor = -1;
    stateMachine->doorOpen = 0;
    initTimer(&stateMachine->timer);
}


void updateLastFloor(ElevatorStateMachine* stateMachine){
    int floor = elevio_floorSensor();
    stateMachine->lastFloor = (floor == -1) ? stateMachine->lastFloor : floor; //update floor when a new floor is reached
}


void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir){
    stateMachine->dir = dir;
    elevio_motorDirection(dir);
}


char* getStateAsStr(State state){
    switch (state)
    {
    case MOVING:
        return "MOVING";
    case IDLE:
        return "IDLE";
    case EMERGENCY_STOP:
        return "EMERGENCY_STOP";
    case DOOR_OPEN:
        return "DOOR_OPEN";
    default:
        return "";
    }
}
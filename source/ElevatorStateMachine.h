#ifndef ELEVATORSTATEMACHINE_H
#define ELEVATORSTATEMACHINE_H
#include "driver/elevio.h"
#include "Timer.h"
#include "stdio.h"

//State enums
typedef enum{
    MOVING,
    IDLE,
    EMERGENCY_STOP,
    DOOR_OPEN
} State;


//State machine class
typedef struct{
    State state;
    State lastState;
    MotorDirection dir;
    MotorDirection lastDir;

    int lastFloor;
    Timer timer; //timer object to monitor how long the door has been open

    int shouldClearAll; //flag to signalize if the Elevator struct should clear the whole queue
    
} ElevatorStateMachine;


void initElevatorStateMachine(ElevatorStateMachine* stateMachine);

void updateState(ElevatorStateMachine* stateMachine, int nextFloor);
void updateLastFloor(ElevatorStateMachine* stateMachine);
void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir);

char* stateToStr(State state);

#endif
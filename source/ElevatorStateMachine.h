#ifndef ELEVATORSTATEMACHINE_H
#define ELEVATORSTATEMACHINE_H
#include "driver/elevio.h"
#include "Timer.h"
#include "stdio.h"

//Declaration of enums
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
    Timer timer;

    int shouldClearAll;
    
} ElevatorStateMachine;


void initElevatorStateMachine(ElevatorStateMachine* stateMachine);
void updateLastFloor(ElevatorStateMachine* stateMachine);

void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir);

void updateStateMachine(ElevatorStateMachine* stateMachine, int nextFloor);
char* stateToStr(State state);
#endif
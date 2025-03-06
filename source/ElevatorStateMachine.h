#ifndef ELEVATORSTATEMACHINE_H
#define ELEVATORSTATEMACHINE_H
#include "driver/elevio.h"
#include "Timer.h"

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
    MotorDirection dir;


    int lastFloor;  
    int doorOpen;
    Timer timer;
    
} ElevatorStateMachine;


void initElevatorStateMachine(ElevatorStateMachine* stateMachine);
void updateLastFloor(ElevatorStateMachine* stateMachine);

void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir);

char* getStateAsStr(State state);
#endif
#include "driver/elevio.h"

//Declaration of enums
typedef enum{
    MOVING,
    IDLE,
} State;


//State machine class
typedef struct{
    State state;
    MotorDirection dir;


    int lastFloor;
    int doorOpen;
    
} ElevatorStateMachine;

//"Member functions"
void setState(ElevatorStateMachine* stateMachine, State state);
void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir);

void setLastFloor(ElevatorStateMachine* stateMachine, int floor);
void setDoorOpen(ElevatorStateMachine* stateMachine, int open);

State getState(ElevatorStateMachine* stateMachine);
MotorDirection getDir(ElevatorStateMachine* stateMachine);
int getLastFloor(ElevatorStateMachine* stateMachine);
int getDoorOpen(ElevatorStateMachine* stateMachine);
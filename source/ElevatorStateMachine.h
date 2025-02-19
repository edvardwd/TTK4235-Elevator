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
    
} ElevetorStateMachine;

//"Member functions"
void setLastFloor(ElevetorStateMachine* stateMachine, int floor);
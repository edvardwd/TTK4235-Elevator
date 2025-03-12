#include "ElevatorStateMachine.h"


void initElevatorStateMachine(ElevatorStateMachine* stateMachine){
    stateMachine->state = IDLE;
    stateMachine->lastState = IDLE;
    stateMachine->dir = DIRN_STOP;
    stateMachine->lastDir = DIRN_STOP;
    stateMachine->lastFloor = -1;
    stateMachine->shouldClearAll = 0;
    initTimer(&stateMachine->timer);
}


void updateLastFloor(ElevatorStateMachine* stateMachine){
    int floor = elevio_floorSensor();
    stateMachine->lastFloor = (floor == -1) ? stateMachine->lastFloor : floor; //update floor when a new floor is reached
}


void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir){
    stateMachine->lastDir = (stateMachine->dir == DIRN_STOP) ? stateMachine->lastDir : stateMachine->dir; //avoid lastDir to be set to DIRN_STOP
    stateMachine->dir = dir;
    elevio_motorDirection(dir);
}


char* stateToStr(State state){
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
        return "ERROR";
    }
}


void updateStateMachine(ElevatorStateMachine* stateMachine, int nextFloor){
    char* stateStr = stateToStr(stateMachine->state);
    printf("Current state is %s\n", stateStr);
    State newState = stateMachine->state;
    switch (stateMachine->state)
    {

    case IDLE: {

        float currentFloor = stateMachine->lastFloor;
        if (elevio_floorSensor() == -1){ //IDLE between floors
            currentFloor = currentFloor + (0.5 * stateMachine->lastDir); 
        }
        if (elevio_stopButton()){
            if (elevio_floorSensor() == -1){
                newState = EMERGENCY_STOP;
            } else{
                newState = DOOR_OPEN;
                startTimer(&stateMachine->timer);
            }

            break;
        }
        if (nextFloor == -1) break;

        if (nextFloor > currentFloor){
            setDir(stateMachine, DIRN_UP);
            newState = MOVING;

        } else if (nextFloor < currentFloor){
            setDir(stateMachine, DIRN_DOWN);
            newState = MOVING;
        } else{
            newState = DOOR_OPEN;
            startTimer(&stateMachine->timer);
        }
        
        break;
    }
    case MOVING:
        if (elevio_stopButton()){
            newState = EMERGENCY_STOP;
            //setDir(stateMachine, DIRN_STOP);
            break;
        }
        int floor = elevio_floorSensor();
        if (floor != -1 && floor == nextFloor){
            newState = DOOR_OPEN;
            startTimer(&stateMachine->timer);
        }
        break;

    case EMERGENCY_STOP:
        setDir(stateMachine, DIRN_STOP);
        if (!elevio_stopButton()){
            newState = IDLE;
        }
        break;

    case DOOR_OPEN:
        setDir(stateMachine, DIRN_STOP);
        if (elevio_stopButton()){
            startTimer(&stateMachine->timer); //reset timer to wait 3 more secs
            stateMachine->shouldClearAll = 1;
            break;
        }
        
        if (elevio_obstruction()){
            startTimer(&stateMachine->timer); //reset the timer to wait 3 more secs if obstruction sensor is high
        }

        if (getTimePassed(&stateMachine->timer) >= 3){
            newState = IDLE;
        }
        break;
    default:
        newState = stateMachine->state; 
        break;
    }
    stateMachine->lastState = stateMachine->state;
    stateMachine->state = newState;
    printf("Updated state. Current: %s, Last: %s, dir: %d\n", stateToStr(stateMachine->state), stateToStr(stateMachine->lastState), stateMachine->dir);
}
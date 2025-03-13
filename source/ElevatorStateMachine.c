#include "ElevatorStateMachine.h"


void initElevatorStateMachine(ElevatorStateMachine* stateMachine){
    // Initializes the ElevatorStateMachine structure by setting default values,
    // and initializing the timer
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
    stateMachine->lastFloor = (floor == -1) ? stateMachine->lastFloor : floor; // Update floor when a new floor is reached
}


void setDir(ElevatorStateMachine* stateMachine, MotorDirection dir){
    // Sets the elevator's movement direction while preserving the last direction.
    // Prevents lastdir from being set to STOP when transitioning states.
    stateMachine->lastDir = (stateMachine->dir == DIRN_STOP) ? stateMachine->lastDir : stateMachine->dir;
    stateMachine->dir = dir;
    elevio_motorDirection(dir);
}


void updateState(ElevatorStateMachine* stateMachine, int nextFloor){
    // Manages states transitions based on current state, queued orders and button presses

    char* stateStr = stateToStr(stateMachine->state);
    printf("Current state is %s\n", stateStr);
    State newState = stateMachine->state;

    switch (stateMachine->state)
    {

    case IDLE: {
        // Handles transitions from IDLE based on button presses or queued orders.
        float currentFloor = stateMachine->lastFloor;

        // Adjusts position if elevator is between floors.
        if (elevio_floorSensor() == -1){ 
            currentFloor = currentFloor + (0.5 * stateMachine->lastDir); 
        }
        // Emergency stop handling.
        if (elevio_stopButton()){
            if (elevio_floorSensor() == -1){
                newState = EMERGENCY_STOP;
            } else{
                newState = DOOR_OPEN;
                startTimer(&stateMachine->timer);
            }

            break;
        }
        // No active orders.
        if (nextFloor == -1) break;

        // Determines direction based on the next order in queue.
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
        // Stops if emergency stop is pressed.
        if (elevio_stopButton()){
            newState = EMERGENCY_STOP;
            break;
        }
        int floor = elevio_floorSensor();

        // Stops when reaching the target floor.
        if (floor != -1 && floor == nextFloor){
            newState = DOOR_OPEN;
            startTimer(&stateMachine->timer);
        }
        break;

    case EMERGENCY_STOP:
        // Stops elevator and waits until stop button is released.
        setDir(stateMachine, DIRN_STOP);
        if (!elevio_stopButton()){
            newState = IDLE;
        }
        break;

    case DOOR_OPEN:
        // Keeps doors open and handles obstructions or stop button presses.
        setDir(stateMachine, DIRN_STOP);
        if (elevio_stopButton()){
            startTimer(&stateMachine->timer); // Resets timer for additional 3 sec delay
            stateMachine->shouldClearAll = 1;
            break;
        }
        
        if (elevio_obstruction()){
            startTimer(&stateMachine->timer); // Resets timer when obstruction is detected.
        }
        // Closes doors after 3 seconds.
        if (getTimePassed(&stateMachine->timer) >= 3){ //3 seconds passed -> close the door
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


char* stateToStr(State state){
    //Helper function for debugging. 
    //Essentially a map with state as key and string as value.
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
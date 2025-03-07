#include "Elevator.h"


void initElevator(Elevator* elevator){ //"constructor"
    elevio_init();
    initElevatorStateMachine(&elevator->stateMachine);
    
    for (int i = 0; i < N_FLOORS; i++){
        int* num = (int*) malloc(sizeof(int));
        *num = -1;
        elevator->fixedFloors[i] = num;
        elevator->queue[i] = num;

        for (int j = 0; j < N_BUTTONS; j++){
            elevator->orders[i][j] = 0;
        }
    }

    //Get to a defined state
    setDir(&elevator->stateMachine, DIRN_DOWN);
    while (elevator->stateMachine.lastFloor == -1){

        updateLastFloor(&elevator->stateMachine);
    }
    setDir(&elevator->stateMachine, DIRN_STOP);
    printf("Startup finished. Reached floor: %d\n", elevator->stateMachine.lastFloor);
    
}



void checkForOrders(Elevator* elevator){
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            if (floor == 0 && button == BUTTON_HALL_DOWN) continue;
            if (floor == N_FLOORS - 1 && button == BUTTON_HALL_UP) continue;

            if (elevio_callButton(floor, button)){
                elevator->orders[floor][button] = 1;
                printOrders(elevator);
            }
        }
    }
}

void clearOrders(Elevator *elevator, int all){
    //Sets all orders in lastFloor (or all floors) to 0, and sets the correspondent floor to -1 in queue
    if (all){
        for (int floor = 0; floor < N_FLOORS; floor++){
            if (elevator->queue[floor] == NULL){
                printf("NULL Pointer!!\n");
                return;
            }
            *(elevator->fixedFloors[floor]) = -1;
            for (int button = 0; button < N_BUTTONS; button++){
                elevator->orders[floor][button] = 0;
            }
        }

    
    } else{ //should be used when a floor is reached
        *(elevator->fixedFloors[elevator->stateMachine.lastFloor]) = -1;
        //printf("Set floor %d to -1 in queue\n", elevator->stateMachine.lastFloor);
        for (int button = 0; button < N_BUTTONS; button++){
            elevator->orders[elevator->stateMachine.lastFloor][button] = 0;
            //printf("Set floor %d button %d to 0\n", elevator->stateMachine.lastFloor, button);
        }
    }
    //printf("New queue: ");
    //printQueue(elevator);
    //printOrders(elevator);
}

void printOrders(Elevator* elevator){
    for (int floor = N_FLOORS - 1; floor >= 0; floor--){
        printf("Floor %d: ", floor);
        for (int button = 0; button < N_BUTTONS; button++){
            printf("%d ", elevator->orders[floor][button]);
        }
        printf("\n");
    }
    printf("----------------\n");
}

void updateQueue(Elevator* elevator){
    /*If an order is placed in the cab it is directly added to the queue.
    If the order comes from a floor, it is placed in the queue if the elevator is IDLE,
    or if the order is in the elevators current path.
    */
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            if (!(elevator->orders[floor][button])) continue; //no order
            
            switch (button){

            case BUTTON_CAB:
                *(elevator->fixedFloors[floor]) = floor; //add to queue if order came from the cab
                break;
            case BUTTON_HALL_DOWN:
                if (elevator->stateMachine.state == IDLE || 
                    (elevator->stateMachine.dir == DIRN_DOWN && elevator->stateMachine.lastFloor > floor)){
                    *(elevator->fixedFloors[floor]) = floor;
                }
                break;
            case BUTTON_HALL_UP:
                if (elevator->stateMachine.state == IDLE || 
                    (elevator->stateMachine.dir == DIRN_UP && elevator->stateMachine.lastFloor < floor)){
                    *(elevator->fixedFloors[floor]) = floor;
                }
                break;
            default:
                break;
            }

        }
    }

    //printQueue(elevator);
    sortQueue(elevator);
}

int queueIsEmpty(Elevator *elevator){
    for (int i = 0; i < N_FLOORS; i++){
        if (*elevator->queue[i] != -1) return 0;
    }
    return 1;
}

int compareUp(int a, int b, int currentFloor, int moving){
    //1 if swap(a, b) is desirable for the queue order
    //printf("Comparing %d and %d. Should swap if %d == -1 or %d > %d or %d < %d\n", a, b, a, a, b, a, currentFloor);
    //printf("Received currentFloor = %d\n", currentFloor);
    if (a == -1) return 1; //send non-orders to the end of the queue
    if (b == -1) return 0;
    if (b < currentFloor) return 0; //b is in the wrong dir
    if ((a == currentFloor) && moving) return 1;
    if (a > b) return 1;
    if (a < currentFloor) return 1; //a is in the wrong dir
    return 0;
}

int compareDown(int a, int b, int currentFloor, int moving){
    //1 if swap(a, b) is desirable for the queue order
    //printf("Received currentFloor = %d\n", currentFloor);
    if (a == -1) return 1; //send non-orders to the end of the queue
    if (b == -1) return 0;
    if (b > currentFloor) return 0; //b is in the wrong dir
    if ((a == currentFloor) && moving) return 1;
    if (a < b) return 1;
    if (a > currentFloor) return 1; //a is in the wrong dir
    return 0;
}

void sortQueue(Elevator* elevator){
    //bubble sort algo
    int swapped;
    int shouldSwap;
    for (int i = 0; i < N_FLOORS; i++){
        swapped = 0;
        for (int j = 0; j < N_FLOORS - i - 1; j++){
            int moving = (elevator->stateMachine.state == MOVING);
            switch (elevator->stateMachine.dir){
                case DIRN_UP:
                    shouldSwap = compareUp(*elevator->queue[j], *elevator->queue[j+1], elevator->stateMachine.lastFloor, moving);
                    //printf("Return value: %d\n", shouldSwap);
                    break;
                case DIRN_DOWN:
                    shouldSwap = compareDown(*elevator->queue[j], *elevator->queue[j+1], elevator->stateMachine.lastFloor, moving);
                    break;
                default:
                    shouldSwap = (*elevator->queue[j] == -1) ? 1 : 0; //move no-orders to the end    
                    break;
            }
            if (shouldSwap){
                int* temp = elevator->queue[j];
                elevator->queue[j] = elevator->queue[j + 1];
                elevator->queue[j + 1] = temp;
                swapped = 1;
                //printf("Swapped nums\n");
            }
        }
        if (!swapped) break;
    }
    //printf("Queue ordered.\nCurrent floor = %d, direction = %d\n", elevator->stateMachine.lastFloor, elevator->stateMachine.dir);
    //printQueue(elevator);

}


void updateLights(Elevator* elevator){ //Updates all lights
        elevio_floorIndicator(elevator->stateMachine.lastFloor);
        stopButtonLight(elevator);
        doorLight(elevator);
        buttonLights(elevator);
}

void stopButtonLight(Elevator* elevator){
    elevio_stopLamp(elevio_stopButton());
}

void doorLight(Elevator* elevator){ //Turns on door light when door is open
    elevio_doorOpenLamp(elevator->stateMachine.state == DOOR_OPEN);
}

void buttonLights (Elevator* elevator){ 
    for (int floor = 0; floor < N_FLOORS; floor++) {
        for (int button = 0; button < N_BUTTONS; button++) {
            // Skip invalid buttons at top and bottom floors
            if ((floor == 0 && button == BUTTON_HALL_DOWN) || 
                (floor == N_FLOORS - 1 && button == BUTTON_HALL_UP)) {
                continue;
            }
            // Set button lights based on active orders
            elevio_buttonLamp(floor, button, elevator->orders[floor][button]);
        }
    }
}

void printQueue(Elevator* elevator){
    printf("Queue: \n");
    for (int i = 0; i < N_FLOORS; i++){
        printf("%d ", *(elevator->queue[i]));
    }
    printf("\n--------------\n");
}

void destroyElevator(Elevator *elevator){ //destructor
    //free all dynamically allocated memory
    for (int floor = 0; floor < N_FLOORS; floor++){
        free(elevator->fixedFloors[floor]);
        elevator->fixedFloors[floor] = NULL;
    }

    //Turn off all lights;
    elevio_stopLamp(0);
    elevio_doorOpenLamp(0);
    
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            elevio_buttonLamp(floor, button, 0);
        }
    }
    printf("Elevator successfully destroyed.\n");
}



void updateState(Elevator* elevator){
    ElevatorStateMachine* stateMachine = &elevator->stateMachine;
    int nextFloor = *elevator->queue[0];
    State newState = stateMachine->state;
    //printf("Current state is %s\n", stateStr);
    switch (stateMachine->state)
    {

    case IDLE:
        printOrders(elevator);
        printQueue(elevator);

        float currentFloor = stateMachine->lastFloor;
        if (elevio_floorSensor() == -1){ //IDLE between floors
            currentFloor = stateMachine->lastFloor + (0.5 * stateMachine->lastDir);
        }

        if (elevio_stopButton()){
            //stateMachine->state = EMERGENCY_STOP;
            if (elevio_floorSensor() == -1){
                newState = EMERGENCY_STOP;
            } else{
                newState = DOOR_OPEN;
                startTimer(&stateMachine->timer);
            }
            //setDir(&elevator->stateMachine, DIRN_STOP);
            clearOrders(elevator, 1); //clear all orders
            break;
        }
        if (nextFloor == -1) break;

        if (nextFloor > currentFloor){
            setDir(stateMachine, DIRN_UP);
            //stateMachine->state = MOVING;
            newState = MOVING;
        } else if (nextFloor < currentFloor){
            setDir(stateMachine, DIRN_DOWN);
            //stateMachine->state = MOVING;
            newState = MOVING;

        } else {
            newState = DOOR_OPEN;
            startTimer(&stateMachine->timer);
        }
        
        break;

    case MOVING:
        if (elevio_stopButton()){
            //stateMachine->state = EMERGENCY_STOP;
            newState = EMERGENCY_STOP;
            //setDir(&elevator->stateMachine, DIRN_STOP);
            clearOrders(elevator, 1); //clear all orders
            break;
        }
        int floor = elevio_floorSensor();
        if (floor != -1 && floor == nextFloor){
            //setDir(stateMachine, DIRN_STOP);
            //stateMachine->state = DOOR_OPEN;
            newState = DOOR_OPEN;
            startTimer(&stateMachine->timer);
        }
        break;

    case EMERGENCY_STOP:
        setDir(stateMachine, DIRN_STOP);
        if (!elevio_stopButton()){
            //stateMachine->state = IDLE;
            newState = IDLE;
        }
        break;

    case DOOR_OPEN:
        clearOrders(elevator, 0); //remove floor from queue and orders
        setDir(stateMachine, DIRN_STOP);
        if (elevio_stopButton()){
            startTimer(&stateMachine->timer); //reset timer to wait 3 more secs
            clearOrders(elevator, 1); //clear all orders
        }
        clearOrders(elevator, 0); //remove floor from queue and orders
        if (elevio_obstruction()){
            startTimer(&stateMachine->timer); //reset the timer to wait 3 more secs if obstruction sensor is high
        }

        if (getTimePassed(&stateMachine->timer) >= 3){
            //stateMachine->state = IDLE;
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


void elevatorMainLoop(Elevator* elevator){
    while (1){
        //if (elevio_stopButton() && elevio_obstruction()) break; //way to stop the simulator

        updateState(elevator);

        updateLastFloor(&elevator->stateMachine);
        checkForOrders(elevator);
        updateQueue(elevator);
        //updateStateMachine(&elevator->stateMachine, *elevator->queue[0]);
        printQueue(elevator);
        updateLights(elevator);

        switch (elevator->stateMachine.state)
        {
        case DOOR_OPEN:
            int clearAll = elevator->stateMachine.shouldClear;
            clearOrders(elevator, clearAll); //clear floor from orders
            //turn on light
            break;
    
        case EMERGENCY_STOP:
            clearOrders(elevator, 1); //clear all orders
            break;

        default:
            break;
        }

        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }

}
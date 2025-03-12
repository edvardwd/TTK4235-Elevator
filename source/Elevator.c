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



void updateQueue(Elevator* elevator){
    //Takes all orders, places them in the queue and sorts the queue
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            if (floor == 0 && button == BUTTON_HALL_DOWN) continue;
            if (floor == N_FLOORS - 1 && button == BUTTON_HALL_UP) continue;

            if (elevio_callButton(floor, button)){
                elevator->orders[floor][button] = 1;
                if (!(elevator->orders[floor][button])) continue; //no order
                *(elevator->fixedFloors[floor]) = floor;
            }
        }
    }
    sortQueue(elevator);
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

        for (int button = 0; button < N_BUTTONS; button++){
            elevator->orders[elevator->stateMachine.lastFloor][button] = 0;
        }
    }
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


int hallOrderWrongDir(int floor, MotorDirection dir, Elevator* elevator){
    //returns true if the order for the current compared order is in the wrong direction
    if (dir == DIRN_UP){
        return (elevator->orders[floor][BUTTON_HALL_DOWN] && !elevator->orders[floor][BUTTON_HALL_UP]
            && !elevator->orders[floor][BUTTON_CAB]);
    }
    return (elevator->orders[floor][BUTTON_HALL_UP] && !elevator->orders[floor][BUTTON_HALL_DOWN]
        && !elevator->orders[floor][BUTTON_CAB]);
}

int compareUp(int a, int b, Elevator* elevator){
    int currentFloor = elevator->stateMachine.lastFloor;
    int moving = (elevator->stateMachine.state == MOVING);
    //1 if swap(a, b) is desirable for the queue order

    if (a == -1) return 1; //send non-orders to the end of the queue
    if (b == -1) return 0;
    if (b < currentFloor) return 0; //b is in the wrong dir
    if ((a == currentFloor) && moving) return 1; //just departed from a
    if (!hallOrderWrongDir(a, DIRN_UP, elevator) && hallOrderWrongDir(b, DIRN_UP, elevator)) return 0;
    if (hallOrderWrongDir(a, DIRN_UP, elevator) && b > a) return 1;
    if (!hallOrderWrongDir(a, DIRN_UP, elevator) && (a > b)) return 1;
    if (!hallOrderWrongDir(b, DIRN_UP, elevator) && (a > b)) return 1;
    if (a < currentFloor) return 1; //a is in the wrong dir
    return 0;
}

int compareDown(int a, int b, Elevator* elevator){
    int currentFloor = elevator->stateMachine.lastFloor;
    int moving = (elevator->stateMachine.state == MOVING);
    //1 if swap(a, b) is desirable for the queue order

    if (a == -1) return 1; //send non-orders to the end of the queue
    if (b == -1) return 0;
    if (b > currentFloor) return 0; //b is in the wrong dir
    if ((a == currentFloor) && moving) return 1;
    if (!hallOrderWrongDir(a, DIRN_DOWN, elevator) && hallOrderWrongDir(b, DIRN_DOWN, elevator)) return 0;
    if (hallOrderWrongDir(a, DIRN_DOWN, elevator) && b < a) return 1;
    if (!hallOrderWrongDir(a, DIRN_DOWN, elevator) && (a < b)) return 1;
    if (!hallOrderWrongDir(b, DIRN_DOWN, elevator) && (a < b)) return 1;
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
            switch (elevator->stateMachine.dir){
                case DIRN_UP:
                    shouldSwap = compareUp(*elevator->queue[j], *elevator->queue[j+1], elevator);
                    //printf("Return value: %d\n", shouldSwap);
                    break;
                case DIRN_DOWN:
                    shouldSwap = compareDown(*elevator->queue[j], *elevator->queue[j+1], elevator);
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

    //Turn off lights
    elevio_stopLamp(0);
    elevio_doorOpenLamp(0);
    
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            elevio_buttonLamp(floor, button, 0);
        }
    }
    printf("Elevator successfully destroyed.\n");
}


void elevatorMainLoop(Elevator* elevator){
    while (1){
        if (elevio_stopButton() && elevio_obstruction()) break; //way to stop the simulator
        elevator->stateMachine.shouldClearAll = 0;
        updateStateMachine(&elevator->stateMachine, *elevator->queue[0]);
        updateLastFloor(&elevator->stateMachine);
        updateQueue(elevator);
        printQueue(elevator);
        updateLights(elevator);

        switch (elevator->stateMachine.state)
        {
        case DOOR_OPEN:{
            int clearAll = elevator->stateMachine.shouldClearAll;
            clearOrders(elevator, clearAll); //clear floor from orders
            break;
        }
        case EMERGENCY_STOP:
            clearOrders(elevator, 1); //clear all orders
            break;

        default:
            break;
        }

        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }

}
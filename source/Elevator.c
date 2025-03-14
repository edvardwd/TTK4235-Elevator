#include "Elevator.h"


void initElevator(Elevator* elevator){ // "constructor"
    // Initializes the elevator and state machine, allocates memory
    // and ensures the elevator reaches a defined state upon startup.
    elevio_init();
    initElevatorStateMachine(&elevator->stateMachine);
    
    for (int i = 0; i < N_FLOORS; i++){
        int* num = (int*) malloc(sizeof(int));
        *num = -1;
        elevator->fixedFloors[i] = num;
        elevator->queue[i] = num;

        for (int j = 0; j < N_BUTTONS; j++){
            elevator->orders[i][j] = 0; // Initialize order matrix with no active orders.
        }
    }

    //Starts from a defined floor.
    setDir(&elevator->stateMachine, DIRN_DOWN);
    while (elevator->stateMachine.lastFloor == -1){

        updateLastFloor(&elevator->stateMachine);
    }
    setDir(&elevator->stateMachine, DIRN_STOP);
    printf("Startup finished. Reached floor: %d\n", elevator->stateMachine.lastFloor);
    
}

void destroyElevator(Elevator *elevator){ 
    // Cleans up recources and resets all elevator-related states before termination.
    
    // Free all dynamically allocated memory.
    for (int floor = 0; floor < N_FLOORS; floor++){
        free(elevator->fixedFloors[floor]);
        elevator->fixedFloors[floor] = NULL;
    }

    // Turn off lights
    elevio_stopLamp(0);
    elevio_doorOpenLamp(0);
    
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            elevio_buttonLamp(floor, button, 0);
        }
    }
    printf("Elevator successfully destroyed.\n");
}

void clearOrders(Elevator *elevator, int all){
    // Clears all active orders. If 'all' is true, it clears all floors;
    // otherwise, it clears only the last floor the elevator stopped at.
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
    
    } else{ 
        // Clears only the floor where the elevator is currently stopped.
        *(elevator->fixedFloors[elevator->stateMachine.lastFloor]) = -1;

        for (int button = 0; button < N_BUTTONS; button++){
            elevator->orders[elevator->stateMachine.lastFloor][button] = 0;
        }
    }
}

void updateQueue(Elevator* elevator){
    // Scans for new orders, updates the queue and sorts it based on priority.
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            // Skip invalid buttons at top and bottom floors
            if (floor == 0 && button == BUTTON_HALL_DOWN) continue;
            if (floor == N_FLOORS - 1 && button == BUTTON_HALL_UP) continue;

            if (elevio_callButton(floor, button) && !elevio_stopButton()){
                elevator->orders[floor][button] = 1;
                *(elevator->fixedFloors[floor]) = floor;
            }
        }
    }
    sortQueue(elevator);
}

int hallOrderWrongDir(int floor, MotorDirection dir, Elevator* elevator){
    // Checks if the only active order on a floor is a hall call in the opposite direction of movement.
    if (dir == DIRN_UP){
        return (elevator->orders[floor][BUTTON_HALL_DOWN] && !elevator->orders[floor][BUTTON_HALL_UP]
            && !elevator->orders[floor][BUTTON_CAB]);
    }
    return (elevator->orders[floor][BUTTON_HALL_UP] && !elevator->orders[floor][BUTTON_HALL_DOWN]
        && !elevator->orders[floor][BUTTON_CAB]);
}

int compareUp(int a, int b, Elevator* elevator){
    // Determines if two queued orders should swap positions when moving up.

    int currentFloor = elevator->stateMachine.lastFloor;
    int moving = (elevator->stateMachine.state == MOVING);

    if (a == -1) return 1; // Send non-orders to the end of the queue.
    if (b == -1) return 0;
    if (b < currentFloor) return 0; // Ignore floors below when moving up
    if ((a == currentFloor) && moving) return 1; // Prioritize departing floor

    // Prevent prioritizing hall orders in the wrong direction
    if (!hallOrderWrongDir(a, DIRN_UP, elevator) && hallOrderWrongDir(b, DIRN_UP, elevator)) return 0;
    if (hallOrderWrongDir(a, DIRN_UP, elevator) && b > a) return 1;
    if (!hallOrderWrongDir(a, DIRN_UP, elevator) && (a > b)) return 1;
    if (!hallOrderWrongDir(b, DIRN_UP, elevator) && (a > b)) return 1;

    if (a < currentFloor) return 1; // a is in the wrong dir
    return 0;
}

int compareDown(int a, int b, Elevator* elevator){
    // Determines if two queued orders should swap positions when moving up.

    int currentFloor = elevator->stateMachine.lastFloor;
    int moving = (elevator->stateMachine.state == MOVING);

    if (a == -1) return 1; // Send non-orders to the end of the queue
    if (b == -1) return 0;
    if (b > currentFloor) return 0; // Ignore floors above when moving down
    if ((a == currentFloor) && moving) return 1; // Prioritize departing floor

    // Makes sure hall orders in the wrong direction are not being prioritized
    if (!hallOrderWrongDir(a, DIRN_DOWN, elevator) && hallOrderWrongDir(b, DIRN_DOWN, elevator)) return 0;
    if (hallOrderWrongDir(a, DIRN_DOWN, elevator) && b < a) return 1;
    if (!hallOrderWrongDir(a, DIRN_DOWN, elevator) && (a < b)) return 1;
    if (!hallOrderWrongDir(b, DIRN_DOWN, elevator) && (a < b)) return 1;

    if (a > currentFloor) return 1; // a is in the wrong dir
    return 0;
}

void sortQueue(Elevator* elevator){
    // Sorts the queue using a bubble sort algorithm.
    // The floors in the queue are swapped if 1 is returned from compareUp or compareDown
    int swapped;
    int shouldSwap;

    for (int i = 0; i < N_FLOORS; i++){
        swapped = 0;
        for (int j = 0; j < N_FLOORS - i - 1; j++){
            switch (elevator->stateMachine.dir){
                case DIRN_UP:
                    shouldSwap = compareUp(*elevator->queue[j], *elevator->queue[j+1], elevator);
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
            }
        }
        if (!swapped) break;
    }
}

void updateLights(Elevator* elevator){
    // Updates all lights according to the elevator's state and position
    elevio_floorIndicator(elevator->stateMachine.lastFloor);
    stopButtonLight(elevator);
    doorLight(elevator);
    buttonLights(elevator);
}

void stopButtonLight(Elevator* elevator){
    elevio_stopLamp(elevio_stopButton());
}

void doorLight(Elevator* elevator){
    elevio_doorOpenLamp(elevator->stateMachine.state == DOOR_OPEN);
}

void buttonLights (Elevator* elevator){ 
    // Sets button lights based on active orders
    for (int floor = 0; floor < N_FLOORS; floor++) {
        for (int button = 0; button < N_BUTTONS; button++) {
            // Skip invalid buttons at top and bottom floors
            if ((floor == 0 && button == BUTTON_HALL_DOWN) || 
                (floor == N_FLOORS - 1 && button == BUTTON_HALL_UP)) {
                continue;
            }
            elevio_buttonLamp(floor, button, elevator->orders[floor][button]);
        }
    }
}

void elevatorMainLoop(Elevator* elevator){
    // Main operational loop handling elevator logic and state updated
    while (1){
        elevator->stateMachine.shouldClearAll = 0;
        updateState(&elevator->stateMachine, *elevator->queue[0]);
        updateLastFloor(&elevator->stateMachine);
        updateQueue(elevator);
        printQueue(elevator);
        updateLights(elevator);

        switch (elevator->stateMachine.state)
        {
        case DOOR_OPEN:{
            int clearAll = elevator->stateMachine.shouldClearAll;
            clearOrders(elevator, clearAll); // Clear floor from orders
            break;
        }
        case EMERGENCY_STOP:
            clearOrders(elevator, 1); // Clear all orders
            break;

        default:
            break;
        }
        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }

}

void printOrders(Elevator* elevator){
    // Helper function for debugging. Prints the queue.
    for (int floor = N_FLOORS - 1; floor >= 0; floor--){
        printf("Floor %d: ", floor);
        for (int button = 0; button < N_BUTTONS; button++){
            printf("%d ", elevator->orders[floor][button]);
        }
        printf("\n");
    }
    printf("----------------\n");
}

void printQueue(Elevator* elevator){
    // Helper function for debugging. Prints the queue.
    printf("Queue: \n");
    for (int i = 0; i < N_FLOORS; i++){
        printf("%d ", *(elevator->queue[i]));
    }
    printf("\n--------------\n");
}
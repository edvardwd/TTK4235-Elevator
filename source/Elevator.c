#include "Elevator.h"


void initElevator(Elevator* elevator){ //"constructor"
    initElevatorStateMachine(&elevator->stateMachine);
    
    for (int i = 0; i < N_FLOORS; i++){
        int* num = (int*) malloc(sizeof(int));
        elevator->fixedFloors[i] = num;
        elevator->queue[i] = num;

        for (int j = 0; j < N_BUTTONS; j++){
            elevator->orders[i][j] = 0;
        }
    }
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
    //The function adds an order to the queue if its placed in the cab, or is in the elevators current path with same direction
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            if (!(elevator->orders[floor][button])) continue; //no order
            
            switch (button){

            case BUTTON_CAB:
                *(elevator->fixedFloors[floor]) = floor; //add to queue if order came from the cab
                break;
            case BUTTON_HALL_DOWN:
                if (elevator->stateMachine.dir == DIRN_DOWN && elevator->stateMachine.lastFloor > floor){
                    *(elevator->fixedFloors[floor]) = floor;
                }
                break;
            case BUTTON_HALL_UP:
                if (elevator->stateMachine.dir == DIRN_UP && elevator->stateMachine.lastFloor < floor){
                    *(elevator->fixedFloors[floor]) = floor;
                }
                break;
            default:
                break;
            }

        }
    }
    //printQueue(elevator);
}

void orderQueue(Elevator* elevator);

void updateLights(Elevator* elevator){ //Turns on correct floor lamp
    if (elevio_floorSensor() != -1){
        elevio_floorIndicator(elevator->stateMachine.lastFloor);
    } else {
        elevio_floorIndicator(elevator->stateMachine.lastFloor);
    }
}

void stopButton(Elevator* elevator){
    if (elevio_stopButton()) {
        elevio_stopLamp(1);
        // Clear all orders when stop is pressed
        for (int i = 0; i < N_FLOORS; i++) {
            for (int j = 0; j < N_BUTTONS; j++) {
                elevator->orders[i][j] = 0;
            }
        }
    } else {
        elevio_stopLamp(0);
    }
}

void doorLight(Elevator* elevator){ //Turns on door light when door is open
    elevio_doorOpenLamp(elevator->stateMachine.doorOpen);
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
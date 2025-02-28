#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"
#include "Timer.h"
#include "Elevator.h"


int main(){
    elevio_init();
    
    printf("=== Example Program ===\n");
    printf("Press the stop button on the elevator panel to exit\n");

    elevio_motorDirection(DIRN_UP);
    Timer timer;
    initTimer(&timer);

    Elevator elevator;
    initElevator(&elevator);

    printOrders(&elevator);
    while(1){
        updateLastFloor(&elevator.stateMachine);
        checkForOrders(&elevator);
        updateQueue(&elevator);
        int floor = elevator.stateMachine.lastFloor;
        //int floor = elevio_floorSensor();

        if(floor == 0){
            elevio_motorDirection(DIRN_UP);
            elevator.stateMachine.dir = DIRN_UP;
        }

        if(floor == N_FLOORS-1){
            elevio_motorDirection(DIRN_DOWN);
            elevator.stateMachine.dir = DIRN_DOWN;
        }


        if(elevio_stopButton()){
            elevio_motorDirection(DIRN_STOP);
            elevator.stateMachine.dir = DIRN_STOP;
            break;
        }
        
        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }


    return 0;
}












/*
int main() {
    elevio_init();

    elevio_motorDirection(DIRN_DOWN);

    Timer timer;
    initTimer(&timer);

    int DESIRED_FLOOR = 3;
    //printf("What is your desired floor? ");
    //scanf(DESIRED_FLOOR);
    //assert(DESIRED_FLOOR < N_FLOORS && N_FLOORS> 0);
    int floor = elevio_floorSensor();
    while (floor == -1){
        floor = elevio_floorSensor();
        if (elevio_stopButton()){
            elevio_motorDirection(DIRN_STOP);
            break;
        }
    }
    int lastFloor = elevio_floorSensor();
    printf("Last floor: %d\n", lastFloor);

    while (1){
        elevio_floorIndicator(lastFloor);
        floor = elevio_floorSensor();
        if (floor == -1){
            elevio_doorOpenLamp(0);
        } else{
            elevio_doorOpenLamp(1);
        }
        lastFloor = (floor == -1) ? lastFloor : floor; //update floor when a new floor is reached

        if (elevio_stopButton()){
            elevio_motorDirection(DIRN_STOP);
            break;
        }
        
        //printf("Floor: %d\n", floor);
        if (lastFloor > DESIRED_FLOOR){
            elevio_motorDirection(DIRN_DOWN);
        } else if (lastFloor < DESIRED_FLOOR){
            elevio_motorDirection(DIRN_UP);
        }
        else if (lastFloor == DESIRED_FLOOR){
            elevio_motorDirection(DIRN_STOP);
            elevio_doorOpenLamp(1);
            printf("Desired floor reached\n");
            elevio_floorIndicator(lastFloor);
            break;
        }

        
    }

    return 0;
}
*/



//Basic example program
/*
int main(){
    elevio_init();
    
    printf("=== Example Program ===\n");
    printf("Press the stop button on the elevator panel to exit\n");

    elevio_motorDirection(DIRN_UP);
    Timer timer;
    initTimer(&timer);

    while(1){
        int floor = elevio_floorSensor();
        if (getTimePassed(&timer) >= 3){
            printf("3s passed\n");
            startTimer(&timer);
        }  
        if(floor == 0){
            elevio_motorDirection(DIRN_UP);
        }

        if(floor == N_FLOORS-1){
            elevio_motorDirection(DIRN_DOWN);
        }


        for(int f = 0; f < N_FLOORS; f++){
            for(int b = 0; b < N_BUTTONS; b++){
                int btnPressed = elevio_callButton(f, b);
                elevio_buttonLamp(f, b, btnPressed);
            }
        }

        if(elevio_obstruction()){
            elevio_stopLamp(1);
        } else {
            elevio_stopLamp(0);
        }
        
        if(elevio_stopButton()){
            elevio_motorDirection(DIRN_STOP);
            break;
        }
        
        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }

    return 0;
}
*/
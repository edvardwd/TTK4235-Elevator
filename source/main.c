#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"
#include "Timer.h"
#include "Elevator.h"

/*
int main(){
    //elevio_init();
    
    printf("=== Example Program ===\n");
    printf("Press the stop button on the elevator panel to exit\n");
    
    Elevator elevator;
    initElevator(&elevator);

    elevio_motorDirection(DIRN_UP);
    Timer timer;
    initTimer(&timer);

    

    printOrders(&elevator);
    while(1){
        updateLastFloor(&elevator.stateMachine);
        checkForOrders(&elevator);
        updateQueue(&elevator);
        orderQueue(&elevator);
        //int floor = elevator.stateMachine.lastFloor;
        int floor = elevio_floorSensor();
        int nextInQueue = *elevator.queue[0];

        if (floor == nextInQueue && floor != -1){
            clearOrders(&elevator, 0);
            printf("Queue cleared\n");
        }

        if(floor == 0){
            elevio_motorDirection(DIRN_UP);
            elevator.stateMachine.dir = DIRN_UP;
        }

        if(floor == N_FLOORS-1){
            elevio_motorDirection(DIRN_DOWN);
            elevator.stateMachine.dir = DIRN_DOWN;
        }
        
        if (floor == 2 && *elevator.fixedFloors[2] == 2 && elevio_floorSensor() == 2){
            printf("Reached floor: %d\n", floor);
            setDir(&elevator.stateMachine, DIRN_STOP);
            clearOrders(&elevator, 0);
            break;
        }
            

        if(elevio_stopButton()){
            elevio_motorDirection(DIRN_STOP);
            elevator.stateMachine.dir = DIRN_STOP;
            break;
        }
        
        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }

    destroyElevator(&elevator);
    return 0;
}

*/










int main() {
    Elevator elevator;
    initElevator(&elevator);

    elevatorMainLoop(&elevator);
    destroyElevator(&elevator);
    printf("Program finished.\n");

    return 0;
}




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
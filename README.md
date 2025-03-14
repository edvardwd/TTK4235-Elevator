# TTK4235 - Embedded Systems - Elevator lab
Group members: Edvard Wekre Dingsør and Havan Palo Darbandi
## C project to run a functioning elevator for the "Embedded Systems" course at NTNU.

You can either run the project on a physical elevator at Sanntidssalen, or using a simulator.

## How to run the physical elevator
1. Open a terminal in the project folder and run
```
elevatorserver
```
2. Open another terminal in the same folder and run
```
make clean
make
./elevator
```
3. Alternatively, you can run:
```
./buildAndRun.sh
```
after running
```
chmod +x buildAndRun.sh
```
once.
## How to run the simulator
1. Open a terminal in the project folder and run
```
./SimElevatorServer
```
if you are using Linux or WSL, or
```
./SimElevatorServerMacOS
```
if you are on MacOS.

2. Follow step 2 or 3 from the physical elevator instruction.
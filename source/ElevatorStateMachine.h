
//Declaration of enums
typedef enum{
    WAIT,
    MOVING,
    STOP
} State;

typedef enum{
    UP,
    DOWN
} Direction;

//State machine class
typedef struct{
    State state;
    Direction dir;
    int pos;

    bool doorOpen;
    
} ElevetorStateMachine;

//"Member functions"

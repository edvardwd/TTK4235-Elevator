package main

import "fmt"
import "net"
import "net/http"
import "encoding/json"
import "time"
//import "io/ioutil"
//import "math"
//import "runtime/pprof"
//import "os"

/*
Three processes:
    simulation
    network interface to the client (student) code (TCP)
    frontend interface (HTTP)


    HTTP 
        post    sim
        post    start
        post    stop
    TCP
        
    


*/

// "Input" and "Output" are as seen from the student code:
// "Outputs" are lights, direction
// "Inputs" are buttons, floor

type Output struct {
    MotorDirection      int         `json:"motorDirection"`
    OrderButtonLights   [][3]bool   `json:"orderButtonLights"`
    FloorIndicator      int         `json:"floorIndicator"`
    DoorOpenLight       bool        `json:"doorOpenLight"`
    StopButtonLight     bool        `json:"stopButtonLight"`
}
type Input struct {
    OrderButtons        [][3]bool   `json:"orderButtons"`
    FloorSensor         int         `json:"floorSensor"`
    StopButton          bool        `json:"stopButton"`
    ObstructionSwitch   bool        `json:"obstructionSwitch"`
    Connected           bool        `json:"connected"`
}
type SimContext struct {
    input           Input
    output          Output
    tcpPort         string
    connected       bool
    terminate       chan struct{}
}


func newSimContext(port string, numFloors int) *SimContext {
    return &SimContext{
        input : Input{
            OrderButtons : make([][3]bool, numFloors),
        },
        output : Output{
            OrderButtonLights : make([][3]bool, numFloors),
        },
        tcpPort     : port,
        terminate   : make(chan struct{}),
    }
}



func main(){
    contexts := make(map[string]*SimContext)
    
    
    httpCorsShim := func(
        res http.ResponseWriter, 
        req *http.Request, 
        fn func(res http.ResponseWriter, req *http.Request),
    ){
        res.Header().Add("Access-Control-Allow-Origin", "*")
        res.Header().Add("Access-Control-Allow-Methods", "POST, GET")
        res.Header().Add("Access-Control-Allow-Headers", "content-type")
        if(req.Method == "OPTIONS"){
            return
        }
        fn(res, req)
    }
    
    http.HandleFunc("/start", func(res http.ResponseWriter, req *http.Request){
        httpCorsShim(res, req, func(res http.ResponseWriter, req *http.Request){
            type Args struct {
                Port        string  `json:"port"`
                NumFloors   int     `json:"numFloors"`
            }
            var args Args
            json.NewDecoder(req.Body).Decode(&args)
            if _, ok := contexts[args.Port]; !ok {
                contexts[args.Port] = newSimContext(args.Port, args.NumFloors)
                go clientInterface(contexts[args.Port])
            }
            type Res struct {
                Input       Input   `json:"input"`
                Output      Output  `json:"output"`
            }
            json.NewEncoder(res).Encode(Res{contexts[args.Port].input, contexts[args.Port].output})
        })
    })
    http.HandleFunc("/stop", func(res http.ResponseWriter, req *http.Request){
        httpCorsShim(res, req, func(res http.ResponseWriter, req *http.Request){
            type Args struct {
                Port string `json:"port"`
            }
            var args Args
            json.NewDecoder(req.Body).Decode(&args)
            if _, ok := contexts[args.Port]; !ok {
                json.NewEncoder(res).Encode(false)
                return
            }
            <-contexts[args.Port].terminate
            delete(contexts, args.Port)
            json.NewEncoder(res).Encode(true)
        })
    })
    http.HandleFunc("/sim", func(res http.ResponseWriter, req *http.Request){
        httpCorsShim(res, req, func(res http.ResponseWriter, req *http.Request){
            var args map[string]Input
            json.NewDecoder(req.Body).Decode(&args)
            for k, v := range args {
                _, ok := contexts[k]
                if ok {
                    contexts[k].input = v
                }
            }
            response := make(map[string]Output)
            for k, v := range contexts {
                _, ok := args[k]
                if ok {
                    response[k] = v.output
                }
            }
            json.NewEncoder(res).Encode(response)
        })
    })
    
    fmt.Println("Simulator host started\nOpen 'sim.html' in a browser")
    http.ListenAndServe(":35382", nil)
}

func clientInterface(
    ctx *SimContext, // intentional race condition: all fields have a single writer
){
    fmt.Println("Starting instance on port", ctx.tcpPort)
    defer fmt.Println("Stopping", ctx.tcpPort)
    
    addr, _ := net.ResolveTCPAddr("tcp", fmt.Sprintf(":%s", ctx.tcpPort))
    acceptSock, _ := net.ListenTCP("tcp", addr)

    var buf [8]byte

    for {
        acceptSock.SetDeadline(time.Now().Add(time.Second))
        conn, err := acceptSock.AcceptTCP()
        if err != nil {
            select {
            case ctx.terminate <- struct{}{}:
                return
            default:
                continue
            }
        }
        ctx.connected = true

        read: for {
            conn.SetReadDeadline(time.Now().Add(time.Second))
            _, err := conn.Read(buf[0:])
            
            select {
            case ctx.terminate <- struct{}{}:
                conn.Close()
                return
            default:
                if err != nil {
                    conn.Close()
                    ctx.connected = false
                    break read
                }
            }
            
            switch buf[0] {
            case 1:
                ctx.output.MotorDirection = int(int8(buf[1]))
            case 2:
                ctx.output.OrderButtonLights[int(buf[2])][int(buf[1])] = buf[3] != 0
            case 3:
                ctx.output.FloorIndicator = int(buf[1])
            case 4:
                ctx.output.DoorOpenLight = buf[1] != 0
            case 5:
                ctx.output.StopButtonLight = buf[1] != 0
            case 6:
                conn.Write([]byte{6, bool2byte(ctx.input.OrderButtons[buf[2]][buf[1]]), 0, 0})
            case 7:
                a := ctx.input.FloorSensor
                if a == -1 {
                    conn.Write([]byte{7, 0, 0, 0})
                } else {
                    conn.Write([]byte{7, 1, byte(a), 0})
                }
            case 8:
                conn.Write([]byte{8, bool2byte(ctx.input.StopButton), 0, 0})
            case 9:
                conn.Write([]byte{9, bool2byte(ctx.input.ObstructionSwitch), 0, 0})
            default:
            }
        }
    }
}

func bool2byte(v bool) byte {
    if v {
        return 1
    }
    return 0
}

/*
type OrderButton struct {
    btn     int
    floor   int
    val     bool
}


func main(){

    clientConnected     := make(chan bool)
    terminate           := make(chan struct{})
    
    motorDir            := make(chan int)
    orderBtnLight       := make(chan OrderButton)
    floorIndicator      := make(chan int)
    doorLight           := make(chan bool)
    stopLight           := make(chan bool)
    
    orderButtons        := make(chan [][3]bool)
    floorSensor         := make(chan int)
    stopButton          := make(chan bool)
    obstruction         := make(chan bool)
    
    orderButtonPress    := make(chan OrderButton)
    stopButtonPress     := make(chan int)
    obstructionSwitch   := make(chan bool)
    
    moveInbounds        := make(chan struct{})    

    go clientInterface(
        15657,
        clientConnected,
        terminate,
        
        motorDir,
        orderBtnLight,
        floorIndicator,
        doorLight,
        stopLight,
        
        orderButtons,
        floorSensor,
        stopButton,
        obstruction,
    )

    go simulation(
        4,
        clientConnected,
        terminate,
        
        motorDir,
        orderBtnLight,
        floorIndicator,
        doorLight,
        stopLight,
        
        orderButtonPress,
        stopButtonPress,
        obstructionSwitch,
        moveInbounds,
        
        orderButtons,
        floorSensor,
        stopButton,
        obstruction,
        
    )

    go func(){
        time.Sleep(5*time.Second)
        pprof.Lookup("goroutine").WriteTo(os.Stdout, 1)
    }()

    for {
        select {
        case <- time.After(4*time.Second):
            fmt.Println("terminating")
            terminate <- struct{}{}
            terminate <- struct{}{}
            
        }
    }

}


func newSimulationState(numFloors int) SimulationState {
    var s SimulationState
    s.orderButtons = make([][3]bool, numFloors)
    s.orderLights  = make([][3]bool, numFloors)
    return s
}
func simulation(
    numFloors           int,
    clientConnected     <-chan bool,
    terminate           <-chan struct{},
    
    motorDir            <-chan int,
    orderBtnLight       <-chan OrderButton,
    floorIndicator      <-chan int,
    doorLight           <-chan bool,
    stopLight           <-chan bool,

    orderButtonPress    <-chan OrderButton,
    stopButtonPress     <-chan int,
    obstructionSwitch   <-chan bool,
    moveInbounds        <-chan struct{},

    orderButtons        chan<- [][3]bool,
    floorSensor         chan<- int,
    stopButton          chan<- bool,
    obstruction         chan<- bool,
){
    defer fmt.Println("sim terminated")

    state := newSimulationState(numFloors)

    floorFromPos := func(p float64) int {
        const close = 0.1
        f := math.Abs(p - math.Round(p))
        if f > close {
            return -1
        } else {
            return int(math.Round(p))
        }
    }
    isOutOfBounds := func(p float64) bool {
        return p > float64(numFloors)-0.5 || p < -0.5
    }
    
    const motorTickMS = 5
    const speed = 1.0/2.5
    motorTicker := time.Tick(motorTickMS * time.Millisecond)

    for {
        select {
        case state.connected  = <-clientConnected:
            if !state.connected {
                state.direction = 0
            }
        case a := <-orderBtnLight:
            state.orderLights[a.floor][a.btn] = a.val
        case state.direction        = <-motorDir:
        case state.floorIndicator   = <-floorIndicator:
        case state.doorLight        = <-doorLight:
        case state.stopButtonLight  = <-stopLight:
        case orderButtons           <- state.orderButtons:
        case floorSensor            <- floorFromPos(state.position):
        case stopButton             <- state.stopButton:
        case obstruction            <- state.obstruction:
        case <-motorTicker:
            state.position += float64(state.direction) * motorTickMS/1000.0 * speed
            if isOutOfBounds(state.position) {
                fmt.Printf(
                    "Elevator has moved out of bounds! " +
                    "Press [%s] to move the elevator within bounds...\n", "0",
                )
                state.direction = 0
            }
        case <-moveInbounds:
            if isOutOfBounds(state.position) {
                state.position = math.Min(state.position, float64(numFloors)-1)
                state.position = math.Max(state.position, 0)
            }
        case <-terminate:
            return
        }
    }
}




func clientInterface(
    tcpPort         int,
    clientConnected chan<- bool,
    terminate       <-chan struct{},
    
    motorDir        chan<- int,
    orderBtnLight   chan<- OrderButton,
    floorIndicator  chan<- int,
    doorLight       chan<- bool,
    stopLight       chan<- bool,

    orderButtons    <-chan [][3]bool,
    floorSensor     <-chan int,
    stopButton      <-chan bool,
    obstruction     <-chan bool,
){
    defer fmt.Println("ci terminated")
    
    addr, _ := net.ResolveTCPAddr("tcp", fmt.Sprintf(":%d", tcpPort))
    acceptSock, _ := net.ListenTCP("tcp", addr)

    var buf [8]byte

    for {
        acceptSock.SetDeadline(time.Now().Add(time.Second))
        conn, err := acceptSock.AcceptTCP()
        if err != nil {
            select {
            case <-terminate:
                return
            default:
                continue
            }
        }
        clientConnected <- true

        read: for {
            conn.SetReadDeadline(time.Now().Add(time.Second))
            _, err := conn.Read(buf[0:])
            
            select {
            case <-terminate:
                conn.Close()
                return
            default:
                if err != nil {
                    conn.Close()
                    clientConnected <- false
                    break read
                }
            }
            
            switch buf[0] {
            case 1:
                motorDir <- int(int8(buf[1]))
            case 2:
                orderBtnLight <- OrderButton{int(buf[1]), int(buf[2]), buf[3] != 0}
            case 3:
                floorIndicator <- int(buf[1])
            case 4:
                doorLight <- buf[1] != 0
            case 5:
                stopLight <- buf[1] != 0
            case 6:
                a := <-orderButtons
                conn.Write([]byte{6, bool2byte(a[buf[2]][buf[1]]), 0, 0})
            case 7:
                a := <-floorSensor
                if a == -1 {
                    conn.Write([]byte{7, 0, 0, 0})
                } else {
                    conn.Write([]byte{7, 1, byte(a), 0})
                }
            case 8:
                a := <-stopButton
                conn.Write([]byte{8, bool2byte(a), 0, 0})
            case 9:
                a := <-obstruction
                conn.Write([]byte{9, bool2byte(a), 0, 0})
            default:
            }
        }
    }
}



*/
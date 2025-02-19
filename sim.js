

async function postJSON(url, data){
    return fetch(url, {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify(data),
    })
    .then(async a => await a.json());
}

var xhr = new XMLHttpRequest();
function post(url, object){
    xhr.open("POST", url, false);
    xhr.setRequestHeader("Content-type", "application/json");
    xhr.send(JSON.stringify(object));
    if(xhr.status != 200){
        alert(xhr.response);
        return false;
    }
    if(xhr.responseText != ""){
        return JSON.parse(xhr.responseText);
    } else {
        return true;
    }
}

const minButtonPressedTimeMs    = 200;
const travelTimeBetweenFloorsMs = 2500;
const simTickIntervalMs         = 33;
const hostPort                  = 35382;

var sims = {};


async function start(port, numFloors){
    sims[port] = await postJSON(`http://localhost:${hostPort}/start`, {'port':(port).toString(), 'numFloors':numFloors || 4});
    var model = document.getElementById(port);
    if(!model){
        model = createElevatorModel(port);
        body.appendChild(model);
    }
}
async function stop(port){
    await postJSON(`http://localhost:${hostPort}/stop`, {'port':(port).toString()});
    delete sims[port];
    var model = document.getElementById(port);
    if(model){
        model.replaceWith();
    }
}
setInterval(function(){
    if(!Object.keys(sims).length){ return; }
    var req = {}
    for(k in sims){
        req[k] = sims[k].input;
    }
    postJSON(`http://localhost:${hostPort}/sim`, req)
    .then(function(res){
        if(!sims){
            return
        }
        for(k in res){
            if(!sims[k]){
                sims[k] = {};
            }
            sims[k].output = res[k];
            var model = document.getElementById(k);
            var numFloors = sims[k].output.orderButtonLights.length;
            for(var f = 0; f < numFloors; f++){
                for(var b = 0; b < 3; b++){
                    var btn = document.getElementById(`${k}-orderButtonLight-${f}-${b}`);
                    if(btn){
                        if(sims[k].output.orderButtonLights[f][b]){
                            btn.classList.add("lit");
                        } else {
                            btn.classList.remove("lit");
                        }
                    }
                }
            }
            for(var f = 0; f < numFloors; f++){
                var floorInd = document.getElementById(`${k}-floorIndicator-${f}`);
                if(floorInd){
                    if(sims[k].output.floorIndicator == f){
                        floorInd.classList.add("lit");
                    } else {
                        floorInd.classList.remove("lit");
                    }
                }
            }
            var door = document.getElementById(`${k}-door`);
            if(door){
                if(sims[k].output.doorOpenLight){
                    door.classList.add("lit");
                } else {
                    door.classList.remove("lit");
                }
            }
            var stop = document.getElementById(`${k}-stop`);
            if(stop){
                if(sims[k].output.stopButtonLight){
                    stop.classList.add("lit");
                } else {
                    stop.classList.remove("lit");
                }
            }
            
            
            var trackHeight = document.getElementById(`${k}-track`).offsetHeight;
            var car = document.getElementById(`${k}-car`);
            var trackLength = trackHeight - car.offsetHeight;
            
            
            
            if(sims[k].position == undefined){
                sims[k].position = Math.random() * (numFloors - 1);
                sims[k].t = new Date();
            }
            var now = new Date();
            var dt = now - sims[k].t;
            sims[k].t = now;
            

            sims[k].position = sims[k].position + sims[k].output.motorDirection * (dt/1000) / (travelTimeBetweenFloorsMs/1000);
            var top = positionToTrackTop(k);
            if(isNaN(top) || top <= 0 || top >= trackLength){
                car.classList.add("car-oob");
                sims[k].position = NaN;
            } else {
                car.classList.remove("car-oob");
            }
            car.style.top = top + "px";
            
            function floorFromPos(p){
                const close = 0.1
                var f = Math.abs(p - Math.round(p))
                if(f > close){
                    return -1
                } else {
                    return Math.round(p)
                }
            }
            sims[k].input.floorSensor = floorFromPos(sims[k].position)
        }
    });
}, simTickIntervalMs);

function positionToTrackTop(port, position){
    var numFloors = sims[k].output.orderButtonLights.length;
    var trackHeight = document.getElementById(`${port}-track`).offsetHeight;
    var car = document.getElementById(`${port}-car`);
    var carCenter = car.offsetHeight / 2;
    var trackLength = trackHeight - car.offsetHeight;
    var trackFloorHeight = (trackHeight + (car.offsetHeight - car.scrollHeight)) / numFloors;
    var top = Math.ceil((numFloors-0.5 - (position || sims[port].position)) * trackFloorHeight - carCenter);
    top = Math.max(top, 0);
    top = Math.min(top, trackLength);
    return top;
}
function trackTopToPosition(port, top){
    var numFloors = sims[port].output.orderButtonLights.length;
    var trackHeight = document.getElementById(`${port}-track`).offsetHeight;
    var car = document.getElementById(`${port}-car`);
    var carCenter = car.offsetHeight / 2;
    var trackFloorHeight = (trackHeight + (car.offsetHeight - car.scrollHeight)) / numFloors;
    return (numFloors - 0.5) - (top + carCenter) / trackFloorHeight;
}



var body = document.getElementById("body");


function createElevatorModel(port){
    function createConnectionControls(){
        var controls = document.createElement("div");
        
        return controls;
    }
    var numFloors = sims[port].output.orderButtonLights.length;
    
    function createButtonPanel(){
        function btn(f, b){
            var btn = document.createElement("button");
            btn.classList.add("panel-button", "button");
            btn.id = `${port}-orderButtonLight-${f}-${b}`;
            var t = null;
            btn.onmousedown = function(){
                sims[port].input.orderButtons[f][b] = true;
                t = new Date();
            }
            btn.onmouseup = function(){
                var dt = (new Date()) - t;
                if(dt < minButtonPressedTimeMs){
                    setTimeout(function(){
                        sims[port].input.orderButtons[f][b] = false;
                    }, minButtonPressedTimeMs - dt);
                } else {
                    sims[port].input.orderButtons[f][b] = false;
                }
            };
            return btn;
        }
        var panel = document.createElement("div");
        panel.classList.add("panel");
        for(var f = numFloors - 1; f >= 0; f--){
            var floorNumber = document.createElement("span");
            floorNumber.classList.add("panel-floornumber", "centered", "rounded");
            floorNumber.innerHTML = f;
            floorNumber.id = `${port}-floorIndicator-${f}`;
            panel.appendChild(floorNumber);
            panel.appendChild(f == numFloors-1  ? document.createElement("span") : btn(f, 0));
            panel.appendChild(f == 0            ? document.createElement("span") : btn(f, 1));
            panel.appendChild(btn(f, 2));
        }
        return panel;
    }
    
    function createTrack(){
        var track = document.createElement("div");
        track.classList.add("track");
        track.id = `${port}-track`;
        
        var car = document.createElement("div");
        car.classList.add("car");
        car.id = `${port}-car`;
        var mouseY;
        var carTopStart;
        car.addEventListener("mousedown", function(ev){
            carTopStart = parseInt(car.style.top);
            mouseY = ev.clientY;
        });
        document.addEventListener("mouseup", function(ev){
            mouseY = undefined;
        });
        document.addEventListener("mousemove", function(ev){
            if(mouseY !== undefined){
                sims[port].position = trackTopToPosition(port, (carTopStart + ev.clientY - mouseY));
            }
        });
        
        track.appendChild(car);
        
        return track;
    }
    
    function createFrontButtons(){
        var frontButtons = document.createElement("div");
        frontButtons.classList.add("frontButtons");
        
        var door = document.createElement("div");
        door.id = `${port}-door`;
        door.classList.add("door", "centered", "rounded");
        door.innerHTML = "Door";
        frontButtons.appendChild(door);
        
        var stop = document.createElement("button");
        stop.id = `${port}-stop`;
        stop.classList.add("stop", "centered", "rounded", "button");
        stop.innerHTML = "Stop";
        var t = null;
        stop.onmousedown = function(){
            sims[port].input.stopButton = true;
            t = new Date();
        }
        stop.onmouseup = function(){
            var dt = (new Date()) - t;
            if(dt < minButtonPressedTimeMs){
                setTimeout(function(){
                    sims[port].input.stopButton = false;
                }, minButtonPressedTimeMs - dt);
            } else {
                sims[port].input.stopButton = false;
            }
        };
        frontButtons.appendChild(stop);
        
        var obstruction = document.createElement("button");
        obstruction.classList.add("obstruction", "centered", "rounded");
        var toggle = document.createElement("div");
        toggle.classList.add("toggle", "rounded");
        obstruction.appendChild(toggle);
        obstruction.onclick = function(){
            if(obstruction.classList.contains("on")){
                sims[port].input.obstructionSwitch = false;
                obstruction.classList.remove("on");
            } else {
                sims[port].input.obstructionSwitch = true;
                obstruction.classList.add("on");
            }
        }
        frontButtons.appendChild(obstruction);
        
        return frontButtons;
    }
    
    
    
    var model = document.createElement("div");
    model.classList.add("model");
    model.id = port;
    
    model.appendChild(createConnectionControls());
    model.appendChild(createButtonPanel());
    model.appendChild(createTrack());
    model.appendChild(createFrontButtons());
    
    
    return model;
}


for(k in sims){
    body.appendChild(createElevatorModel(k));
}


start("15657", 4);
//body.appendChild(createElevatorModel("15657", 6));
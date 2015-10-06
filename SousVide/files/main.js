var output;

if (typeof String.prototype.startsWith != 'function') {
  // see below for better implementation!
  String.prototype.startsWith = function(str) {
    return this.indexOf(str) === 0;
  };
}

// change the slider val for id
function updateVal(id, val) {
	if ($('#' + id).val() == val) {
		return;
	}
	$('#' + id).val(val);
	$('#' + id).slider('refresh');
}

var websocketServerLocation;

function init() {
  output = document.getElementById("output");
  // updateTime();
  $("#flip_sous_state").attr("disabled", true);
  $("#relay1_state").attr("disabled", true);

  $("#relay1").attr("disabled", true);
  $(document).ready(function(){

    $("#flip_sous_state").attr("disabled", false);
    $("#relay1_state").attr("disabled", false);

    $("#flip_sous_state").val("leave").flipswitch('refresh');
    $("#relay1_state").val("leave").flipswitch('refresh');

    $("#flip_sous_state").change(
      function(){
//  	        $(this).toggleClass("down");
        // $(this).toggleClass('off');
        doSend('toggleSousvideOperation:' + (this.value === 'arrive' ? 'true' : 'false') );
        console.log("flip_sous_state clicked " + this.value);
      });

    $("#relay1_state").change(
      function(){
        doSend('toggleRelay:' + (this.value === 'arrive' ? 'true' : 'false'));
        // console.log("relay1_state clicked " + this.value);
      });

    var loc = location.host;
    if (loc==="") {
      loc="127.0.0.1";
    }

    websocketServerLocation = "ws://" + loc + "/";
    startWebSocket();
    // testWebSocket();
  });
}

function testWebSocket() {
  try {
    var wsUri = "ws://" + location.host + "/";
    websocket = new WebSocket(wsUri);
    websocket.onopen = function(evt) {
      onOpen(evt);
    };
    websocket.onclose = function(evt) {
      onClose(evt);
    };
    websocket.onmessage = function(evt) {
      onMessage(evt);
    };
    websocket.onerror = function(evt) {
      onError(evt);
    };
  }
  catch(err) {
      console.log("Web socket not connected");
  }
}

var timerID=0;
var socket;

function startWebSocket(){
  console.log("Trying to connect to ws at " + websocketServerLocation);
    socket = new WebSocket(websocketServerLocation);

    socket.onmessage = function(evt) {
      onMessage(evt);
    };

    socket.onopen = function(event) {
      onOpen(event);
    };

    socket.onclose = function(evt) {
      onClose(evt);
    };

    socket.onerror = function(evt) {
      onClose(evt);
    };
}

function onOpen(event){
  if(window.timerID){ /* a setInterval has been fired */
    window.clearInterval(window.timerID);
    window.timerID=0;
  }
}

function onClose(event){
  if(!window.timerID){ /* avoid firing a new setInterval, after one has been done */
    console.log("Starting 5 sec timer");
    window.timerID=setInterval(function(){startWebSocket(websocketServerLocation)}, 5000);
  }
  /* that way setInterval will be fired only once after loosing connection */
}

function onMessage(evt) {
  handlePayload(evt.data);
}

function parseCommand(com) {
	i = com.indexOf(":");
	var ret = new Array();
	ret[0] = com.substring(0, i);
	ret[1] = com.substring(i+1);
	return ret;
}

function handlePayload(payload) {
	var commands = payload.split(";");
	for (var i = 0; i < commands.length; i++) {
	    var command = commands[i];
	    var cmd = parseCommand(command);
	  //check if need to change the realy button state
		if (cmd[0].startsWith('relayState')) {
			var state = payload.substring('relayState'.length + 1);
		    // console.log('state = ' + state + ", relay1 = " + $("#relay1_state").val());
		    if( (state == 'true' && $("#relay1_state").val() == 'leave') ||
		        (state == 'false' && $("#relay1_state").val() == 'arrive')      )
		    {
		        if (state == 'true') {
		          $("#relay1_state").val('arrive').flipswitch('refresh');
		    		} else {
		          $("#relay1_state").val('leave').flipswitch('refresh');
		        }
		    }
		    else {
		    //  console.log('do nothing');
		    }
		}
		else if (cmd[0].startsWith('updatetime')) {
//			var newTime = payload.substring('updatetime'.length + 1);
			updateTime(cmd[1]);
		}
		else if (cmd[0].startsWith('temp')) {
//			var newTemp = payload.substring('temp'.length + 1);
			updateTemp(cmd[1]);
		}
		else if (cmd[0].startsWith('updatePID')) {
//			var pid = payload.substring('pid'.length + 1);
			updatePID(cmd[1]);
		}
		else if (cmd[0].startsWith('updateSetPoint')) {
//			var set = payload.substring('SetPoint'.length + 1);
			updateVal("SetPoint", cmd[1]);
		}
	}
}

var sliderChange = function(sliderValue) {
  console.log("Slider changed: " + sliderValue);
};

function sendValueChanged(id, value) {
  console.log(id + "="  + value);
  doSend('change-val-' + id + ':' + value);
}

function saveSettings() {
  console.log("save settings");
  doSend("saveSettings:");
}

function updatePID(pid) {
    var pidParts = pid.split(",");
    updateVal("p", pidParts[0]);
    updateVal("i", pidParts[1]);
    updateVal("d", pidParts[2]);
    console.log(pid);
}

function updateTime(newTime) {
    // newTime = "11:22:33";
    var timeParts = newTime.split(":");
    $('#hour').text(timeParts[0]);
    $('#min').text(timeParts[1]);
    $('#sec').text(timeParts[2]);
//    console.log(newTime);
}

function updateTemp(newTemp) {
    $('#temperature').text(newTemp);
    $('#svid_current_temp').text(newTemp);
//    console.log("temp=" + newTemp);
}

function onError(evt) {
  // writeToScreen('<span style="color: red;">ERROR:</span> ' + evt.data);
}

function doSend(message) {
//  writeToScreen("SENT: " + message);
  try {
    socket.send(message);
  } catch (e) {
    console.log("ws not initialized could not send message " + message);
  }
}

function writeToScreen(message) {
//  var pre = document.createElement("p");
//  pre.style.wordWrap = "break-word";
//  pre.innerHTML = message;
//  output.appendChild(pre);
}

function doDisconnect() {
  var disconnect = document.getElementById("disconnect");
  disconnect.disabled = true;
  socket.close();
}

window.addEventListener("load", init, false);

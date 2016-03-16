var websocketServerLocation;
var heartbeat_msg = '--heartbeat--', heartbeat_interval = null, missed_heartbeats = 0;

var timerID=0;
var socket;

function init() {
  output = document.getElementById("output");

  toggleShowOverlay(true);

//  // updateTime();
//  $("#flip_sous_state").attr("disabled", true);
//  $("#relay1_state").attr("disabled", true);
//
//  $("#relay1").attr("disabled", true);
//

 $(document).ready(function(){

  addValListener("target");
  addValListener("p");
  addValListener("i");
  addValListener("d");

  $('#target').trigger('mousemove', {type:'custom mouse move'});
  $('#p').trigger('mousemove', {type:'custom mouse move'});
  $('#i').trigger('mousemove', {type:'custom mouse move'});
  $('#d').trigger('mousemove', {type:'custom mouse move'});

  $('#reset').click(function() {
    doSend("app reset");
  });
  $('#play').click(function() {
    doSend("app play");
  });
  $('#stop').click(function() {
    doSend("app stop");
  });

  handleRelayState(false);

//    $("#flip_sous_state").attr("disabled", false);
//    $("#relay1_state").attr("disabled", false);
//
//    $("#flip_sous_state").val("leave").flipswitch('refresh');
//    $("#relay1_state").val("leave").flipswitch('refresh');

//     $("#flip_sous_state").change(
//       function(){
// //            $(this).toggleClass("down");
//         // $(this).toggleClass('off');
//         doSend('toggleSousvideOperation:' + (this.value === 'arrive' ? 'true' : 'false') );
//         console.log("flip_sous_state clicked " + this.value);
//       });

//     $("#relay1_state").change(
//       function(){
//         doSend('toggleRelay:' + (this.value === 'arrive' ? 'true' : 'false'));
//         // console.log("relay1_state clicked " + this.value);
//       });

    var loc = location.host;
    if (loc==="") {
      loc="127.0.0.1";
    }


//    setFormState(false);

    websocketServerLocation = "ws://" + loc + "/index.html?command=true";
    startWebSocket(false);
    // testWebSocket();
  });
}

function handleRelayState(state) {
  // <img id="isHeatingIcon " src="enable.png">
  // $('#isHeatingIcon').attr("src", "disable.png");

  if (state === "true") {
    $('#isHeatingIcon').attr("src", "enable.png");
    $('#isHeating').html("Heating in progress");
  } else {
    $('#isHeatingIcon').attr("src", "disable.png");
    $('#isHeating').html("Not Heating");
  }

  // $('#isHeatingIcon').click(function() {
  //   if ($(this).hasClass('active')) {
  //     doSend("app toggleEnable");
  //     $(this).html("Disabled <span><img src=\"disable.png\">");
  //
  //   } else {
  //     doSend("app toggleDisable");
  //     $(this).html("Enabled <span><img src=\"enable.png\">");
  //   }
  //
  // });
}

function addValListener(target) {
  $('#'+ target).on("change mousemove", function() {
   $(this).next().html($(this).val());
  });

}

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

var heartbeat_msg = '--heartbeat--', heartbeat_interval = null, missed_heartbeats = 0;

function onOpen(event){
  if(window.timerID){ /* a setInterval has been fired */
    window.clearInterval(window.timerID);
    window.timerID=0;
  }

  if (heartbeat_interval === null) {
        missed_heartbeats = 0;
        heartbeat_interval = setInterval(function() {
            try {
                missed_heartbeats++;
                if (missed_heartbeats >= 2)
                    throw new Error("Too many missed heartbeats.");
                socket.send("app " + heartbeat_msg);
            } catch(e) {
                clearInterval(heartbeat_interval);
                heartbeat_interval = null;
                console.warn("Closing connection. Reason: " + e.message);
                socket.close();
            }
        }, 5000);
    }

    toggleShowOverlay(false);
    // doSend("app getInitData")
    // setFormState(true);
}

function onClose(event){
  // if(!window.timerID){ /* avoid firing a new setInterval, after one has been done */
  //   console.log("Starting 5 sec timer");
  //   window.timerID=setInterval(function(){startWebSocket(websocketServerLocation)}, 5000);
  // }
  /* that way setInterval will be fired only once after loosing connection */

  // setFormState(false);

  toggleShowOverlay(true);

  // $("#NotConnectedDiv").prop('hidden',false);
  // $("#ConnectedDIv").prop('hidden', true);
}

function onMessage(evt) {
  if (evt.data === heartbeat_msg) {
      // reset the counter for missed heartbeats
      missed_heartbeats = 0;
      return;
  }

  handlePayload(evt.data);
}

function parseCommand(com) {
  i = com.indexOf(":");
  var ret = new Array();
  ret[0] = com.substring(0, i);
  ret[1] = com.substring(i+1);
  return ret;
}

//updates from webserver
function handlePayload(payload) {
  var commands = payload.split(";");
  for (var i = 0; i < commands.length; i++) {
      var command = commands[i];
      var cmd = parseCommand(command);
    //check if need to change the realy button state
    if (cmd[0].startsWith('relayState')) {
      var state = cmd[1];
      handleRelayState(state);
    }
    else if (cmd[0].startsWith('updateWorkCounter')) {
      $("#currentTemp").html(cmd[1]);
    }
    else if (cmd[0].startsWith('updateTemp')) {
      $("#workCounter").html(cmd[1]);
      // updateTemp(cmd[1]);
    }
    else if (cmd[0].startsWith('updatePID')) {
      updatePID(cmd[1]);
    }
    else if (cmd[0].startsWith('updateSetPoint')) {
      updateVal("target", cmd[1]);
    }
    else if (cmd[0].startsWith('updateWIFI')) {
      // updateWifi(cmd[1]);
    }
    else if (cmd[0].startsWith('sousvideMode')) {
      updateSousvideMode(cmd[1]);
    }
  }
}

function updateSousvideMode(mode) {
  if (mode==="manual"){
    if (!$('#enable_override_mode').hasClass('active')) {
        $('#enable_override_mode').addClass('active');
    }
  } else {
      if ($('#enable_override_mode').hasClass('active')) {
          $('#enable_override_mode').removeClass('active');
        }
  }
}

// change the slider val for id
function updateVal(id, val) {
	if ($('#' + id).val() == val) {
		return;
	}

	$('#' + id).val(val);
  $('#' + id).trigger('mousemove', {type:'custom mouse move'});
}

function updatePID(pid) {
    var pidParts = pid.split(",");
    updateVal("p", pidParts[0]);
    updateVal("i", pidParts[1]);
    updateVal("d", pidParts[2]);
    console.log(pid);
}

function connect() {
      var wifiCmd = $('#SSID').val() + "," + $('#Password').val();
      doSend('connect:' + wifiCmd);
}

toggelshowWorking = false;
function toggleShowOverlay(state) {
  if (toggelshowWorking == false) {
    return;
  }
    if (state === true) {
      $("#NotConnectedDiv").prop('hidden',false);
      $("#ConnectedDIv").prop('hidden', true);
    }
    else {
      $("#NotConnectedDiv").prop('hidden',true);
      $("#ConnectedDIv").prop('hidden', false);
    }
}

function sendValueChanged(id, value) {
  // console.log(id + "="  + value);
  doSend('sousvide change-val-' + id + ' ' + value);
}

var sousvideOnState = false;
function toggleSousState(newState) {
    if (newState) {
      $('#work');
    }
}

function reset() {
  doSend("app reset");
}

function sendStateChange(newState) {
    doSend("app state " + newState);
}

function doSend(message) {
//  writeToScreen("SENT: " + message);
  try {
    socket.send(message);
  } catch (e) {
    console.log("ws not initialized could not send message " + message);
  }
}

function updateWorkTime(newTime) {
    // new  Time = "11:22:33";
    $('#workCounter').text(newTime);
}

function handleInitData() {

}

window.addEventListener("load", init, false);

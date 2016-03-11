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
  //  $('#target').on("change mousemove", function() {
  //   $(this).next().html($(this).val());
  //  });
  addValListener("target");
  addValListener("p");
  addValListener("i");
  addValListener("d");

  $('#target').trigger('mousemove', {type:'custom mouse move'});
  $('#p').trigger('mousemove', {type:'custom mouse move'});
  $('#i').trigger('mousemove', {type:'custom mouse move'});
  $('#d').trigger('mousemove', {type:'custom mouse move'});


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

    websocketServerLocation = "ws://" + loc + "/";
    startWebSocket();
    // testWebSocket();
  });
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
                socket.send(heartbeat_msg);
            } catch(e) {
                clearInterval(heartbeat_interval);
                heartbeat_interval = null;
                console.warn("Closing connection. Reason: " + e.message);
                socket.close();
            }
        }, 5000);
    }

    toggleShowOverlay(false);
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
      updateTime(cmd[1]);
    }
    else if (cmd[0].startsWith('temp')) {
      updateTemp(cmd[1]);
    }
    else if (cmd[0].startsWith('updatePID')) {
      updatePID(cmd[1]);
    }
    else if (cmd[0].startsWith('updateSetPoint')) {
      updateVal("SetPoint", cmd[1]);
    }
    else if (cmd[0].startsWith('updateWIFI')) {
      updateWifi(cmd[1]);
    }
  }
}

function connect() {
      var wifiCmd = $('#SSID').val() + "," + $('#Password').val();
      doSend('connect:' + wifiCmd);
}

function toggleShowOverlay(state) {
    // if (state === true) {
    //   $("#NotConnectedDiv").prop('hidden',false);
    //   $("#ConnectedDIv").prop('hidden', true);
    // }
    // else {
      $("#NotConnectedDiv").prop('hidden',true);
      $("#ConnectedDIv").prop('hidden', false);
    // }
}

function sendValueChanged(id, value) {


  // console.log(id + "="  + value);
  doSend('fromClient change-val-' + id + ':' + value);
}

function doSend(message) {
//  writeToScreen("SENT: " + message);
  try {
    socket.send(message);
  } catch (e) {
    console.log("ws not initialized could not send message " + message);
  }
}

function updateTime(newTime) {
    // new  Time = "11:22:33";
    $('#currentTime').text(newTime);
    // var timeParts = newTime.split(":");
    // $('#hour').text(timeParts[0]);
    // $('#min').text(timeParts[1]);
    // $('#sec').text(timeParts[2]);
//    console.log(newTime);
}

window.addEventListener("load", init, false);

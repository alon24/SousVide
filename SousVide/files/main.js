var output;

if (typeof String.prototype.startsWith != 'function') {
  // see below for better implementation!
  String.prototype.startsWith = function(str) {
    return this.indexOf(str) === 0;
  };
}

//change the slider val for id
function channgeVal(id, val) {
  // $('#needed_temp').val(val);
  // $('#needed_temp').slider('refresh');

  $('#' + id).val(val);
  $('#' + id).slider('refresh');
}

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


      //put in slider labels
      //new an attempt to label the min max values on the loan Amount slider page1
      //code found on jquery forum: https://forum.jquery.com/topic/how-do-i-add-text-labels-below-slider
      //Note: Use of the theme for the slider induced some issues, so
      //had to put in following inline alteration to the text are would show up with a text-shadow:
      //text-shadow:none; color:black; font-weight:normal
      $.fn.extend({
        sliderLabels: function(left,right) {
          var $this = $(this);
          var $sliderdiv= $this.next("div.ui-slider[role='application']");
          //
          $sliderdiv
          .prepend('<span class="ui-slider-inner-label" style="position: absolute; left:0px; top:20px; text-shadow:none; color:black; font-weight:normal">'+left+ '</span>')
          // .append('<span class="ui-slider-inner-label" style="position: absolute; right:0px; bottom:20px; text-shadow:none; color:black; font-weight:normal">'+right+ '</span>');
          //
        }
       });
       //
      $('#slider-PrincipleAmnt').sliderLabels('Min: $20', 'Max: $999');

  testWebSocket();
  });
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
  doSend("cmd:saveSettings");
}
function testWebSocket() {
  try {
    var wsUri = "ws://" + location.host + "/";
    websocket = new WebSocket(wsUri);
    websocket.onopen = function(evt) {
      onOpen(evt)
    };
    websocket.onclose = function(evt) {
      onClose(evt)
    };
    websocket.onmessage = function(evt) {
      onMessage(evt)
    };
    websocket.onerror = function(evt) {
      onError(evt)
    };
  }
  catch(err) {
      console.log("Web socket not connected");
  }
}

function onOpen(evt) {
  writeToScreen("CONNECTED");
  doSend("Sming love WebSockets");\

  //query sming data
  //time
  //pid
  doSend("query:time,p,i,d,state,needed_temp");

}

function onClose(evt) {
  writeToScreen("DISCONNECTED");
}

function onMessage(evt) {
  //    writeToScreen('<span style="color: blue;">RESPONSE: ' + evt.data+'</span>');
  //websocket.close();
  handlePayload(evt.data);
}


function handlePayload(payload) {
	//check if need to change the realy button state
	if (payload.startsWith('relayState')) {
		var state = payload.substring('relayState'.length + 1);
		if (state === 'true') {
      $("#relay1_state").val("leave").flipswitch('refresh');
			// $('#relay1').prop('checked', (state === ));
			// $('#relay1').addClass("ss");
		}
	}
	else if (payload.startsWith('updatetime')) {
		var newTime = payload.substring('updatetime'.length + 1);
		updateTime(newTime);
	}
	else if (payload.startsWith('temp')) {
		var newTemp = payload.substring('temp'.length + 1);
		updateTemp(newTemp);
	}
  else if (payload.startsWith('pid')) {
		var pid = payload.substring('pid'.length + 1);
		updatePID(pid);
	}
}

function updatePID(pid) {
    var pidParts = pid.split(",");
    $('#p').value=pidParts[0];
    $('#i').value=pidParts[1];
    $('#d').value=pidParts[2];
    console.log(pid);
}

function updateTime(newTime) {
    // newTime = "11:22:33";
    var timeParts = newTime.split(":");
    $('#hour').text(timeParts[0]);
    $('#min').text(timeParts[1]);
    $('#sec').text(timeParts[2]);
    console.log(newTime);
}

function updateTemp(newTemp) {
    $('#temperature').text(newTemp);
    $('#svid_current_temp').text(newTemp);
    console.log("temp=" + newTemp);
}

function onError(evt) {
  // writeToScreen('<span style="color: red;">ERROR:</span> ' + evt.data);
}

function doSend(message) {
//  writeToScreen("SENT: " + message);
  try {
    websocket.send(message);
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
  websocket.close();
}

window.addEventListener("load", init, false);

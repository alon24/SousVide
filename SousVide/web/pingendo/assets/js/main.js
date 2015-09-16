var output;

if (typeof String.prototype.startsWith != 'function') {
  // see below for better implementation!
  String.prototype.startsWith = function(str) {
    return this.indexOf(str) === 0;
  };
}


function init() {
  output = document.getElementById("output");
  // updateTime();
  testWebSocket();

}

function testWebSocket() {
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

function onOpen(evt) {
  writeToScreen("CONNECTED");
  doSend("Sming love WebSockets");
}

function onClose(evt) {
  writeToScreen("DISCONNECTED");
}

function onMessage(evt) {
  //    writeToScreen('<span style="color: blue;">RESPONSE: ' + evt.data+'</span>');
  //websocket.close();
  handlePayload(evt.data);
  //    if (evt.data.startsWith('relayState')) {
  //      var state = evt.data.substring('relayState'.length +1);
  //      if (state === 'true') {
  //        $('#relay1').prop('checked', true);
  //      }
  //      else {
  //        $('#relay1').prop('checked', false);
  //      }
  //    }
}


function handlePayload(payload) {
	//check if need to change the realy button state
	if (payload.startsWith('relayState')) {
		var state = payload.substring('relayState'.length + 1);
		if (state === 'true') {
			// $('#relay1').prop('checked', (state === ));
			// $('#relay1').addClass("ss");
		}
	} 
	else if (payload.startsWith('updatetime')) {
		var newTime = payload.substring('updatetime'.length + 1);
    updatetime(newTime);  
	}
}

function updateTime(newTime) {
    // newTime = "11:22:33";
    var timeParts = newTime.split(":");
    $('#hour').text(timeParts[0]);
    $('#min').text(timeParts[1]);
    $('#sec').text(timeParts[2]);
    console.log(newTime);
}

function onError(evt) {
  writeToScreen('<span style="color: red;">ERROR:</span> ' + evt.data);
}

function doSend(message) {
//  writeToScreen("SENT: " + message);
  websocket.send(message);
}

function writeToScreen(message) {
  var pre = document.createElement("p");
  pre.style.wordWrap = "break-word";
  pre.innerHTML = message;
  output.appendChild(pre);
}

function doDisconnect() {
  var disconnect = document.getElementById("disconnect");
  disconnect.disabled = true;
  websocket.close();
}

window.addEventListener("load", init, false);
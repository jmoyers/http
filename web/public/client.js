var ws = new WebSocket("ws://localhost:8080/chat");

ws.onopen = function() {
  console.log("Sending message");
  ws.send("Message to send");
}

ws.onmessage = function (e) { 
  console.log(e.data);
}

ws.onclose = function() { 
  console.log("closed");
}

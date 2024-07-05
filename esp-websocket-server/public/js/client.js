const ws = new WebSocket('ws://localhost:3000');

ws.onopen = function () {
  console.log('Connected to the server');
};

ws.onmessage = function (event) {
  console.log('Received from server: ' + event.data);
};

ws.onclose = function () {
  console.log('Disconnected from the server');
};

function sendMessage() {
  const message = document.getElementById('message').value;
  ws.send(message);
  console.log('Sent to server: ' + message);
}

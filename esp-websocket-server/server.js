const express = require('express');
const http = require('http');
const path = require('path');
const WebSocket = require('ws');

const app = express();
const port = 3000;

// Serve static files from the 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

// Serve the main HTML file
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Create an HTTP server
const server = http.createServer(app);

// Create a WebSocket server and attach it to the HTTP server
const wss = new WebSocket.Server({ server });

wss.on('connection', function connection(ws) {
  console.log('A new client connected!');

  // Function to log all existing clients
  const logExistingClients = () => {
    console.log('Existing clients:');
    wss.clients.forEach(client => {
      if (client.readyState === WebSocket.OPEN) {
        console.log(client._socket.remoteAddress);
      }
    });
  };

  // Call the function to log existing clients
  logExistingClients();

  ws.on('message', function incoming(message) {
    console.log('received: %s', message);

    // Broadcast the message to all clients except the sender
    wss.clients.forEach(function each(client) {
      if (client.readyState === WebSocket.OPEN) {
        console.log(`Sending message to client: ${client._socket.remoteAddress}`);
        client.send(message.toString()); // Ensure message is sent as a string
      }
    });
  });

  ws.send('Welcome to the WebSocket server!');
});

// Start the server
server.listen(port, () => {
  console.log(`Server is listening on http://localhost:${port}`);
});

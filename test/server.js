var WebSocketServer = require('websocket').server;
var http = require('http');

var server = http.createServer(function(request, response) {

});
server.listen(8971, function() { });

// create the server
wsServer = new WebSocketServer({
    httpServer: server
});

// WebSocket server
wsServer.on('request', function(request) {
    console.log("client connected");
    var connection = request.accept(null, request.origin);
    var cinterval = setInterval(function () {
        connection.send("1 Sec interval message from the server!");
        console.log("Sending message");
    }, 1000);
    // This is the most important callback for us, we'll handle
    // all messages from users here.
    connection.on('message', function(message) {
        console.log("Message: " + message.utf8Data);
        if (message.type === 'utf8') {
        // process WebSocket message
        }
    });

    connection.on('close', function(connection) {
        // close user connection
        console.log("Client disconnected");
        clearInterval(cinterval);
    });
});

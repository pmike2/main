
const express = require('express');
const path = require('path');
const app = express();
const fs = require('fs');
app.use(express.json());

// pour pouvoir référencer des fichiers
app.use(express.static(path.join(__dirname, "./css")));
//app.use(express.static(path.join(__dirname, "../js")));

const http = require('http');
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);


// dans un navigateur Web mettre http://localhost:3001/ affiche index.html
app.get('/', (req, res) => {
	res.sendFile(path.resolve(__dirname, 'racing.html'));
});


io.on('connection', (socket) => {
	socket.on('data', (data) => {
		console.log(data);
		io.sockets.emit("data", data);
	});
});


// on écoute sur le port 3001
server.listen(3001, () => {
	console.log('listening on *:3001');
});

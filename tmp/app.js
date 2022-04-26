/*
Partie serveur, à démarrer en faisant :
node app.js

Ecoute (POST) le code c++ et la page HTML (socket.on())
Quand le code c++ envoie par POST, la partie serveur envoie au HTML (io.sockets.emit()) ce qu'il a reçu
Quand le HTML envoie par socket.emit(), la partie serveur met à jour la variable js_config
Quand le code c++ fait un get la partie serveur lui renvoie la valeur actuelle de js_config

*/


const express = require('express');
const app = express();
app.use(express.json());
const http = require('http');
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);

var js_config= {};


// dans un navigateur Web mettre http://localhost:3000/ affiche index.html
app.get('/', (req, res) => {
	res.sendFile(__dirname + '/index.html');
});


// renvoie la valeur de js_config
app.get('/config', (req, res) => {
	res.statusCode = 200;
	res.setHeader('Content-Type', 'application/json');
	res.end(JSON.stringify(js_config));
});


// met à jour la valeur de js_config et emit
app.post('/config', (req, res) => {
	//console.log(req.body);
	js_config= req.body;
	res.statusCode = 200;
	/*res.setHeader('Content-Type', 'text/plain');
	res.end('OK bien recu\n');*/
	res.end();
	io.sockets.emit("send2client", js_config);
});


// connection faite par client ; on écoute les messages 'send2server'
io.on('connection', (socket) => {
	//console.log('a user is connected');
	socket.on('send2server', (msg) => {
		//console.log('message: ' + JSON.stringify(msg));
		js_config= msg;
	});
});


// on écoute sur le port 3000
server.listen(3000, () => {
	console.log('listening on *:3000');
});

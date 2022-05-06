/*
Partie serveur, à démarrer en faisant :
node app.js

Ecoute (POST) le code c++ et la page HTML (socket.on())
Quand le code c++ envoie par POST, la partie serveur envoie au HTML (io.sockets.emit()) ce qu'il a reçu
Quand le HTML envoie par socket.emit(), la partie serveur met à jour la variable js_config
Quand le code c++ fait un get la partie serveur lui renvoie la valeur actuelle de js_config

*/


const express = require('express');
var path = require('path');
const app = express();
const fs = require('fs');
app.use(express.json());
// pour pouvoir référencer des fichiers présents dans le dossier courant
app.use(express.static(__dirname));
const http = require('http');
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);

var js_config_server= {};


// dans un navigateur Web mettre http://localhost:3000/ affiche index.html
app.get('/', (req, res) => {
	res.sendFile(__dirname + '/index.html');
});

app.get('/test', (req, res) => {
	res.sendFile(__dirname + '/polygons.html');
});

/*
function get_configs_files(req, res, next) {
	fs.readdir(path.resolve(__dirname, 'configs'), function (err, images) {
		if (err) { return next(err); }
		res.filenames= images.map(x => path.resolve(__dirname, 'configs', x));
		next();
	});
}*/

function list_files(folder) {
	return new Promise(function(resolve, reject) {
		fs.readdir(folder, function(err, filenames){
			if (err) 
				reject(err); 
			else 
				resolve(filenames);
		});
	});
};
/*
app.get('/get_list_configs', get_configs_files, (req, res) => {
	console.log("ok");
	res.statusCode = 200;
	res.setHeader('Content-Type', 'application/json');
	res.end(JSON.stringify(res.filenames));
	io.sockets.emit("send_list_configs", js_config);
});*/


// renvoie la valeur de js_config
app.get('/config', (req, res) => {
	res.statusCode = 200;
	res.setHeader('Content-Type', 'application/json');
	res.end(JSON.stringify(js_config_server));
});


// met à jour la valeur de js_config et emit
/*app.post('/config', (req, res) => {
	js_config= req.body;
	res.statusCode = 200;
	res.end();
	io.sockets.emit("send2client", js_config);
});
*/

io.on('connection', (socket) => {
	//console.log('a user is connected');

	socket.on('get_list_configs', () => {
		list_files(path.resolve(__dirname, 'configs'))
		.then((files) => {
			var model_data= fs.readFileSync(path.resolve(__dirname, "model.json"), "utf8");

			let data2send= {"model" : JSON.parse(model_data), "configs" : {}};

			for (let i=0; i<files.length; ++i) {
				var file_data= fs.readFileSync(path.resolve(__dirname, 'configs', files[i]), "utf8");
				data2send["configs"][files[i]]= JSON.parse(file_data);
			}
			io.sockets.emit("send_list_configs", data2send);
		})
		.catch((error) => console.log(error));
	});

	socket.on('save_config', (js_config) => {
		list_files(path.resolve(__dirname, 'configs'))
		.then((files) => {
			let max_idx= 0;
			for (let i=0; i<files.length; ++i) {
				const regex = new RegExp("^config_([0-9]+).json$");
				const found = files[i].match(regex);
				if (found) {
					const idx= parseInt(found[1]);
					if (max_idx< idx) {
						max_idx= idx;
					}
				}
			}
			let saved_path= "config_"+ (max_idx+ 1).toLocaleString(undefined, {minimumIntegerDigits: 2})+ ".json";
			fs.writeFile(path.resolve(__dirname, "configs", saved_path), JSON.stringify(js_config, null, 2), err => {
				if (err) {
					console.error(err);
					return;
				}
			})
			io.sockets.emit("config_saved");
		})
		.catch((error) => console.log(error));
	});

	socket.on('config_changed', (js_config) => {
		js_config_server= js_config
		//console.log(js_config_server);
	});
});


// on écoute sur le port 3000
server.listen(3000, () => {
	console.log('listening on *:3000');
});

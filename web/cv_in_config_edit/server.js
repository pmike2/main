
const express = require('express');
const path = require('path');
const app = express();
const fs = require('fs');
const walk= require('walk');
app.use(express.json());

// pour pouvoir référencer des fichiers
app.use(express.static(path.join(__dirname, "../css")));
app.use(express.static(path.join(__dirname, "../js")));

const http = require('http');
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);

var args = process.argv.slice(2);

const root_data= args[0];
var dir_configs= null;

var js_config_server= {};


// dans un navigateur Web mettre http://localhost:3003/ affiche index.html
app.get('/', (req, res) => {
	res.sendFile(path.resolve(__dirname, 'index.html'));
});


// renvoie la valeur de js_config_server
app.get('/config', (req, res) => {
	res.statusCode = 200;
	res.setHeader('Content-Type', 'application/json');
	res.json(js_config_server);
});


io.on('connection', (socket) => {
	//console.log('a user is connected');

	socket.on('client2server_get_data', () => {
		let data2send= {};

		let model_path= "./json/cv_in_model.json";
		const json_data= fs.readFileSync(model_path, "utf8");
		data2send[model_path.split("/").pop()]= JSON.parse(json_data);

		const walker= walk.walk(root_data, {followLinks: true});

		function walker_func(directory, filename) {
			const abs_path= path.resolve(directory, filename);
			
			if (filename.split(".").pop()== "json") {
				const json_data= fs.readFileSync(abs_path, "utf8");
				data2send[filename]= JSON.parse(json_data);
				if (filename.includes("cv_in")) {
					dir_configs= directory;
				}
			}
			else {
				data2send[abs_path]= null;
			}
		}

		walker.on('file', (root, stat, next) => {
			walker_func(root, stat.name);
			next();
		}).on('symlink', (root, stat, next) => {
			walker_func(root, stat.name);
			next();
		}).on('end', () => {
			io.sockets.emit("server2client_data_send", data2send);
		});
	});


	socket.on('client2server_save_config', (js_config) => {
		let max_idx= 0;
		fs.readdirSync(dir_configs).forEach(file => {
			const regex = new RegExp("^cv_in_config_([0-9]+).json$");
			const found = file.match(regex);
			if (found) {
				const idx= parseInt(found[1]);
				if (max_idx< idx) {
					max_idx= idx;
				}
			}
		});

		let saved_path= "cv_config_"+ (max_idx+ 1).toLocaleString(undefined, {minimumIntegerDigits: 2})+ ".json";
		fs.writeFile(path.resolve(dir_configs, saved_path), JSON.stringify(js_config, null, 2), err => {
			if (err) {
				console.error(err);
				return;
			}
		})
		io.sockets.emit("server2client_config_saved", saved_path);
	});


	socket.on('client2server_js_config_changed', (js_config) => {
		js_config_server= js_config
		io.sockets.emit("server2client_config_changed", JSON.stringify(js_config_server));
	});
});


// on écoute sur le port 3003
server.listen(3003, () => {
	console.log('listening on *:3003');
});

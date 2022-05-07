const express = require('express');
const path = require('path');
const app = express();
const fs = require('fs');
const walk= require('walk');
app.use(express.json());
// pour pouvoir référencer des fichiers présents dans le dossier courant
app.use(express.static(__dirname));
const http = require('http');
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);

const root_data= path.resolve(__dirname, 'data2serve');
const dir_configs= path.resolve(__dirname, 'data2serve', 'configs');

var js_config_server= {};


// dans un navigateur Web mettre http://localhost:3000/ affiche index.html
app.get('/', (req, res) => {
	res.sendFile(path.resolve(__dirname, 'index.html'));
});

app.get('/polygons_test', (req, res) => {
	res.sendFile(path.resolve(__dirname, 'polygons_test.html'));
});

app.get('/timeline_test', (req, res) => {
	res.sendFile(path.resolve(__dirname, 'timeline_test.html'));
});


// renvoie la valeur de js_config_server
app.get('/config', (req, res) => {
	res.statusCode = 200;
	res.setHeader('Content-Type', 'application/json');
	res.json(js_config_server);
});


io.on('connection', (socket) => {
	//console.log('a user is connected');

	socket.on('get_data', () => {
		let data2send= {};
		const walker= walk.walk(root_data, {followLinks: false});

		walker.on('file', (root, stat, next) => {
			const filename= stat.name;
			const abs_path= path.resolve(root, filename);
			
			if (filename.split(".").pop()== "json") {
				const json_data= fs.readFileSync(abs_path, "utf8");
				//data2send[abs_path]= JSON.parse(json_data);
				data2send[filename]= JSON.parse(json_data);
			}
			else {
				//data2send[abs_path]= null;
				data2send[filename]= null;
			}
			next();
		}).on('end', () => {
			//console.log(JSON.stringify(data2send));
			io.sockets.emit("send_data", data2send);
		});
	});


	socket.on('save_config', (js_config) => {
		let max_idx= 0;
		fs.readdirSync(dir_configs).forEach(file => {
			const regex = new RegExp("^config_([0-9]+).json$");
			const found = file.match(regex);
			if (found) {
				const idx= parseInt(found[1]);
				if (max_idx< idx) {
					max_idx= idx;
				}
			}
		});

		let saved_path= "config_"+ (max_idx+ 1).toLocaleString(undefined, {minimumIntegerDigits: 2})+ ".json";
		fs.writeFile(path.resolve(dir_configs, saved_path), JSON.stringify(js_config, null, 2), err => {
			if (err) {
				console.error(err);
				return;
			}
		})
		io.sockets.emit("config_saved", saved_path);
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

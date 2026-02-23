#!/usr/bin/env python3

#import socketio

import socket
import json

#sio = socketio.SimpleClient()
#sio.connect('http://localhost:5000')
#sio.emit('my message', {'foo': 'bar'})

connexion_serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
connexion_serveur.connect(("127.0.0.1", 8080))
connexion_serveur.send(json.dumps({"aaa" : "bbb"}).encode("utf-8"))

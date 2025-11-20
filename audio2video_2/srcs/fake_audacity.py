#!/usr/bin/env python3

import time
import socket
import json


connexion_serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
connexion_serveur.connect(("127.0.0.1", 8080))
#connexion_serveur.setblocking(0)
connexion_serveur.send(json.dumps({"delta_offset" : 512, "samplerate" : 48000}).encode("utf-8"))


while True:
	time.sleep(3.0)
	envelope = {"idx_freq_group": 0, "amplitudes": [100.0 + i * 10.0 for i in range(100)]}
	connexion_serveur.send(json.dumps(envelope).encode("utf-8"))

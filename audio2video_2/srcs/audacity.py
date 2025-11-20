#!/usr/bin/env python3

"""
Pour faire fonctionner cs script il faut 

créer un env conda
activer cet env
installer pyaudacity
ouvrir Audacity préférences / Modules -> activé puis redémarrer Audacity

ref des actions de pyaudacity : https://manual.audacityteam.org/man/scripting_reference.html
"""

import os
import sys
import time
import json
import socket
import signal

import pyaudacity as pa

pa_active = False


#json_path = "../data/wav/bcl7/bcl7.json"
json_path = "../data/wav/metronome/metronome.json"

looping = True


def on_ctrl_c(sig, frame):
	if pa_active:
		pa.do("Stop")
	sys.exit(0)
signal.signal(signal.SIGINT, on_ctrl_c)


with open(json_path) as f:
	js = json.load(f)

envelopes = js["envelopes"]
envelopes.sort(key=lambda x : x["block_idx"])

connexion_serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
connexion_serveur.connect(("127.0.0.1", 8080))
connexion_serveur.send(json.dumps({"delta_offset" : js["delta_offset"], "samplerate" : js["samplerate"]}).encode("utf-8"))

if pa_active:
	pa.do("CursProjectStart")
# sleep nécessaire entre les pa.do sinon plante
time.sleep(3.0)

time_start = time.time()
if pa_active:
	pa.do("Play")
duration = float(js["frames"])/ float(js["samplerate"])

last_idx_env = -1
while True:
	time_current = time.time()
	if time_current > time_start + duration:
		if looping:
			# ne fonctionne pas car nécessite des sleep entre chaque pa.do
			
			#last_idx_env = -1
			#time_start += duration
			#pa.do("Stop")
			#pa.do("CursProjectStart")
			#pa.do("Play")
			break
		else:
			break

	for idx_env, envelope in enumerate(envelopes):
		if idx_env <= last_idx_env:
			continue
		time_envelope = time_start + float(envelope["block_idx"] * js["delta_offset"]) / float(js["samplerate"])
		if time_current > time_envelope:
			#print(f"idx_freq_group={envelope['idx_freq_group']}")

			connexion_serveur.send(json.dumps(envelope).encode("utf-8")+ b"|")

			last_idx_env = idx_env
		else:
			break
if pa_active:
	pa.do("Stop")

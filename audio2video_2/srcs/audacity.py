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

import pyaudacity as pa


#json_path = "../data/wav/bcl7/bcl7.json"
json_path = "../data/wav/record_beat_simple/record_beat_simple.json"


with open(json_path) as f:
	js = json.load(f)

envelopes = js["envelopes"]
envelopes.sort(key=lambda x : x["block_idx"])

connexion_serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
connexion_serveur.connect(("127.0.0.1", 8080))
connexion_serveur.send(json.dumps({"delta_offset" : js["delta_offset"], "samplerate" : js["samplerate"]}).encode("utf-8"))

pa.do("CursProjectStart")
# sleep nécessaire entre les pa.do sinon plante
time.sleep(3.0)

time_start = time.time()
pa.do("Play")
time_end = time_start + float(js["frames"])/ float(js["samplerate"])
#print(f"time_start = {time_start} ; time_end = {time_end}")

last_idx_env = -1
while True:
	time_current = time.time()
	if time_current > time_end:
		break

	for idx_env, envelope in enumerate(envelopes):
		if idx_env <= last_idx_env:
			continue
		time_envelope = time_start + float(envelope["block_idx"] * js["delta_offset"]) / float(js["samplerate"])
		if time_current > time_envelope:
			print(f"idx_freq_group={envelope['idx_freq_group']}")

			connexion_serveur.send(json.dumps(envelope).encode("utf-8")+ b"|")

			last_idx_env = idx_env
		else:
			break

pa.do("Stop")

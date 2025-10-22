#!/usr/bin/env python3

"""
Pour faire fonctionner cs script il faut 
l'installer dans un env conda
activer cet env
ouvrir Audacity préférences / Modules -> activé puis redémarrer Audacity
installer dans l'env pythonosc (https://pypi.org/project/python-osc/)

ref des actions de pyaudacity : https://manual.audacityteam.org/man/scripting_reference.html
"""

import os
import sys

import time
import json

import pyaudacity as pa
from pythonosc import udp_client
from pythonosc import osc_bundle_builder
from pythonosc import osc_message_builder


json_path = "../data/wav/bcl7/bcl7.json"


with open(json_path) as f:
	js = json.load(f)

envelopes = js["envelopes"]
envelopes.sort(key=lambda x : x["block_idx"])

pa.do("CursProjectStart")
# sleep nécessaire entre les pa.do sinon plante
time.sleep(0.1)

time_start = time.time()
#pa.do("Play")
time_end = time_start + float(js["frames"])/ float(js["samplerate"])
print(f"time_start = {time_start} ; time_end = {time_end}")

client = udp_client.SimpleUDPClient("127.0.0.1", 57111)


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
			#print(envelope["block_idx"])
			
			# message simple
			#client.send_message("/block_idx", envelope["block_idx"])
			
			# bundle
			msg = osc_message_builder.OscMessageBuilder(address="/env")
			msg.add_arg("idx_freq_group")
			msg.add_arg(envelope["idx_freq_group"])
			bundle = osc_bundle_builder.OscBundleBuilder(osc_bundle_builder.IMMEDIATELY)
			bundle.add_content(msg.build())
			bundle = bundle.build()
			client.send(bundle)

			last_idx_env = idx_env
		else:
			break

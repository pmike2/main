#!/usr/bin/env python3

import os
from pprint import pprint as pp
import json

N_MAX_FRAMES = 10

dir_script = os.path.dirname(__file__)
json_path_in = os.path.join(os.path.dirname(dir_script), "data", "test_ini.json")
json_path_out = os.path.join(os.path.dirname(dir_script), "data", "test.json")

with open(json_path_in) as f:
	d = json.load(f)

actions = d["actions"]

for action_name, l_frames in actions.items():
	l_frames_light = []
	step = int(len(l_frames) / N_MAX_FRAMES)
	if step == 0:
		step = 1
	print(step)
	compt = 0
	while compt < len(l_frames):
		l_frames_light.append(l_frames[compt])
		compt += step
	
	d["actions"][action_name] = l_frames_light

with open(json_path_out, 'w') as f:
	json.dump(d, f, indent=4)

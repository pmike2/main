#!/usr/bin/env python3

import os
import sys
from pprint import pprint as pp
import json


def prune(json_path_in, json_path_out, n_max_frames):
	with open(json_path_in) as f:
		d = json.load(f)

	actions = d["actions"]

	for action_name, l_frames in actions.items():
		l_frames_light = []
		step = int(len(l_frames) / n_max_frames)
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

json_path_in = sys.argv[1]
json_path_out = sys.argv[2]
n_max_frames = int(sys.argv[3])
prune(json_path_in, json_path_out, n_max_frames)

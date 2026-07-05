#!/usr/bin/env python3


from pprint import pprint as pp
import json

N_MAX_FRAMES = 10

json_path_in = "/Users/home/git_dir/main/test_obj_anim/data/test_ini.json"
json_path_out = "/Users/home/git_dir/main/test_obj_anim/data/test.json"

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
		#l_frames_light.append(l_frames[compt])
		keys = l_frames[compt].keys()
		d_frame_identity = {}
		for k in keys:
			d_frame_identity[k] = [
				1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0
			]
		l_frames_light.append(d_frame_identity)
		compt += step
	
	d["actions"][action_name] = l_frames_light

with open(json_path_out, 'w') as f:
	json.dump(d, f, indent=4)

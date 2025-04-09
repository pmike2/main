#!/usr/bin/env python3

import os
import json


root= "../data/cars"
jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
for js in jsons:
	with open(js) as f:
		data= json.load(f)
	
	#data["max_thrust"]= 3.5
	data["friction_threshold"]= 0.07
	data["backward_dynamic_friction"]= 0.9

	with open(js, "w") as f:
		json.dump(data, f, indent=4)

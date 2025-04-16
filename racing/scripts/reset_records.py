#!/usr/bin/env python3

# reset de tous les records de toutes les pistes

import os
import json


root= "../data/tracks"
jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
for js in jsons:
	with open(js) as f:
		data= json.load(f)
	
	data["best_lap"]    = [{"name" : "XXX", "time" : 1000.0}]* 3
	data["best_overall"]= [{"name" : "XXX", "time" : 1000.0}]* 3

	with open(js, "w") as f:
		json.dump(data, f, indent=4)

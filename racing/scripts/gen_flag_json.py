#!/usr/bin/env python3

import os
import json


json_path= "../data/static_objects/floating_objects/start.json"
root_pngs= "../data/static_objects/floating_objects/textures"
flag_pngs= sorted([os.path.join(root_pngs, x) for x in os.listdir(root_pngs) if x.startswith("start_")])

data= {
	"type" : "start",
	"actions" : {
		"active" : []
	}
}

for png in flag_pngs:
	data["actions"]["active"].append({
		"texture" : os.path.basename(png),
		"n_ms" : 40
	})

with open(json_path, "w") as f:
	json.dump(data, f, indent=4)

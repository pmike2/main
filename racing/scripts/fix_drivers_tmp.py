#!/usr/bin/env python3

# script applicable à tous les drivers

import os, json
from copy import deepcopy

def f1():
	pmb= "/Volumes/Data/perso/dev/main/racing/data/drivers/pmb.json"
	with open(pmb) as f:
		data_pmb= json.load(f)

	others= ("lauren", "minett", "mneit", "niek", "robin")

	for other in others:
		data= deepcopy(data_pmb)
		for key, val in data["actions"].items():
			for idx, d in enumerate(val):
				data["actions"][key][idx]["texture"]= d["texture"].replace("pmb", other)
		
		json_path= pmb.replace("pmb", other)
		with open(json_path, "w") as f:
			json.dump(data, f, indent=4)


root= "/Volumes/Data/perso/dev/main/racing/data/drivers"
jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
for js in jsons:
	with open(js) as f:
		data= json.load(f)
	data["expressions"]= deepcopy(data["actions"])
	del data["actions"]
	with open(js, "w") as f:
		json.dump(data, f, indent=4)

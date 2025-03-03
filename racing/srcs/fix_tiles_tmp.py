#!/usr/bin/env python3

import os
import sys
import json


def fix(json_path):
	with open(json_path) as f:
		data= json.load(f)

		data["restitution"]= 0.2

		with open(json_path, "w") as f:
			json.dump(data, f, indent=4)


def fix_all(root_jsons):
	jsons= [os.path.join(root_jsons, x) for x in os.listdir(root_jsons) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		fix(json_path)


if __name__== "__main__":
	root_jsons= sys.argv[1]
	fix_all(root_jsons)
	#fix(sys.argv[1])

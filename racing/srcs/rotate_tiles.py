#!/usr/bin/env python3

#
# INUTILISE, gardé pour mémoire
#


import os
import sys
import json


IDX2NAME= ("90", "180", "270")


def rotate_pt(pt):
	return (1- pt[1], pt[0])


def rotate_tile(json_path):
	with open(json_path) as f:
		data= json.load(f)
	
	for i in range(3):
		for idx_pt in range(len(data["footprint"])):
			data["footprint"][idx_pt]= rotate_pt(data["footprint"][idx_pt])
		
		json_path_out= os.path.splitext(json_path)[0]+ IDX2NAME[i]+ ".json"
		with open(json_path_out, "w") as f:
			json.dump(data, f, indent=4)


def rotate_all(root_jsons):
	jsons= [os.path.join(root_jsons, x) for x in os.listdir(root_jsons) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		#if os.path.basename(json_path) in ("empty.json", "full.json"):
		#	continue
		rotated= False
		for s in IDX2NAME:
			if s in os.path.basename(json_path):
				rotated= True
				break
		if rotated:
			continue
		
		rotate_tile(json_path)


if __name__== "__main__":
	#root_jsons= sys.argv[1]
	#rotate_all(root_jsons)
	pass

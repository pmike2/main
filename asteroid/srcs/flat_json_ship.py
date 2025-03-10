#!/usr/bin/env python3

import sys
from pprint import pprint as pp
import json


def flat_json(json_path_in, json_path_out):
	#print(f"flat_json : {json_path_in} -> {json_path_out}")

	with open(json_path_in) as f:
		data= json.load(f)

	#pp(data)
	#print("---------------------")

	# expand textures
	# while True:
	# 	modified= False
	# 	for action_name, l_actions in data["actions"].items():
	# 		if modified:
	# 			break
	# 		for idx_action, action in enumerate(l_actions):
	# 			if "name" in action.keys() and "texture" in action.keys():
	# 				l_actions_found= None
					
	# 				for action_name2, l_actions2 in data["actions"].items():
	# 					if action_name2== action["name"]:
	# 						l_actions_found= l_actions2
	# 						break
					
	# 				if l_actions_found is None:
	# 					raise RuntimeError("aaa")
					
	# 				print("---------")
	# 				print(action_name)
	# 				print(action["name"])
	# 				for idx_action2, action2 in enumerate(l_actions_found):
	# 					print(action2)
	# 					if "texture" not in action2.keys():
	# 						l_actions_found[idx_action2]["texture"]= action["texture"]
	# 						modified= True
	# 				break
	# 	if not modified:
	# 		break

	# ajout texture par défaut
	for action_name, l_actions in data["actions"].items():
		for idx_action, action in enumerate(l_actions):
			if "texture" not in action.keys():
				l_actions[idx_action]["texture"]= "main_tex"

	# expand paths
	while True:
		modified= False
		for action_name, l_actions in data["actions"].items():
			if modified:
				break
			for idx_action, action in enumerate(l_actions):
				if "name" in action.keys():
					n= action["n"] if "n" in action.keys() else 1
					l_actions_found= None
					
					for action_name2, l_actions2 in data["actions"].items():
						if action_name2== action["name"]:
							l_actions_found= l_actions2
							break
					
					if l_actions_found is None:
						raise RuntimeError("aaa")
					
					data["actions"][action_name]= l_actions[:idx_action]+ n* l_actions_found+ l_actions[idx_action+ 1:]
					modified= True
					break
		if not modified:
			break

	#pp(data)

	if "main" not in data["actions"].keys():
		raise RuntimeError("bbb")
	
	#data["actions"]= data["actions"]["main"]

	with open(json_path_out, "w") as f:
		json.dump(data, f, indent=4)


if __name__== "__main__":
	json_path_in, json_path_out= sys.argv[1], sys.argv[2]
	flat_json(json_path_in, json_path_out)

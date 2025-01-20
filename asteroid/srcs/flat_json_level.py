#!/usr/bin/env python3

import sys
from pprint import pprint as pp
import json
import copy


def flat_json(json_path_in, json_path_out):
	#print(f"flat_json : {json_path_in} -> {json_path_out}")

	with open(json_path_in) as f:
		data= json.load(f)

	#pp(data)
	#print("---------------------")

	while True:
		#pp(data)
		modified= False
		for event_name, l_events in data.items():
			#if event_name== "main":
			#	continue

			if modified:
				break
			
			for idx_event, event in enumerate(l_events):
				if "name" in event.keys():
					t= event["t"] if "t" in event.keys() else 0
					n= event["n"] if "n" in event.keys() else 1
					delta_t= event["delta_t"] if "delta_t" in event.keys() else 0
					delta_position_x= event["delta_position"][0] if "delta_position" in event.keys() else 0.0
					delta_position_y= event["delta_position"][1] if "delta_position" in event.keys() else 0.0

					l_events_found= None
					for event_name2, l_events2 in data.items():
						if event_name2== event["name"]:
							l_events_found= l_events2
							break
					
					if l_events_found is None:
						raise RuntimeError("aaa")
					
					l_events2insert= []
					for i in range(n):
						for event in l_events_found:
							moved_event= copy.deepcopy(event)
							moved_event["t"]= t+ moved_event["t"]+ i* delta_t
							moved_event["position"][0]+= i* delta_position_x
							moved_event["position"][1]+= i* delta_position_y
							l_events2insert.append(moved_event)
					
					data[event_name]= l_events[:idx_event]+ l_events2insert+ l_events[idx_event+ 1:]
					modified= True
					break
		if not modified:
			break

	#pp(data)

	if "main" not in data.keys():
		raise RuntimeError("bbb")

	data= data["main"]
	
	with open(json_path_out, "w") as f:
		json.dump(data, f, indent=4)


if __name__== "__main__":
	json_path_in, json_path_out= sys.argv[1], sys.argv[2]
	flat_json(json_path_in, json_path_out)

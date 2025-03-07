#!/usr/bin/env python3

import os
import json
from pprint import pprint as pp

EPS= 1e-12


def cross(p1, p2):
	return [p1[0]* p2[1]- p1[1]* p2[0]]


def is_left(pt_ref, dir_ref, pt_test):
	return cross(pt_test- pt_ref, dir_ref)<= 0.0


def rotate_pt(pt):
	return (-1.0* pt[1], pt[0])


def rot(footprint):
	return [rotate_pt(x) for x in footprint]


def xmirror(footprint):
	return [[-1.0* x[0], x[1]] for x in footprint]


def polygonize(footprint, dist):
	new_footprint= []
	for i in range(len(footprint)):
		p0= footprint[i]
		p1= footprint[(i+ 1)% len(footprint)]
		new_pt= []
		new_pt.append(0.5* (p0[0]+ p1[0])+ dist* (p1[1]- p0[1]))
		new_pt.append(0.5* (p0[1]+ p1[1])- dist* (p1[0]- p0[0]))
		new_footprint.append(p0)
		new_footprint.append(new_pt)
		#new_footprint.append(p1)
	
	return new_footprint


def crop(footprint):
	new_footprint= []
	for i in range(len(footprint)):
		pt= footprint[i]
		if pt[0]< -0.5- EPS or pt[0]> 0.5+ EPS or pt[1]< -0.5- EPS or pt[1]> 0.5+ EPS:
			continue
		new_footprint.append(pt)

	return new_footprint


def gen_json(json_path, pts):
	data= {
		"type": "obstacle_setting",
		"footprint": pts,
		"restitution": 0.2
	}
	with open(json_path, "w") as f:
		json.dump(data, f, indent=4)



def gen_all_jsons(root, n_subdiv):
	#os.system(f"rm {root}/*.json")
	jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		if os.path.basename(json_path) not in ("empty.json", "full.json"):
			os.remove(json_path)

	compt= 0

	pt2= (0.5, -0.5)
	for i in range(n_subdiv):
		pt1= (float(i)/ float(n_subdiv)- 0.5, -0.5)
		for j in range(n_subdiv):
			pt3= (0.5, float(j+ 1)/ float(n_subdiv)- 0.5)
			
			footprint= [pt1, pt2, pt3]
			json_path= os.path.join(root, f"{compt}.json")
			gen_json(json_path, footprint)
			compt+= 1

			for idx_angle in range(3):
				footprint= rot(footprint)
				json_path= os.path.join(root, f"{compt}.json")
				gen_json(json_path, footprint)
				compt+= 1
			
			#footprint= rot(footprint)
			#footprint= xmirror(footprint)

			#for idx_angle in range(3):
			#	footprint= rot(footprint)
			#	print(footprint)
			#	json_path= os.path.join(root, f"{compt}.json")
			#	gen_json(json_path, footprint)
			#	compt+= 1


def polygonize_all(root):
	jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		with open(json_path) as f:
			data= json.load(f)

		for i in range(3):
			data["footprint"]= polygonize(data["footprint"], 0.1/ (1.0+ float(i)))
			data["footprint"]= crop(data["footprint"])
		
		with open(json_path, "w") as f:
			json.dump(data, f, indent=4)


if __name__== "__main__":
	#footprint= [(0.0, 0.0), (1.0, 0.0), (0.0, 1.0)]
	#dist= 0.1
	#pp(polygonize(footprint, dist))

	root= "../data/static_objects/tiles"
	n_subdiv= 2
	gen_all_jsons(root, n_subdiv)
	
	polygonize_all(root)

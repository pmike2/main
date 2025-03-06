#!/usr/bin/env python3

import os
import json


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


def polygonize(footprint, n):
	eps= 1e-5
	d01= (footprint[0][0]- footprint[1][0])* (footprint[0][0]- footprint[1][0])+ (footprint[0][1]- footprint[1][1])* (footprint[0][1]- footprint[1][1])
	d02= (footprint[0][0]- footprint[2][0])* (footprint[0][0]- footprint[2][0])+ (footprint[0][1]- footprint[2][1])* (footprint[0][1]- footprint[2][1])
	d12= (footprint[1][0]- footprint[2][0])* (footprint[1][0]- footprint[2][0])+ (footprint[1][1]- footprint[2][1])* (footprint[1][1]- footprint[2][1])
	if d01> d02+ eps and d01> d12+ eps:
		pt0= footprint[0]
		pt1= footprint[1]
		pt_out= footprint[2]
	elif d02> d01+ eps and d02> d12+ eps:
		pt0= footprint[0]
		pt1= footprint[2]
		pt_out= footprint[1]
	else:
		pt0= footprint[1]
		pt1= footprint[2]
		pt_out= footprint[0]

	if is_left(pt0, pt1- pt0, pt_out):
		tmp= pt0
		pt0= pt1
		pt1= tmp

	



def gen_json(json_path, pts):
	data= {
		"type": "obstacle_setting",
		"footprint": pts,
		"restitution": 0.2
	}
	with open(json_path, "w") as f:
		json.dump(data, f, indent=4)



def gen_all_jsons(root, n_subdiv):
	os.system(f"rm {root}/*.json")

	compt= 0

	pt2= (0.5, -0.5)
	for i in range(n_subdiv):
		pt1= (float(i)/ float(n_subdiv)- 0.5, -0.5)
		for j in range(n_subdiv):
			pt3= (0.5, float(j+ 1)/ float(n_subdiv)- 0.5)
			
			footprint= [pt1, pt2, pt3]
			print(footprint)
			json_path= os.path.join(root, f"{compt}.json")
			gen_json(json_path, footprint)
			compt+= 1

			for idx_angle in range(3):
				footprint= rot(footprint)
				print(footprint)
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


if __name__== "__main__":
	root= "/Users/home/git_dir/main/racing/data/static_objects/tiles"
	n_subdiv= 2
	gen_all_jsons(root, n_subdiv)

#!/usr/bin/env python3

"""
Création des tuiles obstacle
"""

import os
import json
from pprint import pprint as pp


EPS= 1e-12
FULL_FOOTPRINT= [[-0.5, -0.5], [0.5, -0.5], [0.5, 0.5], [-0.5, 0.5]]


def rotate_pt(pt):
	"""rotation de pi/2 d'un pt"""
	return (-1.0* pt[1], pt[0])


def rot(footprint):
	"""rotation de pi/2 d'un polygone"""
	return [rotate_pt(x) for x in footprint]


def xmirror(footprint):
	"""symétrie axe x"""
	return [[-1.0* x[0], x[1]] for x in footprint]


def polygonize(footprint, dist):
	"""subdivision de chaque coté d'un polygone en 2 cotés"""
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
	"""restreint à l'emprise -0.5 0.5"""
	new_footprint= []
	for i in range(len(footprint)):
		pt= footprint[i]
		if pt[0]< -0.5- EPS or pt[0]> 0.5+ EPS or pt[1]< -0.5- EPS or pt[1]> 0.5+ EPS:
			continue
		new_footprint.append(pt)

	return new_footprint


def gen_json(json_path, type_, footprint, material):
	data= {"type" : type_, "footprint" : footprint, "material" : material}
	
	with open(json_path, "w") as f:
		json.dump(data, f, indent=4)


def gen_jsons(root, json_name, footprint):
	json_path= os.path.join(root, json_name+ "_obst.json")
	gen_json(json_path, "obstacle_tile", footprint, "wall")

	json_path= os.path.join(root, json_name+ "_sand.json")
	gen_json(json_path, "surface_tile", footprint, "sand")


def gen_all_jsons(root, n_subdiv):
	"""Création de tous les jsons"""
	os.system(f"rm {root}/*.json 2>/dev/null")
	#jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
	#for json_path in jsons:
	#	os.remove(json_path)

	gen_json(os.path.join(root, "empty.json"), "obstacle_tile", [], "wall")
	gen_jsons(root, "full", FULL_FOOTPRINT)

	# bandes verticales gauche
	for i in range(1, n_subdiv):
		pt0= (-0.5, -0.5)
		pt1= (-0.5+ float(i)/ float(n_subdiv), -0.5)
		pt2= (-0.5+ float(i)/ float(n_subdiv), 0.5)
		pt3= (-0.5, 0.5)
		footprint= [pt0, pt1, pt2, pt3]
		gen_jsons(root, f"band_vg_{i}", footprint)

	# bandes verticales droite
	for i in range(1, n_subdiv):
		pt0= (-0.5+ float(i)/ float(n_subdiv), -0.5)
		pt1= (0.5, -0.5)
		pt2= (0.5, 0.5)
		pt3= (-0.5+ float(i)/ float(n_subdiv), 0.5)
		footprint= [pt0, pt1, pt2, pt3]
		gen_jsons(root, f"band_vd_{i}", footprint)

	# bandes horizontales basse
	for i in range(1, n_subdiv):
		pt0= (-0.5, -0.5)
		pt1= (0.5, -0.5)
		pt2= (0.5, -0.5+ float(i)/ float(n_subdiv))
		pt3= (-0.5, -0.5+ float(i)/ float(n_subdiv))
		footprint= [pt0, pt1, pt2, pt3]
		gen_jsons(root, f"band_hb_{i}", footprint)

	# bandes horizontales haute
	for i in range(1, n_subdiv):
		pt0= (-0.5, -0.5+ float(i)/ float(n_subdiv))
		pt1= (0.5, -0.5+ float(i)/ float(n_subdiv))
		pt2= (0.5, 0.5)
		pt3= (-0.5, 0.5)
		footprint= [pt0, pt1, pt2, pt3]
		gen_jsons(root, f"band_hh_{i}", footprint)

	# triangles
	pt2= (0.5, -0.5)
	for i in range(n_subdiv):
		pt1= (-0.5+ float(i)/ float(n_subdiv), -0.5)
		for j in range(n_subdiv):
			pt3= (0.5, float(j+ 1)/ float(n_subdiv)- 0.5)
			
			footprint= [pt1, pt2, pt3]
			gen_jsons(root, f"tri_{i}_{j}", footprint)

			for idx_angle in range(3):
				footprint= rot(footprint)
				gen_jsons(root, f"tri_{i}_{j}_{idx_angle}", footprint)

	# complémentaires triangles
	for i in range(n_subdiv):
		pt1= (-0.5+ float(i)/ float(n_subdiv), -0.5)
		for j in range(n_subdiv):
			pt2= (0.5, float(j+ 1)/ float(n_subdiv)- 0.5)
			if i== 0:
				if j== n_subdiv- 1:
					#pt3= (-0.5, 0.5)
					#footprint= [pt1, pt2, pt3]
					continue
				else:
					pt3= (0.5, 0.5)
					pt4= (-0.5, 0.5)
					footprint= [pt1, pt2, pt3, pt4]
			else:
				if j== n_subdiv- 1:
					pt3= (-0.5, 0.5)
					pt4= (-0.5, -0.5)
					footprint= [pt1, pt2, pt3, pt4]
				else:
					pt3= (0.5, 0.5)
					pt4= (-0.5, 0.5)
					pt5= (-0.5, -0.5)
					footprint= [pt1, pt2, pt3, pt4, pt5]

			gen_jsons(root, f"tri_comp_{i}_{j}", footprint)

			for idx_angle in range(3):
				footprint= rot(footprint)
				gen_jsons(root, f"tri_comp_{i}_{j}_{idx_angle}", footprint)


def polygonize_all(root):
	"""Ajout de jsons à partir des jsons d'un dossier en subdivisant tous les cotés"""
	jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		if os.path.basename(json_path) in ("empty.json", "full_obst.json", "full_sand.json"):
			continue
		
		if "sand" in os.path.basename(json_path):
			continue
		
		with open(json_path) as f:
			data= json.load(f)

		for i in range(3):
			data["footprint"]= polygonize(data["footprint"], 0.1/ (1.0+ float(i)))
			data["footprint"]= crop(data["footprint"])
		
		new_json_path= os.path.splitext(json_path)[0]+ "_curve.json"
		with open(new_json_path, "w") as f:
			json.dump(data, f, indent=4)


if __name__== "__main__":
	#footprint= [(0.0, 0.0), (1.0, 0.0), (0.0, 1.0)]
	#dist= 0.1
	#pp(polygonize(footprint, dist))

	root= "../data/static_objects/tiles"
	n_subdiv= 2
	gen_all_jsons(root, n_subdiv)
	
	polygonize_all(root)

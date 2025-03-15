#!/usr/bin/env python3

"""
Création des tuiles obstacle
"""

import os
import json
from pprint import pprint as pp


EPS= 1e-12
TILE_RESTITUTION= 0.02


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


def gen_json(json_path, pts):
	"""création d'un json à partir d'un polygone"""
	data= {
		"type": "obstacle_setting",
		"footprint": pts,
		"restitution": TILE_RESTITUTION
	}
	with open(json_path, "w") as f:
		json.dump(data, f, indent=4)



def gen_all_jsons(root, n_subdiv):
	"""Création de tous les jsons"""
	os.system(f"rm {root}/*.json")
	#jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
	#for json_path in jsons:
	#	os.remove(json_path)

	gen_json(os.path.join(root, "empty.json"), [])
	gen_json(os.path.join(root, "full.json"), [[-0.5, -0.5], [0.5, -0.5], [0.5, 0.5], [-0.5, 0.5]])

	compt= 0

	# bandes verticales gauche
	for i in range(1, n_subdiv):
		pt0= (-0.5, -0.5)
		pt1= (-0.5+ float(i)/ float(n_subdiv), -0.5)
		pt2= (-0.5+ float(i)/ float(n_subdiv), 0.5)
		pt3= (-0.5, 0.5)
		footprint= [pt0, pt1, pt2, pt3]
		json_path= os.path.join(root, f"{compt:03d}.json")
		gen_json(json_path, footprint)
		compt+= 1

	# bandes verticales droite
	for i in range(1, n_subdiv):
		pt0= (-0.5+ float(i)/ float(n_subdiv), -0.5)
		pt1= (0.5, -0.5)
		pt2= (0.5, 0.5)
		pt3= (-0.5+ float(i)/ float(n_subdiv), 0.5)
		footprint= [pt0, pt1, pt2, pt3]
		json_path= os.path.join(root, f"{compt:03d}.json")
		gen_json(json_path, footprint)
		compt+= 1

	# bandes horizontales basse
	for i in range(1, n_subdiv):
		pt0= (-0.5, -0.5)
		pt1= (0.5, -0.5)
		pt2= (0.5, -0.5+ float(i)/ float(n_subdiv))
		pt3= (-0.5, -0.5+ float(i)/ float(n_subdiv))
		footprint= [pt0, pt1, pt2, pt3]
		json_path= os.path.join(root, f"{compt:03d}.json")
		gen_json(json_path, footprint)
		compt+= 1

	# bandes horizontales haute
	for i in range(1, n_subdiv):
		pt0= (-0.5, -0.5+ float(i)/ float(n_subdiv))
		pt1= (0.5, -0.5+ float(i)/ float(n_subdiv))
		pt2= (0.5, 0.5)
		pt3= (-0.5, 0.5)
		footprint= [pt0, pt1, pt2, pt3]
		json_path= os.path.join(root, f"{compt:03d}.json")
		gen_json(json_path, footprint)
		compt+= 1

	# triangles
	pt2= (0.5, -0.5)
	for i in range(n_subdiv):
		pt1= (-0.5+ float(i)/ float(n_subdiv), -0.5)
		for j in range(n_subdiv):
			pt3= (0.5, float(j+ 1)/ float(n_subdiv)- 0.5)
			
			footprint= [pt1, pt2, pt3]
			json_path= os.path.join(root, f"{compt:03d}.json")
			gen_json(json_path, footprint)
			compt+= 1

			for idx_angle in range(3):
				footprint= rot(footprint)
				json_path= os.path.join(root, f"{compt:03d}.json")
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

			json_path= os.path.join(root, f"{compt:03d}.json")
			gen_json(json_path, footprint)
			compt+= 1

			for idx_angle in range(3):
				footprint= rot(footprint)
				json_path= os.path.join(root, f"{compt:03d}.json")
				gen_json(json_path, footprint)
				compt+= 1


def polygonize_all(root):
	"""Ajout de jsons à partir des jsons d'un dossier en subdivisant tous les cotés"""
	jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		if os.path.basename(json_path) in ("empty.json", "full.json"):
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

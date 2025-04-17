#!/usr/bin/env python3

# script applicable à toutes les courses

import os
import json


root= "../data/tracks"
jsons= [os.path.join(root, x) for x in os.listdir(root) if os.path.splitext(x)[1]== ".json"]
for js in jsons:
	with open(js) as f:
		data= json.load(f)
	
	width_ini, height_ini= data["width"], data["height"]
	tiles_ini= data["tiles"][:]
	"band_hb_1_sand" # haut
	"band_hh_1_sand" # bas
	"band_vg_1_sand" # droite
	"band_vd_1_sand" # gauche
	"tri_0_1_0_sand" # bas-gauche
	"tri_0_1_1_sand" # bas-droite
	"tri_0_1_2_sand" # haut-droite
	"tri_0_1_sand" # haut-gauche

	data["tiles"]= []
	
	data["tiles"].append("tri_0_1_0_sand")
	for i in range(width_ini):
		data["tiles"].append("band_hh_1_sand")
	data["tiles"].append("tri_0_1_1_sand")

	compt= 0
	for col_idx in range(height_ini):
		data["tiles"].append("band_vd_1_sand")
		for row_idx in range(width_ini):
			data["tiles"].append(tiles_ini[compt])
			compt+= 1
		data["tiles"].append("band_vg_1_sand")
	
	data["tiles"].append("tri_0_1_sand")
	for i in range(width_ini):
		data["tiles"].append("band_hb_1_sand")
	data["tiles"].append("tri_0_1_2_sand")

	data["width"]= width_ini+ 2
	data["height"]= height_ini+ 2

	with open(js, "w") as f:
		json.dump(data, f, indent=4)

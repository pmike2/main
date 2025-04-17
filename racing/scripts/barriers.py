#!/usr/bin/env python3

#
# renseignement balise barriers des modèles
#

import os
import json

import shapely
from shapely import Polygon, MultiPolygon


# lecture des footprints
root_tiles= "../data/static_objects/tiles"
jsons= [os.path.join(root_tiles, x) for x in os.listdir(root_tiles) if os.path.splitext(x)[1]== ".json"]
tile2poly= {}
for js in jsons:
	with open(js) as f:
		data= json.load(f)
	
	if data["type"]!= "obstacle_tile":
		continue

	footprint= data["footprint"]
	if len(footprint)== 0:
		continue
	
	footprint.append(footprint[0])

	tile2poly[os.path.basename(os.path.splitext(js)[0])]= footprint


# pour chaque piste on fait l'union des footprints des tuiles
root_tracks= "../data/tracks"
tracks= [os.path.join(root_tracks, x) for x in os.listdir(root_tracks) if os.path.splitext(x)[1]== ".json"]
for track_path in tracks:
	with open(track_path) as f:
		data= json.load(f)
	tiles= data["tiles"]
	cell_size= data["cell_size"]
	width, height= data["width"], data["height"]

	union= MultiPolygon()
	for idx, tile in enumerate(tiles):
		if tile not in tile2poly.keys():
			continue
		col, row= idx % width, idx // width
		poly= tile2poly[tile]
		poly_sized= []
		for pt in poly:
			poly_sized.append([cell_size* (float(col)+ pt[0]+ 0.5), cell_size* (float(row)+ pt[1]+ 0.5)])
		#union= shapely.union(union, shapely.buffer(Polygon(poly_sized), 0.001))
		union= shapely.union(union, Polygon(poly_sized))

	#print(len(union.geoms))
	#print(union.wkt)

	data["barriers"]= []
	
	# on ne veut pas mettre de barrière sur l'extérieur du plus grand polygone
	if union.geom_type== "MultiPolygon":
		idx_biggest_poly, max_area= -1, -1.0
		for idx_poly, poly in enumerate(union.geoms):
			area= poly.exterior.area
			if area> max_area:
				max_area= area
				idx_biggest_poly= idx_poly
		
		for idx_poly, poly in enumerate(union.geoms):
			if idx_poly!= idx_biggest_poly:
				data["barriers"].append(list(poly.exterior.coords))
			for interior in poly.interiors:
				data["barriers"].append(list(interior.coords))
	
	elif union.geom_type== "Polygon":
		#data["barriers"].append(list(union.exterior.coords))
		for interior in union.interiors:
			data["barriers"].append(list(interior.coords))
	else:
		print(f"track {track_path} : type {union.geom_type} non supporté")

	with open(track_path, "w") as f:
		json.dump(data, f, indent=4)

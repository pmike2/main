#!/usr/bin/env python3

import os
import json

import shapely
from shapely import Polygon, MultiPolygon


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
	if union.geom_type== "MultiPolygon":
		for poly in union.geoms:
			data["barriers"].append(list(poly.exterior.coords))
			for interior in poly.interiors:
				data["barriers"].append(list(interior.coords))
	elif union.geom_type== "Polygon":
		data["barriers"].append(list(union.exterior.coords))
		for interior in union.interiors:
			data["barriers"].append(list(interior.coords))
	else:
		print(f"track {track_path} : type {union.geom_type} non supporté")

	with open(track_path, "w") as f:
		json.dump(data, f, indent=4)

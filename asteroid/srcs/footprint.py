#!/usr/bin/env python3

import sys, os
import json

from PIL import Image


def footprint(png_path):
	if not os.path.isfile(png_path):
		raise RuntimeError(f"{png_path} n'existe pas.")

	with Image.open(png_path) as im:
		pixels = list(im.getdata())
		width, height = im.size
	
	pixels_by_rows = [pixels[i * width:(i + 1) * width] for i in range(height)]
	pixels_by_cols= list(zip(*pixels_by_rows))
	# len(pixels_by_rows) == height len(pixels_by_rows[0]) == width
	# len(pixels_by_cols) == width len(pixels_by_cols[0]) == height

	# min_col == plus petit idx de colonne non nulle ; max_col == plus gd idx de colonne non nulle
	# min_row == plus petit idx de ligne non nulle ; max_row == plus gd idx de ligne non nulle
	# attention le 0 du y est en haut-gauche
	min_col= 1e10
	max_col= -1e10
	for row in pixels_by_rows:
		for idx_col, pix in enumerate(row):
			r, g, b, a= pix
			if r!= 0 or g!= 0 or b!= 0:
				if idx_col< min_col:
					min_col= idx_col
				if idx_col> max_col:
					max_col= idx_col

	min_row= 1e10
	max_row= -1e10
	for col in pixels_by_cols:
		for idx_row, pix in enumerate(col):
			r, g, b, a= pix
			if r!= 0 or g!= 0 or b!= 0:
				if idx_row< min_row:
					min_row= idx_row
				if idx_row> max_row:
					max_row= idx_row


	#print(f"min_col={min_col} ; max_col={max_col} ; min_row={min_row} ; max_row={max_row}")

	xmin= float(min_col)/ float(width)
	ymin= float(height- max_row)/ float(height)
	xmax= float(max_col)/ float(width)
	ymax= float(height- min_row)/ float(height)

	#return {"xmin" : xmin, "ymin" : ymin, "xmax" : xmax, "ymax" : ymax}
	return {"pos" : [xmin, ymin], "size" : [xmax- xmin, ymax- ymin]}


def modif_json(json_path):
	with open(json_path) as f:
		data= json.load(f)
	for tex_name, l_img in data["textures"].items():
		for img in l_img:
			png_path= os.path.join(os.path.dirname(json_path), os.path.basename(os.path.splitext(json_path)[0]), img["png"])
			img["footprint"]= footprint(png_path)
	with open(json_path, "w") as f:
		json.dump(data, f, indent=4)


def modif_all(root):
	for d in ["bullets", "enemies", "heroes"]:
		l_jsons= [os.path.join(root, d, x) for x in os.listdir(os.path.join(root, d)) if os.path.splitext(x)[1]== ".json"]
		for json_path in l_jsons:
			modif_json(json_path)


if __name__== "__main__":
	if os.path.isdir(sys.argv[1]):
		modif_all(sys.argv[1])
	else:
		modif_json(sys.argv[1])

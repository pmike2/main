#!/usr/bin/env python3

"""
Passage footprint des tuiles obstacle -> textures
"""

import os
import json
from PIL import Image, ImageDraw

EPS= 1e-10
SIZE= 1024


def coord2pixel(x, y):
	"""coords -0.5 0.5 -> pixels texture"""
	return [int((x+ 0.5)* float(SIZE- 1)), SIZE- 1- int((y+ 0.5)* float(SIZE- 1))]


# utile pour passer des coords dans le png (sous gimp) vers le footprint
def pixel2coord(i, j):
	"""pixels texture -> coords -0.5 0.5"""
	return [float(i)/ (float(SIZE- 1))- 0.5, float(SIZE- 1- j)/ float(SIZE- 1)- 0.5]


def pt_on_border(pt):
	"""le pixel est-t'il au bord"""
	if pt[0] in (0, SIZE- 1) or pt[1] in (0, SIZE- 1):
		return True
	return False


def pts_on_same_bord(pt1, pt2):
	if not pt_on_border(pt1) or not pt_on_border(pt2):
		return False
	if pt1[0] in (0, SIZE- 1) and pt1[0]== pt2[0]:
		return True
	if pt1[1] in (0, SIZE- 1) and pt1[1]== pt2[1]:
		return True
	return False


def fill_texture(tex_in_fill, tex_in_empty, tex_in_brick, border_width, footprint, result):
	"""Création texture.
	
	tex_in_fill = chemin texture de plein
	tex_in_empty = chemin texture de vide
	tex_in_brick = chemin texture de délimitation plein / vide
	footprint = emprise polygonale du plein
	result = png résultat
	"""
	fgr_img= Image.new('RGBA', (SIZE, SIZE), color=(0, 0, 0, 0))
	
	mask_img_empty = Image.new('L', (SIZE, SIZE), color=255)
	mask_img_empty_draw = ImageDraw.Draw(mask_img_empty)
	mask_img_empty_draw.polygon([0, 0, SIZE- 1, 0, SIZE- 1, SIZE- 1, 0, SIZE- 1, 0, 0], fill=0)

	if len(footprint)> 0:
		mask_img_fill = Image.new('L', (SIZE, SIZE), color=255)
		mask_img_fill_draw = ImageDraw.Draw(mask_img_fill)
		mask_img_fill_draw.polygon(footprint, fill=0)

		border_footprint= []
		last_pt= None
		for i in range(len(footprint)// 2):
			p0= [footprint[2* i], footprint[2* i+ 1]]
			if i== len(footprint)// 2- 1:
				p1= [footprint[0], footprint[1]]
			else:
				p1= [footprint[2* (i+ 1)], footprint[2* (i+ 1)+ 1]]
			if not pts_on_same_bord(p0, p1):
				border_footprint.append(p0)
				last_pt= p1

		if last_pt is not None:
			border_footprint.append(last_pt)
		border_footprint= [x for y in border_footprint for x in y]
		
		mask_img_brick= None
		if border_width> 0 and len(border_footprint)> 0:
			mask_img_brick = Image.new('L', (SIZE, SIZE), color=255)
			mask_img_brick_draw = ImageDraw.Draw(mask_img_brick)
			# joint="curve" permet d'avoir de l'arrondi
			mask_img_brick_draw.line(border_footprint, fill=0, width=border_width)

	with Image.open(tex_in_fill) as tex_img_fill:
		with Image.open(tex_in_empty) as tex_img_empty:
			with Image.open(tex_in_brick) as tex_img_brick:
				result_img= Image.composite(fgr_img, tex_img_empty, mask_img_empty)
				if len(footprint)> 0:
					result_img= Image.composite(result_img, tex_img_fill, mask_img_fill)
					if mask_img_brick is not None:
						result_img= Image.composite(result_img, tex_img_brick, mask_img_brick)
				result_img.save(result)


def fill_texture_json(tex_in_fill, tex_in_empty, tex_in_brick, tex_in_sand, border_width, json_path):
	"""Création texture à partir d'un json d'une tuile obstacle"""
	with open(json_path) as f:
		data= json.load(f)
	
	result= os.path.join(os.path.dirname(json_path), "textures", os.path.basename(os.path.splitext(json_path)[0])+ ".png")

	if data["type"]== "obstacle_tile":
		footprint= data["footprint"]
		footprint= [coord2pixel(x[0], x[1]) for x in footprint]
		footprint= [x for y in footprint for x in y]
		fill_texture(tex_in_fill, tex_in_empty, tex_in_brick, border_width, footprint, result)
	
	elif data["type"]== "surface_tile":
		footprint= data["footprint"]
		footprint= [coord2pixel(x[0], x[1]) for x in footprint]
		footprint= [x for y in footprint for x in y]
		fill_texture(tex_in_sand, tex_in_empty, tex_in_brick, 0, footprint, result)


def fill_dir(tex_in_fill, tex_in_empty, tex_in_brick, tex_in_sand, border_width, dir_path):
	"""Lancement sur un dossier de jsons"""
	os.system(f"rm {dir_path}/textures/*.png 2>/dev/null")
	#pngs= [os.path.join(dir_path, x) for x in os.listdir(dir_path) if os.path.splitext(x)[1]== ".png"]
	#for png in pngs:
	#	if os.path.basename(png) not in ("empty.png", "full.png"):
	#		os.remove(png)
	
	jsons= [os.path.join(dir_path, x) for x in os.listdir(dir_path) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		print(json_path)
		fill_texture_json(tex_in_fill, tex_in_empty, tex_in_brick, tex_in_sand, border_width, json_path)


if __name__== "__main__":
	tex_in_fill= "../data/tests/textures/full.png"
	tex_in_empty= "../data/tests/textures/empty.png"
	tex_in_brick= "../data/tests/textures/brick.png"
	tex_in_sand= "../data/tests/textures/sand.png"
	border_width= 50

	#json_path= "../data/static_objects/tiles/empty.json"
	#fill_texture_json(tex_in_fill, tex_in_empty, tex_in_brick, tex_in_sand, border_width, json_path)

	dir_path= "../data/static_objects/tiles"
	fill_dir(tex_in_fill, tex_in_empty, tex_in_brick, tex_in_sand, border_width, dir_path)

#!/usr/bin/env python3

import os
import json
from PIL import Image, ImageDraw

EPS= 1e-10
SIZE= 1024


def coord2pixel(x, y):
	return [int((x+ 0.5)* float(SIZE- 1)), SIZE- 1- int((y+ 0.5)* float(SIZE- 1))]


# utile pour passer des coords dans le png (sous gimp) vers le footprint
def pixel2coord(i, j):
	return [float(i)/ (float(SIZE- 1))- 0.5, float(SIZE- 1- j)/ float(SIZE- 1)- 0.5]


def pt_on_border(pt):
	if pt[0] in (0, SIZE- 1) or pt[1] in (0, SIZE- 1):
		return True
	return False


def fill_texture(tex_in_fill, tex_in_empty, tex_in_brick, footprint, result):
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
			if not pt_on_border(p0) or not pt_on_border(p1):
				border_footprint.append(p0)
				last_pt= p1
		if last_pt is not None:
			border_footprint.append(last_pt)
		border_footprint= [x for y in border_footprint for x in y]
		
		if len(border_footprint)> 0:
			mask_img_brick = Image.new('L', (SIZE, SIZE), color=255)
			mask_img_brick_draw = ImageDraw.Draw(mask_img_brick)
			mask_img_brick_draw.line(border_footprint, fill=0, width=20, joint="curve")

	with Image.open(tex_in_fill) as tex_img_fill:
		with Image.open(tex_in_empty) as tex_img_empty:
			with Image.open(tex_in_brick) as tex_img_brick:
				result_img= Image.composite(fgr_img, tex_img_empty, mask_img_empty)
				if len(footprint)> 0:
					result_img= Image.composite(result_img, tex_img_fill, mask_img_fill)
					if len(border_footprint)> 0:
						result_img= Image.composite(result_img, tex_img_brick, mask_img_brick)
				result_img.save(result)


def fill_texture_json(tex_in_fill, tex_in_empty, tex_in_brick, json_path):
	with open(json_path) as f:
		data= json.load(f)
	#print(data)
	footprint= data["footprint"]
	footprint= [coord2pixel(x[0], x[1]) for x in footprint]
	footprint= [x for y in footprint for x in y]
	#print(footprint)
	result= os.path.join(os.path.dirname(json_path), "textures", os.path.basename(os.path.splitext(json_path)[0])+ ".png")
	#print(result)
	fill_texture(tex_in_fill, tex_in_empty, tex_in_brick, footprint, result)


def fill_dir(tex_in_fill, tex_in_empty, tex_in_brick, dir_path):
	#os.system(f"rm {dir_path}/textures/*.png")
	pngs= [os.path.join(dir_path, x) for x in os.listdir(dir_path) if os.path.splitext(x)[1]== ".png"]
	for png in pngs:
		if os.path.basename(png) not in ("empty.png", "full.png"):
			os.remove(png)
	
	jsons= [os.path.join(dir_path, x) for x in os.listdir(dir_path) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		print(json_path)
		fill_texture_json(tex_in_fill, tex_in_empty, tex_in_brick, json_path)


if __name__== "__main__":
	#print(coord2pixel(-0.5, -0.5))
	#print(coord2pixel(0.5, 0.5))

	tex_in_fill= "../data/tests/textures/full.png"
	tex_in_empty= "../data/tests/textures/empty.png"
	tex_in_brick= "../data/tests/textures/brick.png"
	#footprint= [0, 0, 100, 0, 100, 100, 0, 0]
	#result= "../data/static_objects/test.png"
	#fill_texture(tex_in_fill, tex_in_empty, footprint, result)

	#json_path= "/Users/home/git_dir/main/racing/data/static_objects/tiles/lower_right_tri.json"
	#fill_texture_json(tex_in_fill, tex_in_empty, json_path)

	dir_path= "../data/static_objects/tiles"
	fill_dir(tex_in_fill, tex_in_empty, tex_in_brick, dir_path)

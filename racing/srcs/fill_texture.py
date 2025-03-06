#!/usr/bin/env python3

import os
import json
from PIL import Image, ImageDraw


SIZE= 1024


def coord2pixel(x, y):
	return [int((x+ 0.5)* float(SIZE)), SIZE- int((y+ 0.5)* float(SIZE))]


def fill_texture(tex_in_fill, tex_in_empty, footprint, result):
	fgr_img= Image.new('RGBA', (SIZE, SIZE), color=(0, 0, 0, 0))
	
	mask_img_empty = Image.new('L', (SIZE, SIZE), color=255)
	mask_img_empty_draw = ImageDraw.Draw(mask_img_empty)
	mask_img_empty_draw.polygon([0, 0, SIZE- 1, 0, SIZE- 1, SIZE- 1, 0, SIZE- 1, 0, 0], fill=0)

	if len(footprint)> 0:
		mask_img_fill = Image.new('L', (SIZE, SIZE), color=255)
		mask_img_fill_draw = ImageDraw.Draw(mask_img_fill)
		mask_img_fill_draw.polygon(footprint, fill=0)

	with Image.open(tex_in_fill) as tex_img_fill:
		with Image.open(tex_in_empty) as tex_img_empty:
			result_img= Image.composite(fgr_img, tex_img_empty, mask_img_empty)
			if len(footprint)> 0:
				result_img= Image.composite(result_img, tex_img_fill, mask_img_fill)
			result_img.save(result)


def fill_texture_json(tex_in_fill, tex_in_empty, json_path):
	with open(json_path) as f:
		data= json.load(f)
	#print(data)
	footprint= data["footprint"]
	footprint= [coord2pixel(x[0], x[1]) for x in footprint]
	footprint= [x for y in footprint for x in y]
	#print(footprint)
	result= os.path.join(os.path.dirname(json_path), "textures", os.path.basename(os.path.splitext(json_path)[0])+ ".png")
	#print(result)
	fill_texture(tex_in_fill, tex_in_empty, footprint, result)


def fill_dir(tex_in_fill, tex_in_empty, dir_path):
	os.system(f"rm {dir_path}/textures/*.png")
	jsons= [os.path.join(dir_path, x) for x in os.listdir(dir_path) if os.path.splitext(x)[1]== ".json"]
	for json_path in jsons:
		print(json_path)
		fill_texture_json(tex_in_fill, tex_in_empty, json_path)


if __name__== "__main__":
	tex_in_fill= "../data/tests/textures/full.png"
	tex_in_empty= "../data/tests/textures/empty.png"
	#footprint= [0, 0, 100, 0, 100, 100, 0, 0]
	#result= "../data/static_objects/test.png"
	#fill_texture(tex_in_fill, tex_in_empty, footprint, result)

	#json_path= "/Users/home/git_dir/main/racing/data/static_objects/tiles/lower_right_tri.json"
	#fill_texture_json(tex_in_fill, tex_in_empty, json_path)

	dir_path= "../data/static_objects/tiles"
	fill_dir(tex_in_fill, tex_in_empty, dir_path)

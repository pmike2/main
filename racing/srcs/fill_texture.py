#!/usr/bin/env python3

import os
import json
from PIL import Image, ImageDraw

SIZE= 1024


def coord2pixel(coord):
	

def opposite_footprint(footprint):


def fill_texture(tex_in_fill, tex_in_empty, footprint, result):
	fgr_img= Image.new('RGBA', (SIZE, SIZE), color=(0, 0, 0, 0))
	mask_img = Image.new('L', (SIZE, SIZE), color=255)
	mask_img_draw = ImageDraw.Draw(mask_img)
	mask_img_draw.polygon(footprint, fill=0)

	with Image.open(tex_in_fill) as tex_img:
		result_img= Image.composite(fgr_img, tex_img, mask_img)
		result_img.save(result)


def fill_texture_json(tex_in, json_path):
	with open(json_path) as f:
		data= json.load(f)
	footprint= data["footprint"]
	footprint= [x for y in footprint for x in y]
	print(footprint)
	result= os.path.join(os.path.dirname(json_path), "textures", os.path.basename(os.path.splitext(footprint)[0])+ ".png")
	print(result)
	fill_texture(tex_in, footprint, result)


if __name__== "__main__":
	#tex_in= "/Volumes/Data/perso/dev/main/racing/data/static_objects/tiles/textures/empty.png"
	#footprint= [0, 0, 100, 0, 100, 100, 0, 0]
	#result= "/Volumes/Data/perso/dev/main/racing/data/static_objects/test.png"
	#fill_texture(tex_in, footprint, result)

	tex_in= "/Volumes/Data/perso/dev/main/racing/data/static_objects/tiles/textures/empty.png"
	json_path= ""
	fill_texture_json(tex_in, json_path)
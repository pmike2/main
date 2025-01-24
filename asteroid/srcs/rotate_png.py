#!/usr/bin/env python3

import os, sys

from PIL import Image


def rotate(png_path, angle=180):
	with Image.open(png_path) as im:
		im.rotate(angle).save(os.path.splitext(png_path)[0]+ "_rotate.png", "PNG")


def rotate_dir(d, angle=180):
	l_pngs= [os.path.join(d, x) for x in os.listdir(d) if os.path.splitext(x)[1]== ".png"]
	for png_path in l_pngs:
		rotate(png_path, angle)


if __name__== "__main__":
	if os.path.isfile(sys.argv[1]):
		rotate(sys.argv[1])
	else:
		rotate_dir(sys.argv[1])

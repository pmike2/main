#!/usr/bin/env python3

import sys

from PIL import Image

im= Image.open(sys.argv[1])
pixels = list(im.getdata())
width, height = im.size
pixels_by_rows = [pixels[i * width:(i + 1) * width] for i in range(height)]
# len(pixels_by_rows) == height
# len(pixels_by_rows[0]) == width

# min_col == plus petit idx de colonne non nulle ; max_col == plus gd idx de colonne non nulle
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

print(min_col)
print(max_col)

pixels_by_cols = [pixels[i * height:(i + 1) * height] for i in range(width)]
# len(pixels_by_cols) == width
# len(pixels_by_cols[0]) == height

# min_col == plus petit idx de colonne non nulle ; max_col == plus gd idx de colonne non nulle
min_row= 1e10
max_row= -1e10
for col in pixels_by_cols:
	for idx_row, pix in enumerate(col):
		r, g, b, a= pix
		if r!= 0 or g!= 0 or b!= 0:
			if idx_row< min_row:
				print(f"{min_row} -> {idx_row}")
				min_row= idx_row
			if idx_row> max_row:
				max_row= idx_row

print(min_row)
print(max_row)

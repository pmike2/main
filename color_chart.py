#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys, re

import colorsys, math
from PIL import Image

os.system("rm color_chart* 2>/dev/null ")

NH= 50
NS= 6
NV= 5
WIDTH= 600
HEIGHT= int(WIDTH* 297./ 210)
EPS_W= 20
EPS_H= 30

for idx_h in range(NH):
	image = Image.new("RGB", (WIDTH, HEIGHT), (255, 255, 255))
	for idx_s in range(NS):
		for idx_v in range(NV):
			h= float(idx_h)/ NH
			#s= float(idx_s)/ (NS- 1)
			#v= float(idx_v)/ (NV- 1)
			s= float(idx_s+ 1)/ (NS- 1)
			v= float(idx_v+ 1)/ (NV- 1)
			r, g, b= map(lambda x : int(math.floor(255* x)), colorsys.hsv_to_rgb(h, s, v))
			#print h, s, v, r, g, b

			xmin= (idx_s+ 1)* EPS_W+ idx_s* (WIDTH- EPS_W* (NS+ 1))/ NS
			xmax= xmin+ (WIDTH- EPS_W* (NS+ 1))/ NS
			ymin= (idx_v+ 1)* EPS_H+ idx_v* (HEIGHT- EPS_H* (NV+ 1))/ NV
			ymax= ymin+ (HEIGHT- EPS_H* (NV+ 1))/ NV

			image.paste((r, g, b), [xmin, ymin, xmax, ymax])

	image.save("color_chart_%02d.jpg" % (idx_h+ 1), "JPEG")
	del image
	
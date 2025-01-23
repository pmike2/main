#!/usr/bin/env python3

"""Copie des pngs de debug"""

import os

# le mode doit correspondre à TEXTURE_SIZE défini dans asteroid.h
#MODE= "512"
MODE= "1024"

data_dir= os.path.join(os.path.dirname(os.path.dirname(__file__)), "data")
debug_dir= os.path.join(data_dir, "debug")
for d in ("bullets", "enemies", "heroes"):
	l_dirs= [os.path.join(data_dir, d, x) for x in os.listdir(os.path.join(data_dir, d)) if os.path.isdir(os.path.join(data_dir, d, x))]
	for dd in l_dirs:
		for f in ("main1", "main2", "shoot1", "shoot2"):
			src= os.path.join(debug_dir, f"{f}_{MODE}.png")
			dst= os.path.join(dd, f"{f}.png")
			if os.path.isfile(dst):
				cmd= f"cp {src} {dst}"
				#print(cmd)
				os.system(f"cp {src} {dst}")

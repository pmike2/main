#!/usr/bin/env python3

import os

data_dir= os.path.join(os.path.dirname(os.path.dirname(__file__)), "data")
debug_dir= os.path.join(data_dir, "debug")
for d in ("bullets", "enemies", "heroes"):
	l_dirs= [os.path.join(data_dir, d, x) for x in os.listdir(os.path.join(data_dir, d)) if os.path.isdir(os.path.join(data_dir, d, x))]
	for dd in l_dirs:
		for f in ("main1.png", "main2.png", "shoot1.png", "shoot2.png"):
			src= os.path.join(debug_dir, f)
			dst= os.path.join(dd, f)
			if os.path.isfile(dst):
				cmd= f"cp {src} {dst}"
				#print(cmd)
				os.system(f"cp {src} {dst}")

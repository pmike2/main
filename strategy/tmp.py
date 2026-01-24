#!/usr/bin/env python

import os

root_in = "/Users/home/git_dir/main/strategy/data/textures_ihm_"
root_out = "/Users/home/git_dir/main/strategy/data/textures_ihm"

png_template = "/Users/home/git_dir/main/strategy/data/textures_ihm_/mode/play.png"

for root, dirs, files in os.walk(root_in):
	for d in dirs:
		d_out = os.path.join(root, d).replace(root_in, root_out)
		if not os.path.isdir(d_out):
			os.makedirs(d_out)
	for f in files:
		f_out = os.path.join(root, f).replace(root_in, root_out)
		if not os.path.isfile(f_out):
			os.system("cp %s %s" % (png_template, f_out))

#!/usr/bin/env python3

# https://medium.com/@a.j.kruschwitz/how-to-generate-a-normal-map-from-an-image-the-quick-and-dirty-way-36b73a18f1f1


import os, sys
import math
import subprocess

from PIL import Image, ImageFilter


def png2normal(diffuse_path, normal_path):
	with Image.open(diffuse_path) as im:
		grey= im.convert('L')
		sobelx= grey.filter(ImageFilter.Kernel((3, 3), (1, 2, 1, 0, 0, 0, -1, -2, -1), 1, 0))
		sobely= grey.filter(ImageFilter.Kernel((3, 3), (1, 0, -1, 2, 0, -2, 1, 0, -1), 1, 0))
		z= sobelx.point(lambda x : 0.5)
		sobelx_data= list(sobelx.getdata(0))
		sobely_data= list(sobely.getdata(0))
		normal_data= []
		for i in range(len(sobelx_data)):
			x= -1.0* float(sobelx_data[i])
			y= -1.0* float(sobely_data[i])
			z= 0.5

			norm= math.sqrt(x* x+ y* y+ z* z)
			x/= norm
			y/= norm
			z/= norm
			#print((x, y, z))

			r= int((x+ 1.0)* 0.5* 255.0)
			g= int((y+ 1.0)* 0.5* 255.0)
			b= int((z+ 1.0)* 0.5* 255.0)
			#print((r, g, b))

			normal_data.append((r, g, b))
		
		im_normal= Image.new("RGB", (1024, 1024))
		im_normal.putdata(normal_data)
		im_normal.save(normal_path)


def main(gif):
	with Image.open(gif) as im:
		try:
			compt= 0
			while 1:
				compt+= 1
				im.seek(im.tell() + 1)

				diffuse_path= os.path.join(os.path.dirname(gif), "diffuse", f"{os.path.basename(os.path.splitext(gif)[0])}_{compt}.png")
				normal_path = os.path.join(os.path.dirname(gif), "normal" , f"{os.path.basename(os.path.splitext(gif)[0])}_{compt}.png")

				for p in (diffuse_path, normal_path):
					if not os.path.isdir(os.path.dirname(p)):
						subprocess.run(("mkdir", "-p", os.path.dirname(p)))

				resized= im.resize((1024, 1024), Image.LANCZOS)
				resized.save(diffuse_path)

				png2normal(diffuse_path, normal_path)
		except EOFError:
			pass # end of sequence


if __name__== "__main__":
	assert len(sys.argv)== 2, "Donner en argument le chemin du gif à traiter"
	gif= sys.argv[1]
	assert os.path.isfile(gif) and os.path.splitext(gif)[1].lower()== ".gif", f"{gif} n'est pas un gif"
	main(gif)

#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os, sys, re, math
import colorsys
from subprocess import Popen, PIPE
from ftplib import FTP

import matplotlib
import matplotlib.pyplot as plt


HOST= "ftp.zoide.fr"
USER= "zoidefrkrg"
PASS= "7aTYt6zG7Tkp"


# ----------------------------------------------------------------------------------------
def get_info(ch):
	cmd= "identify %s" % ch
	#print cmd
	pop= Popen(cmd, shell=True, bufsize=-1, stdout=PIPE)
	pipe= pop.stdout
	line= pipe.readline()
	pipe.close()

	ncols_img, nligs_img= map(lambda x : int(x), line.split()[2].split("x"))
	return [ncols_img, nligs_img]


def get_mean_color(jpg):
	cmd= 'convert %s -filter triangle -resize 1x1! -format "%%[pixel:u]" info:' % jpg
	pop= Popen(cmd, shell=True, bufsize=-1, stdout=PIPE)
	pipe= pop.stdout
	line= pipe.readline().strip()
	pipe.close()
	red, green, blue= [ int(x) for x in line.split("srgb(")[1].split(")")[0].split(",") ]
	hue, saturation, value= colorsys.rgb_to_hsv(float(red)/ 255., float(green)/ 255., float(blue)/ 255.)
	return {"red" : red, "green" : green, "blue" : blue, "hue" : hue, "saturation" : saturation, "value" : value}


def get_mean_color_xing(jpg):
	cmd= 'XinG -i %s -H -n:stdout' % jpg
	pop= Popen(cmd, shell=True, bufsize=-1, stdout=PIPE)
	pipe= pop.stdout
	buf= pipe.readlines()
	pipe.close()
	l_moys= [ ]
	for line in buf:
		if "moyenne" in line:
			l_moys.append(float(line.split()[-1]))
	red, green, blue= l_moys
	hue, saturation, value= colorsys.rgb_to_hsv(float(red)/ 255., float(green)/ 255., float(blue)/ 255.)
	return {"red" : red, "green" : green, "blue" : blue, "hue" : hue, "saturation" : saturation, "value" : value}
	

# ----------------------------------------------------------------------------------------
def parse_nuancier_txt(nuancier_txt):
	f= open(nuancier_txt)
	buf= f.readlines()
	f.close()

	dic_noms= { }
	dic_params= { }
	l_mixs= [ ]

	for line in buf:
		if not line.strip() or line.strip().startswith("#"):
			continue
		if "=" in line:
			key= line.split("=")[0].strip()
			val= line.split("=")[1].strip()
			if key in ("nligs_color", "ncols_color", "color_ratio", "margin_ratio"):
				dic_params[key]= int(val)
			elif key in ("subsub_ratio", ):
				dic_params[key]= float(val)
			else:
				nom_long= val.split(";")[0].strip()
				marque  = val.split(";")[1].strip()
				dic_noms[key]= {"nom_long" : nom_long, "marque" : marque}
		else:
			l_mixs.append(map(lambda x : x.strip(), line.split(";")))
	
	return {"dic_noms" : dic_noms, "dic_params" : dic_params, "l_mixs" : l_mixs}
	
	
def gen_sub_imgs(dic_nuancier, root_subs, nuancier):
	ncols_img, nligs_img= get_info(nuancier)
	
	for col in range(dic_nuancier["dic_params"]["ncols_color"]):
		for lig in range(dic_nuancier["dic_params"]["nligs_color"]):
			
			if dic_nuancier["l_mixs"][lig][col]== "null":
				continue
			
			# a recoder : suppose que le rapport coleur / marge est 2 (ce qui est la cas ici : 2cm / 1cm)
			col_margin_size= ncols_img/ float(dic_nuancier["dic_params"]["ncols_color"]* dic_nuancier["dic_params"]["color_ratio"]+ (dic_nuancier["dic_params"]["ncols_color"]- 1)* dic_nuancier["dic_params"]["margin_ratio"])
			col_size= 2* col_margin_size
			col_offset= col* (col_margin_size+ col_size)

			lig_margin_size= nligs_img/ float(dic_nuancier["dic_params"]["nligs_color"]* dic_nuancier["dic_params"]["color_ratio"]+ (dic_nuancier["dic_params"]["nligs_color"]- 1)* dic_nuancier["dic_params"]["margin_ratio"])
			lig_size= 2* lig_margin_size
			lig_offset= lig* (lig_margin_size+ lig_size)

			sub_img= os.path.join(root_subs, "%s_%s.jpg" % (col, lig))
			cmd= "convert -extract %sx%s+%s+%s %s %s" % (col_size, lig_size, col_offset, lig_offset, nuancier, sub_img)
			#print cmd
			os.system(cmd)


def gen_subsub_imgs(dic_nuancier, root_subs, root_subsubs):
	for col in range(dic_nuancier["dic_params"]["ncols_color"]):
		for lig in range(dic_nuancier["dic_params"]["nligs_color"]):
			
			if dic_nuancier["l_mixs"][lig][col]== "null":
				continue
			
			sub_img= os.path.join(root_subs, "%s_%s.jpg" % (col, lig))
			ncols_img, nligs_img= get_info(sub_img)
			
			col_size= ncols_img* dic_nuancier["dic_params"]["subsub_ratio"]
			col_offset= ncols_img* (1.- dic_nuancier["dic_params"]["subsub_ratio"])* 0.5

			lig_size= nligs_img* dic_nuancier["dic_params"]["subsub_ratio"]
			lig_offset= nligs_img* (1.- dic_nuancier["dic_params"]["subsub_ratio"])* 0.5
			
			subsub_img= os.path.join(root_subsubs, "%s_%s.jpg" % (col, lig))
			cmd= "convert -extract %sx%s+%s+%s %s %s" % (col_size, lig_size, col_offset, lig_offset, sub_img, subsub_img)
			#print cmd
			os.system(cmd)


def gen_mixs_txt(dic_nuancier, root_subsubs, mixs_txt):
	to_w= "# subsub_img\tprimary_color\tprimary_color_marque\tsecondary_color\tsecondary_color_marque\tniveau_blanc\tred\tgreen\tblue\thue\tsaturation\tvalue\n\n"
	for col in range(dic_nuancier["dic_params"]["ncols_color"]):
		for lig in range(dic_nuancier["dic_params"]["nligs_color"]):
			
			if dic_nuancier["l_mixs"][lig][col]== "null":
				continue
			
			subsub_img= os.path.join(root_subsubs, "%s_%s.jpg" % (col, lig))
			dic_color= get_mean_color(subsub_img)

			mix= dic_nuancier["l_mixs"][lig][col]
			mix_split= mix.split("_")
			if len(mix_split)== 2:
				primary_color       = dic_nuancier["dic_noms"][mix_split[0]]["nom_long"]
				primary_color_marque= dic_nuancier["dic_noms"][mix_split[0]]["marque"]
				secondary_color, secondary_color_marque= None, None
				niveau_blanc= int(mix_split[1])
			elif len(mix_split)== 3:
				primary_color         = dic_nuancier["dic_noms"][mix_split[0]]["nom_long"]
				primary_color_marque  = dic_nuancier["dic_noms"][mix_split[0]]["marque"]
				secondary_color       = dic_nuancier["dic_noms"][mix_split[1]]["nom_long"]
				secondary_color_marque= dic_nuancier["dic_noms"][mix_split[1]]["marque"]
				niveau_blanc= int(mix_split[2])
			to_w+= "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (os.path.basename(subsub_img), primary_color, primary_color_marque, secondary_color, secondary_color_marque, niveau_blanc, 
				dic_color["red"], dic_color["green"], dic_color["blue"], dic_color["hue"], dic_color["saturation"], dic_color["value"])
	
	f= open(mixs_txt, "w")
	f.write(to_w)
	f.close()


# ----------------------------------------------------------------------------------------
def parse_nuancier_ref_txt(nuancier_ref_txt):
	f= open(nuancier_ref_txt)
	buf= f.readlines()
	f.close()
	
	dic_params= { }
	l_refs= { }
	for line in buf:
		if line.startswith("#") or not line.strip():
			continue
		
		if "=" in line:
			key= line.split("=")[0].strip()
			val= line.split("=")[1].strip()
			if key in ("nligs_color", "ncols_color"):
				dic_params[key]= int(val)
			elif key in ("subsub_ratio", "color_ratio", "margin_ratio"):
				dic_params[key]= float(val)
		else:
			lig, col, red, green, blue= map(lambda x : int(x), line.strip().split())
			if lig not in l_refs.keys():
				l_refs[lig]= { }
			l_refs[lig][col]= {"red" : red, "green" : green, "blue" : blue}
	
	return {"dic_params" : dic_params, "l_refs" : l_refs}
	

def gen_sub_imgs_ref(dic_nuancier_ref, root_subs_ref, nuancier_ref):
	ncols_img, nligs_img= get_info(nuancier_ref)
	
	for col in range(dic_nuancier_ref["dic_params"]["ncols_color"]):
		for lig in range(dic_nuancier_ref["dic_params"]["nligs_color"]):
			
			col_color_ratio= dic_nuancier_ref["dic_params"]["color_ratio"]/ (dic_nuancier_ref["dic_params"]["color_ratio"]* dic_nuancier_ref["dic_params"]["ncols_color"]+ dic_nuancier_ref["dic_params"]["margin_ratio"]* (dic_nuancier_ref["dic_params"]["ncols_color"]- 1))
			col_color_size= col_color_ratio* ncols_img
			col_margin_ratio= dic_nuancier_ref["dic_params"]["margin_ratio"]/ (dic_nuancier_ref["dic_params"]["color_ratio"]* dic_nuancier_ref["dic_params"]["ncols_color"]+ dic_nuancier_ref["dic_params"]["margin_ratio"]* (dic_nuancier_ref["dic_params"]["ncols_color"]- 1))
			col_margin_size= col_margin_ratio* ncols_img
			
			lig_color_ratio= dic_nuancier_ref["dic_params"]["color_ratio"]/ (dic_nuancier_ref["dic_params"]["color_ratio"]* dic_nuancier_ref["dic_params"]["nligs_color"]+ dic_nuancier_ref["dic_params"]["margin_ratio"]* (dic_nuancier_ref["dic_params"]["nligs_color"]- 1))
			lig_color_size= lig_color_ratio* nligs_img
			lig_margin_ratio= dic_nuancier_ref["dic_params"]["margin_ratio"]/ (dic_nuancier_ref["dic_params"]["color_ratio"]* dic_nuancier_ref["dic_params"]["nligs_color"]+ dic_nuancier_ref["dic_params"]["margin_ratio"]* (dic_nuancier_ref["dic_params"]["nligs_color"]- 1))
			lig_margin_size= lig_margin_ratio* nligs_img
			
			col_offset= col* (col_margin_size+ col_color_size)
			lig_offset= lig* (lig_margin_size+ lig_color_size)

			sub_img_ref= os.path.join(root_subs_ref, "%s_%s.jpg" % (col, lig))
			cmd= "convert -extract %sx%s+%s+%s %s %s" % (col_color_size, lig_color_size, col_offset, lig_offset, nuancier_ref, sub_img_ref)
			#print cmd
			os.system(cmd)
	
	
def gen_subsub_imgs_ref(dic_nuancier_ref, root_subs_ref, root_subsubs_ref):
	for col in range(dic_nuancier_ref["dic_params"]["ncols_color"]):
		for lig in range(dic_nuancier_ref["dic_params"]["nligs_color"]):
			
			sub_img_ref= os.path.join(root_subs_ref, "%s_%s.jpg" % (col, lig))
			ncols_img, nligs_img= get_info(sub_img_ref)
			
			col_size= ncols_img* dic_nuancier_ref["dic_params"]["subsub_ratio"]
			col_offset= ncols_img* (1.- dic_nuancier_ref["dic_params"]["subsub_ratio"])* 0.5

			lig_size= nligs_img* dic_nuancier_ref["dic_params"]["subsub_ratio"]
			lig_offset= nligs_img* (1.- dic_nuancier_ref["dic_params"]["subsub_ratio"])* 0.5
			
			subsub_img_ref= os.path.join(root_subsubs_ref, "%s_%s.jpg" % (col, lig))
			cmd= "convert -extract %sx%s+%s+%s %s %s" % (col_size, lig_size, col_offset, lig_offset, sub_img_ref, subsub_img_ref)
			#print cmd
			os.system(cmd)


def gen_refs_txt(dic_nuancier_ref, root_subsubs_ref, refs_txt):
	to_w= "# subsub_img_ref\tred_theo\tgreen_theo\tblue_theo\tred\tgreen\tblue\thue\tsaturation\tvalue\n\n"
	for col in range(dic_nuancier_ref["dic_params"]["ncols_color"]):
		for lig in range(dic_nuancier_ref["dic_params"]["nligs_color"]):
			
			subsub_img_ref= os.path.join(root_subsubs_ref, "%s_%s.jpg" % (col, lig))
			dic_color= get_mean_color(subsub_img_ref)
			
			ref= dic_nuancier_ref["l_refs"][lig][col]
			to_w+= "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (os.path.basename(subsub_img_ref), ref["red"], ref["green"], ref["blue"], dic_color["red"], dic_color["green"], dic_color["blue"], dic_color["hue"], dic_color["saturation"], dic_color["value"])
	
	f= open(refs_txt, "w")
	f.write(to_w)
	f.close()
	
	

# ----------------------------------------------------------------------------------------
def plot_ref(refs_txt, coeffs):
	f= open(refs_txt)
	buf= f.readlines()
	f.close()
	
	lx, ly= {"red" : [ ], "green" : [ ], "blue" : [ ]}, {"red" : [ ], "green" : [ ], "blue" : [ ]}
	
	for line in buf:
		if line.startswith("#") or not line.strip():
			continue
		
		subsub_img_ref, red_theo, green_theo, blue_theo, red, green, blue, hue, saturation, value= line.split()
		red_theo, green_theo, blue_theo, red, green, blue= map(lambda x : int(x), [red_theo, green_theo, blue_theo, red, green, blue])
		#hue, saturation, value= map(lambda x : float(x), [hue, saturation, value])
		
		lx["red"].append(red)
		ly["red"].append(red_theo)
		lx["green"].append(green)
		ly["green"].append(green_theo)
		lx["blue"].append(blue)
		ly["blue"].append(blue_theo)
	
	for color in ("red", "green", "blue"):
		fig, ax= plt.subplots()
		ax.plot(lx[color], ly[color], 'o')
		xmin, xmax= 0, 255
		ax.plot([xmin, xmax], [coeffs[color]["a"]* xmin+ coeffs[color]["b"], coeffs[color]["a"]* xmax+ coeffs[color]["b"]])
		ax.set_title(color)
		plt.show()


def compute_linear_coeffs(refs_txt):
	f= open(refs_txt)
	buf= f.readlines()
	f.close()
	
	pts= {"red" : [ ], "green" : [ ], "blue" : [ ]}
	
	for line in buf:
		if line.startswith("#") or not line.strip():
			continue
		
		subsub_img_ref, red_theo, green_theo, blue_theo, red, green, blue, hue, saturation, value= line.split()
		red_theo, green_theo, blue_theo, red, green, blue= map(lambda x : float(x), [red_theo, green_theo, blue_theo, red, green, blue])
		#hue, saturation, value= map(lambda x : float(x), [hue, saturation, value])
	
		pts["red"].append((red, red_theo))
		pts["green"].append((green, green_theo))
		pts["blue"].append((blue, blue_theo))
	
	coeffs= { }
	for color in ("red", "green", "blue"):
		lx, ly= zip(*pts[color])[0], zip(*pts[color])[1]
		npts= float(len(pts[color]))
		x_moy= sum(lx)/ npts
		y_moy= sum(ly)/ npts
		x_var= sum(map(lambda z : z** 2, lx))/ npts- x_moy** 2
		y_var= sum(map(lambda z : z** 2, ly))/ npts- y_moy** 2
		covar= sum(map(lambda z : z[0]* z[1], pts[color]))/ npts- x_moy* y_moy
		a= covar/ x_var
		b= y_moy- a* x_moy
		
		# problème : même en faisant gaffe, le nuancier reçoit de la lumière pas assez diffuse;
		# le nuancier de référence est mat, mais les couleurs à l'huile sont brillantes
		# donc ici b sera trop grand et toutes les couleurs insérées comme trop claires
		# je baisse arbitrairement b ...
		b-= 30
		
		coeffs[color]= {"a" : a, "b" : b}
	
	return coeffs


def gen_mixs_txt_corrected(mixs_txt, coeffs, mixs_corrected_txt):
	f= open(mixs_txt)
	buf= f.readlines()
	f.close()
	
	to_w= "# subsub_img\tprimary_color\tprimary_color_marque\tsecondary_color\tsecondary_color_marque\tniveau_blanc\tred\tgreen\tblue\thue\tsaturation\tvalue\n\n"
	for line in buf:
		if line.startswith("#") or not line.strip():
			continue
	
		subsub_img, primary_color, primary_color_marque, secondary_color, secondary_color_marque, niveau_blanc, red, green, blue, hue, saturation, value= line.split("\t")
		red, green, blue= map(lambda x : float(x), [red, green, blue])
		#hue, saturation, value= map(lambda x : float(x), [hue, saturation, value])
		
		red_corrected  = coeffs["red"]["a"]  * red  + coeffs["red"]["b"]
		green_corrected= coeffs["green"]["a"]* green+ coeffs["green"]["b"]
		blue_corrected = coeffs["blue"]["a"] * blue + coeffs["blue"]["b"]
		hue_corrected, saturation_corrected, value_corrected= colorsys.rgb_to_hsv(red_corrected/ 255.0, green_corrected/ 255.0, blue_corrected/ 255.0)
		
		to_w+= "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (os.path.basename(subsub_img), primary_color, primary_color_marque, secondary_color, secondary_color_marque, niveau_blanc, 
			red_corrected, green_corrected, blue_corrected, hue_corrected, saturation_corrected, value_corrected)
	
	f= open(mixs_corrected_txt, "w")
	f.write(to_w)
	f.close()


def gen_subsub_imgs_corrected(root_subsubs, coeffs, root_subsubs_corrected):
	subsubs_img= [ os.path.join(root_subsubs, x) for x in os.listdir(root_subsubs) if os.path.splitext(x)[1].upper() in (".JPEG", ".JPG") ]
	for subsub_img in subsubs_img:
		subsub_img_corrected= os.path.join(root_subsubs_corrected, os.path.basename(subsub_img))
		subsub_img_corrected_red  = os.path.splitext(subsub_img_corrected)[0]+ "_red.jpg"
		subsub_img_corrected_green= os.path.splitext(subsub_img_corrected)[0]+ "_green.jpg"
		subsub_img_corrected_blue = os.path.splitext(subsub_img_corrected)[0]+ "_blue.jpg"

		cmd= 'convert %s -fx "u.r* %s + %s" %s' % (subsub_img, coeffs["red"]["a"]  , coeffs["red"]["b"]/ 255.0  , subsub_img_corrected_red)
		#print cmd
		os.system(cmd)
		cmd= 'convert %s -fx "u.g* %s + %s" %s' % (subsub_img, coeffs["green"]["a"], coeffs["green"]["b"]/ 255.0, subsub_img_corrected_green)
		#print cmd
		os.system(cmd)
		cmd= 'convert %s -fx "u.b* %s + %s" %s' % (subsub_img, coeffs["blue"]["a"] , coeffs["blue"]["b"]/ 255.0 , subsub_img_corrected_blue)
		#print cmd
		os.system(cmd)
		cmd= "convert %s %s %s -channel RGB -colorspace sRGB -combine %s" % (subsub_img_corrected_red, subsub_img_corrected_green, subsub_img_corrected_blue, subsub_img_corrected)
		#print cmd
		os.system(cmd)
		
		os.system("rm %s %s %s" % (subsub_img_corrected_red, subsub_img_corrected_green, subsub_img_corrected_blue))
		#break


def gen_ref_corrected(nuancier_ref, coeffs, nuancier_ref_corrected):
	ref_corrected_red  = os.path.splitext(nuancier_ref_corrected)[0]+ "_red.jpg"
	ref_corrected_green= os.path.splitext(nuancier_ref_corrected)[0]+ "_green.jpg"
	ref_corrected_blue = os.path.splitext(nuancier_ref_corrected)[0]+ "_blue.jpg"
	
	cmd= 'convert %s -fx "u.r* %s + %s" %s' % (nuancier_ref, coeffs["red"]["a"]  , coeffs["red"]["b"]/ 255.0  , ref_corrected_red)
	#print cmd
	os.system(cmd)
	cmd= 'convert %s -fx "u.g* %s + %s" %s' % (nuancier_ref, coeffs["green"]["a"], coeffs["green"]["b"]/ 255.0, ref_corrected_green)
	#print cmd
	os.system(cmd)
	cmd= 'convert %s -fx "u.b* %s + %s" %s' % (nuancier_ref, coeffs["blue"]["a"] , coeffs["blue"]["b"]/ 255.0 , ref_corrected_blue)
	#print cmd
	os.system(cmd)
	cmd= "convert %s %s %s -channel RGB -colorspace sRGB -combine %s" % (ref_corrected_red, ref_corrected_green, ref_corrected_blue, nuancier_ref_corrected)
	#print cmd
	os.system(cmd)
	
	os.system("rm %s %s %s" % (ref_corrected_red, ref_corrected_green, ref_corrected_blue))
	

def upload_colors(root_subsubs_corrected, mixs_corrected_txt, dir_serveur):
	files2upload= [ os.path.join(root_subsubs_corrected, x) for x in os.listdir(root_subsubs_corrected) if os.path.splitext(x)[1].upper() in (".JPG", ".JPEG") and not x.startswith(".") ]
	files2upload.append(mixs_corrected_txt)
	
	ftp= FTP(HOST)
	ftp.login(USER, PASS)
	ftp.cwd(dir_serveur)

	for ch in files2upload:
		os.chdir(os.path.dirname(ch))
		f= open(os.path.basename(ch), "rb")
		ftp.storbinary("STOR %s" % os.path.basename(ch), f)
		f.close()
	
	os.chdir(root_subsubs_corrected)

	ftp.quit()
	
	
		
# ----------------------------------------------------------------------------------------
def main():
	root= "/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/images/nuanciers_V2"
	#root= "/Users/home/ovh/www/images/nuanciers_V2"
	root_serveur= "/www/images/nuanciers_V2"
	nuancier_ref_txt= os.path.join(root, "srcs", "nuancier_ref.txt")

	for i in range(1, 5):
		#if i!= 1: continue # debug

		root_out              = os.path.join(root, "nuancier_%s" % i)
		root_subs             = os.path.join(root_out, "root_subs")
		root_subsubs          = os.path.join(root_out, "root_subsubs")
		root_subs_ref         = os.path.join(root_out, "root_subs_ref")
		root_subsubs_ref      = os.path.join(root_out, "root_subsubs_ref")
		root_subsubs_corrected= os.path.join(root_out, "root_subsubs_corrected")
		nuancier              = os.path.join(root, "srcs", "nuancier_%s.JPG" % i)
		nuancier_txt          = os.path.join(root, "srcs", "nuancier_%s.txt" % i)
		nuancier_ref          = os.path.join(root, "srcs", "nuancier_%s_ref.JPG" % i)
		nuancier_ref_corrected= os.path.join(root_out, "nuancier_%s_ref_corrected.JPG" % i)
		refs_txt              = os.path.join(root_out, "refs.txt")
		mixs_txt              = os.path.join(root_out, "mixs.txt")
		mixs_corrected_txt    = os.path.join(root_out, "mixs_corrected.txt")
		dir_serveur           = os.path.join(root_serveur, "nuancier_%s" % i)

		os.system("rm -r %s 2>/dev/null" % root_out)
		os.mkdir(root_out)
		os.mkdir(root_subs)
		os.mkdir(root_subsubs)
		os.mkdir(root_subs_ref)
		os.mkdir(root_subsubs_ref)
		os.mkdir(root_subsubs_corrected)
		
		dic_nuancier= parse_nuancier_txt(nuancier_txt)
		gen_sub_imgs(dic_nuancier, root_subs, nuancier)
		gen_subsub_imgs(dic_nuancier, root_subs, root_subsubs)
		gen_mixs_txt(dic_nuancier, root_subsubs, mixs_txt)
	
		dic_nuancier_ref= parse_nuancier_ref_txt(nuancier_ref_txt)
		gen_sub_imgs_ref(dic_nuancier_ref, root_subs_ref, nuancier_ref)
		gen_subsub_imgs_ref(dic_nuancier_ref, root_subs_ref, root_subsubs_ref)
		gen_refs_txt(dic_nuancier_ref, root_subsubs_ref, refs_txt)
		
		coeffs= compute_linear_coeffs(refs_txt)
		print coeffs
		#plot_ref(refs_txt, coeffs)
		gen_mixs_txt_corrected(mixs_txt, coeffs, mixs_corrected_txt)
		gen_subsub_imgs_corrected(root_subsubs, coeffs, root_subsubs_corrected)
		
		# pour verif
		gen_ref_corrected(nuancier_ref, coeffs, nuancier_ref_corrected)
		
		upload_colors(root_subsubs_corrected, mixs_corrected_txt, dir_serveur)


def test():
	#root_subsubs          = "/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/images/nuanciers_V2/nuancier_1/root_subsubs"
	#root_subsubs_corrected= "/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/images/nuanciers_V2/nuancier_1/root_subsubs_corrected"
	#coeffs= {
	#	'blue' : {'a': 1.350786673914732 , 'b': 40.020431259868985},
	#	'green': {'a': 1.3109730238046948, 'b': 38.39448306381722},
	#	'red'  : {'a': 1.2110349583133087, 'b': 44.26599818208412}
	#}
	#gen_subsub_imgs_corrected(root_subsubs, coeffs, root_subsubs_corrected)
	
	root_subsubs_corrected= "/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/images/nuanciers_V2/nuancier_1/root_subsubs_corrected"
	mixs_corrected_txt= "/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/images/nuanciers_V2/nuancier_1/mixs_corrected.txt"
	dir_serveur= "/www/images/nuanciers_V2/nuancier_1"
	upload_colors(root_subsubs_corrected, mixs_corrected_txt, dir_serveur)
	pass	
	
	
# ----------------------------------------------------------------------------------------
main()
#test()

#!/usr/bin/env python
# -*- coding:utf-8 -*- 

#
# script GIMP, faire un lien symbolique de ce script dans le dossier /Users/home/Library/Application\ Support/GIMP/2.8/plug-ins
# pour le faire apparaitre dans GIMP
# on le déclenche depuis GIMP menu/Filtres/MesFiltres/Anim 2D
# ATTENTION ne pas sauvegarder le projet à l'issue du lancement de ce script car tous les calques auront subis un miroir
#


import os, sys, re

from gimpfu import *

# pour faire un print ouvrir la console d'erreurs et faire :
#gimp.pdb.gimp_message('This is displayed as a message')

default_dir= os.path.join(os.path.dirname(os.path.realpath(__file__)), "../data/anim_textures")


# ---------------------------------------------------------------------------------------------------------
def anim_2d(root_out):
	
	savefn= gimp.pdb['file-png-save-defaults']

	img= gimp.image_list()[0]

	for group in img.layers:
		for lay in group.layers:
			out_name= os.path.join(root_out, group.name, lay.name + ".png")
			if not os.path.isdir(os.path.dirname(out_name)):
				os.system("mkdir -p %s" % os.path.dirname(out_name))
			savefn(img, lay, out_name, out_name)


def anim_2d_symmetry(root_out):
	
	savefn= gimp.pdb['file-png-save-defaults']

	img= gimp.image_list()[0]

	for group in img.layers:
		for lay in group.layers:
			out_name= os.path.join(root_out, "right_"+ group.name, "right_"+ lay.name + ".png")
			if not os.path.isdir(os.path.dirname(out_name)):
				os.system("mkdir -p %s" % os.path.dirname(out_name))
			savefn(img, lay, out_name, out_name)

	pdb.gimp_image_flip(img, ORIENTATION_HORIZONTAL)

	for group in img.layers:
		for lay in group.layers:
			out_name= os.path.join(root_out, "left_"+ group.name, "left_"+ lay.name + ".png")
			if not os.path.isdir(os.path.dirname(out_name)):
				os.system("mkdir -p %s" % os.path.dirname(out_name))
			savefn(img, lay, out_name, out_name)

# ---------------------------------------------------------------------------------------------------------
register(
	"python_fu_anim_2d", # name
	"Anim 2D", # blurb
	"save pour anim 2D", # help
	"PMB", # author
	"PMB", # copyright
	"2020", # date
	"Anim 2D", # menupath
	"", # imagetypes
	[ 
		(PF_DIRNAME, "root_out", "root_out", default_dir)
	], # params
	[], # results
	anim_2d, # function
	menu="<Image>/Filters/MesFiltres"
)


register(
	"python_fu_anim_2d_symmetry", # name
	"Anim 2D Symmetry", # blurb
	"save + mirror pour anim 2D", # help
	"PMB", # author
	"PMB", # copyright
	"2020", # date
	"Anim 2D Symmetry", # menupath
	"", # imagetypes
	[ 
		(PF_DIRNAME, "root_out", "root_out", default_dir)
	], # params
	[], # results
	anim_2d_symmetry, # function
	menu="<Image>/Filters/MesFiltres"
)


# ---------------------------------------------------------------------------------------------------------
main()

#!/usr/bin/env python
# -*- coding:utf-8 -*- 

#
# script GIMP, faire un lien symbolique de ce script dans le dossier /Users/home/Library/Application\ Support/GIMP/2.8/plug-ins
# pour le faire apparaitre dans GIMP
# on le déclenche depuis GIMP menu/Filtres/MesFiltres/Anim 2D
#


import os, sys, re

from gimpfu import *


def anim_2d(root_out):
    # pour faire un print ouvrir la console d'erreurs et faire :
    #gimp.pdb.gimp_message('This is displayed as a message')
    
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


register(
    "python_fu_anim_2d",
    "Anim 2D",
    "save + mirror pour anim 2D",
    "PMB",
    "PMB",
    "2020",
    "Anim 2D",
    "",
    [
        (PF_DIRNAME, "root_out", "root_out", '/Users/home/git_dir/main/anim_2d/data/anim_textures')
    ],
    [],
    anim_2d, menu="<Image>/Filters/MesFiltres")

main()

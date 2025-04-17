#!/usr/bin/env bash

source /Volumes/Data/perso/image_magick/set_image_magick_env.sh

root_pngs=$1

magick -delay 4 -loop 1 ${root_pngs}/*.png anim.gif

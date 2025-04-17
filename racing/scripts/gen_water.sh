#!/usr/bin/env bash

# génération eau avec vagues

# cf /Volumes/Data/perso/image_magick/set_image_magick_env.sh
# afin d'avoir une version de ImageMagick qui gère les PNG

source /Volumes/Data/perso/image_magick/set_image_magick_env.sh

water="../data/tests/textures/water.png"
root_water_out="../data/water"

rm ${root_water_out}/water*.png

for i in {0..31}
do
	s=`printf '%02d' $i`
	water_warped="${root_water_out}/water_${s}.png"
	offset=$((i*32))
	magick $water -splice ${offset}x0+0+0 -wave 10x512 -chop ${offset}x0+0+0 $water_warped
	magick $water_warped -gravity South -chop 0x60 $water_warped
	magick $water_warped -gravity North -chop 0x60 $water_warped
	magick $water_warped -resize 1024x1024\! $water_warped
done

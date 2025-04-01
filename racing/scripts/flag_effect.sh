#!/usr/bin/env bash

# cf /Volumes/Data/perso/image_magick/set_image_magick_env.sh
# afin d'avoir une version de ImageMagick qui gère les PNG

source /Volumes/Data/perso/image_magick/set_image_magick_env.sh

start="/Volumes/Data/perso/dev/main/racing/data/static_objects/floating_objects/textures/start.png"

for i in {1..10}
do
	start_warped="${start%.*}_active_${i}.png"
	offset=$((i*30))
	magick $start -splice ${offset}x0+0+0 -wave 30x500 -chop ${offset}x0+0+0 $start_warped
	magick $start_warped -gravity South -chop 0x60 $start_warped
	magick $start_warped -transparent white $start_warped
done

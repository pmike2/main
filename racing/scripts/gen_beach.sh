#!/usr/bin/env bash

# génération plage

# cf /Volumes/Data/perso/image_magick/set_image_magick_env.sh
# afin d'avoir une version de ImageMagick qui gère les PNG

source /Volumes/Data/perso/image_magick/set_image_magick_env.sh

sand="../data/tests/textures/sand.png"
root_beach_out="../data/water"
beach_right="${root_beach_out}/beach_right.png"
beach_left="${root_beach_out}/beach_left.png"
beach_top="${root_beach_out}/beach_top.png"
beach_bottom="${root_beach_out}/beach_bottom.png"
beach_bottom_left="${root_beach_out}/beach_bottom_left.png"
beach_bottom_right="${root_beach_out}/beach_bottom_right.png"
beach_top_left="${root_beach_out}/beach_top_left.png"
beach_top_right="${root_beach_out}/beach_top_right.png"

rm ${root_beach_out}/beach*.png

# les bords
magick $sand -alpha set -background none -channel A -sparse-color barycentric "0,0 none %[w],0 white" +channel $beach_right
magick $sand -alpha set -background none -channel A -sparse-color barycentric "%[w],0 none 0,0 white" +channel $beach_left
magick $sand -alpha set -background none -channel A -sparse-color barycentric "0,%[h] none 0,0 white" +channel $beach_top
magick $sand -alpha set -background none -channel A -sparse-color barycentric "0,0 none 0,%[h] white" +channel $beach_bottom

# les coins
magick $sand -alpha set -background none -channel A -sparse-color barycentric "512,512 none 0,1024 white" +channel $beach_bottom_left
magick $sand -alpha set -background none -channel A -sparse-color barycentric "512,512 none 1024,1024 white" +channel $beach_bottom_right
magick $sand -alpha set -background none -channel A -sparse-color barycentric "512,512 none 0,0 white" +channel $beach_top_left
magick $sand -alpha set -background none -channel A -sparse-color barycentric "512,512 none 1024,0 white" +channel $beach_top_right

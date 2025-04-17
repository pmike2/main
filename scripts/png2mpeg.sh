#!/usr/bin/env bash

# cf https://stackoverflow.com/questions/24961127/how-to-create-a-video-from-images-with-ffmpeg

# il faut ffmpeg compilé avec libx264
# git clone https://code.videolan.org/videolan/x264.git
# ./configure --enable-static --disable-opencl --disable-asm
# make && sudo make install
# git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
# ./configure --enable-libx264 --enable-gpl
# make && sudo make install

ffmpeg -framerate 30 -i ../data/out_%05d.png -c:v libx264 -pix_fmt yuv420p ../data/out.mp4

# add sound
#ffmpeg -framerate 30 -pattern_type glob -i '../data/*.png' -i audio.ogg -c:a copy -shortest -c:v libx264 -pix_fmt yuv420p ../data/out.mp4

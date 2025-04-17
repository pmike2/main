#!/bin/bash


# lister les devices
#ffmpeg -f avfoundation -list_devices true -i ""


# record de l'écran (si 1 est l'écran)
ffmpeg -f avfoundation -r 25 -s 1280x720 -i 1: out.mov

#ffmpeg -f avfoundation -r 25 -s 1280x720 -i 1:1 -c:v libx264 -c:a ac3 -crf 20 -map 0:v:0 -map 0:a:0 out.mp4

#ffmpeg -f avfoundation -r 25 -s 1280x720 -i 1:0 -c:v libx264 -codec:a libmp3lame -qscale:a 2 -crf 20 -map 0:v:0 -map 0:a:0 -map_channel 0.1.0:0.1 out.mp4

#ffmpeg -f avfoundation -i 1:1 -map 0:v:0 -map 0:a:0 out.mp4
#ffmpeg -f avfoundation -i 1:1 -map 0:a:0 out.mp3


#ffmpeg -f avfoundation \
#-i 1:1 \
#-c:a ac3 \
#-map 0:0 \
#out.mp3


#ffmpeg -f avfoundation -i 1:1 -filter_complex "scale=320:-1;[0:7][0:8]amerge" -codec:v libx264 \
#-preset medium -crf 23 -codec:a libmp3lame -q:a 5 -ar 44100 output.mp4


#ffmpeg -f avfoundation -i 1:1 -codec:a libmp3lame -qscale:a 2 -map 0:a:0 -map_channel 0.1.0:0.0 -map_channel 0.1.1:0.1 \
#-map_channel -1 \
#-map_channel -1 \
#-map_channel -1 \
#-map_channel -1 \
#-map_channel -1 \
#-map_channel -1 \
#-map_channel -1 \
#-map_channel -1 \
#-map_channel -1 \
#-map_channel -1 \
#-ac 2 out.mp3

#ffmpeg -f avfoundation -i 1:1 -codec:a libmp3lame -qscale:a 2 -filter_complex "[0:1:0][0:1:1]amerge" out.mp3
#ffmpeg -f avfoundation -i 1:1 -codec:a pcm_f32le -filter_complex "[0:1:0][0:1:1]amerge=inputs=2[a]" -map "[a]" -y out.wav


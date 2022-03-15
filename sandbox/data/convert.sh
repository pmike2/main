#!/usr/bin/env bash

# https://stackoverflow.com/questions/46494532/ffmpeg-highest-quality-mp4-to-mpg

mp4="/Volumes/Data/perso/dev/main/sandbox/data/test.mp4"
mpg="/Volumes/Data/perso/dev/main/sandbox/data/test.mpg"
ffmpeg -y -i $mp4 -c:v mpeg1video -q:v 5 -c:a mp2 -f vob $mpg
#ffmpeg -y -i $mp4 -c:v mpeg1video -q:v 1 $mpg

#ffprobe $mp4 2>&1 | grep -i stream

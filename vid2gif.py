#!/usr/bin/env python
# -*- coding:utf-8 -*- 

import os, sys, re
from subprocess import Popen, PIPE


# emplacements binaires
FFMPEG = "/Volumes/Vol-Users/Users/pmbeau2/Desktop/ffmpeg"
FFPROBE= "/Volumes/Vol-Users/Users/pmbeau2/Desktop/ffprobe"

#FFMPEG = "C:\\Users\\pmb\\Desktop\\test_cam\\ffmpeg\\bin\\ffmpeg.exe"
#FFPROBE= "C:\\Users\\pmb\\Desktop\\test_cam\\ffmpeg\\bin\\ffprobe.exe"

# facteur d'accélération de la vidéo; + SPEED_UP est petit, + la video est accélérée
SPEED_UP= 0.01

# nbre de fois que la derniere image est loopée
LAST_FRAME_LOOP= 150

# si None, pas de rotation, sinon (cf http://stackoverflow.com/questions/3937387/rotating-videos-with-ffmpeg) :
# 1=clockwise, 2=counterclockwise
TRANSPOSE= None

# hauteur constante
HEIGHT= 240

# niveau de log
LOGLEVEL= "error"

# ----------------------------------------------------------------------------------------
# chemins
assert len(sys.argv)== 2, "Donner le chemin de la vidéo ou du gif à traiter en argument."
vid_in= sys.argv[1]
vid_gif= os.path.splitext(vid_in)[0]+ "_tmp.gif"
last_frame= os.path.splitext(vid_in)[0]+ ".png"
last_frame_looped= os.path.splitext(vid_in)[0]+ "_last_frame_looped.gif"
vid_out= os.path.splitext(vid_in)[0]+ "_freezed.gif"

# ----------------------------------------------------------------------------------------
# recup dimensions
cmd= '%s -loglevel %s -show_entries stream=width,height %s' % (FFPROBE, LOGLEVEL, vid_in)
pop= Popen(cmd, shell=True, bufsize=-1, stdout=PIPE)
pipe= pop.stdout
buf= pipe.readlines()
pipe.close()
for line in buf:
	line= line.decode("utf-8").strip()
	if line.startswith("width="):
		width= int(line.split("width=")[1])
	if line.startswith("height="):
		height= int(line.split("height=")[1])
ratio= float(width)/ float(height)
#print("width=%s; height=%s; ratio=%s" % (width, height, ratio))

# ----------------------------------------------------------------------------------------
# conversion en gif aux bonnes dimensions et accélérée si pas déjà .gif
if os.path.splitext(vid_in)[1].lower()== ".gif":
	vid_gif= vid_in
else:
	if TRANSPOSE is None:
		gif_height= HEIGHT
		gif_width= (width* HEIGHT)/ height
	else:
		gif_width= HEIGHT
		gif_height= (height* HEIGHT)/ width
		
	filters= ["setpts=%s*PTS" % SPEED_UP, "scale=%s:%s" % (gif_width, gif_height)]
	if TRANSPOSE is not None:
		filters.append("transpose=%s" % TRANSPOSE)

	cmd= """%s -loglevel %s -y -i %s -filter:v "%s" %s""" % (FFMPEG, LOGLEVEL, vid_in, ",".join(filters), vid_gif)
	os.system(cmd)

# ----------------------------------------------------------------------------------------
# récup du nombre de frames
cmd= "%s -loglevel %s -show_frames -i %s" % (FFPROBE, LOGLEVEL, vid_gif)
pop= Popen(cmd, shell=True, bufsize=-1, stdout=PIPE)
pipe= pop.stdout
buf= pipe.readlines()
pipe.close()
n_frames= 0
for line in buf:
	line= line.decode("utf-8").strip()
	if "[FRAME]" in line:
		n_frames+=1
#print(n_frames)

# ----------------------------------------------------------------------------------------
# récup du dernier frame
cmd= """%s -loglevel %s -y -i %s -vf "select='eq(n,%s)'" -vframes 1 %s""" % (FFMPEG, LOGLEVEL, vid_gif, n_frames- 1, last_frame)
os.system(cmd)

# ----------------------------------------------------------------------------------------
# gif qui boucle sur le dernier frame
cmd= "%s -loglevel %s -y -stream_loop %s -i %s %s" % (FFMPEG, LOGLEVEL, LAST_FRAME_LOOP, last_frame, last_frame_looped)
os.system(cmd)

# ----------------------------------------------------------------------------------------
# concaténation du gif initial et du gif du dernier frame loopé
cmd= """%s -loglevel %s -y -i %s -i %s -filter_complex "[0:v:0] [1:v:0] concat=n=2:v=1 [v]" -map "[v]" %s""" % (FFMPEG, LOGLEVEL, vid_gif, last_frame_looped, vid_out)
os.system(cmd)

# ----------------------------------------------------------------------------------------
# nettoyage
for ch in (last_frame, last_frame_looped):
	if os.path.exists(ch):
		os.remove(ch)
if os.path.splitext(vid_in)[1].lower()!= ".gif":
	os.remove(vid_gif)


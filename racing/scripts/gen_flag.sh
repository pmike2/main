#!/usr/bin/env bash

# script de génération du drapeau animé

# cf /Volumes/Data/perso/image_magick/set_image_magick_env.sh
# afin d'avoir une version de ImageMagick qui gère les PNG

source /Volumes/Data/perso/image_magick/set_image_magick_env.sh

start="../data/static_objects/floating_objects/textures/start.png"

rm ${start%.*}*

square_size=128
n_squares=$((1024/square_size))
draw_str=''

for i in $(seq $n_squares)
do
	for j in $(seq $n_squares)
	do
		parity=$(((i+j)%2))
		if [ $parity -eq 0 ]
		then
			x0=$(((i-1)* square_size))
			y0=$(((j-1)* square_size))
			x1=$((x0+square_size))
			y1=$((y0+square_size))
			draw_str="${draw_str} rectangle ${x0},${y0} ${x1},${y1}"
		fi
	done
done

#echo $draw_str

magick -size 1024x1024 canvas:white $start
magick $start -fill black -draw "$draw_str" $start
magick $start -alpha set -background none -channel A -evaluate multiply 0.5 +channel $start

for i in {0..31}
do
	s=`printf '%02d' $i`
	start_warped="${start%.*}_${s}.png"
	offset=$((i*32))
	magick $start -splice ${offset}x0+0+0 -wave 10x512 -chop ${offset}x0+0+0 $start_warped
	magick $start_warped -gravity South -chop 0x60 $start_warped
	magick $start_warped -transparent white -transparent black $start_warped
	magick $start_warped -fill red -draw "rectangle 0,970 1024,1024" $start_warped
done

#magick $start -fill red -draw "rectangle 0,970 1024,1024" $start
rm $start

pyscript_path=`dirname $0`/gen_flag_json.py
python3 $pyscript_path

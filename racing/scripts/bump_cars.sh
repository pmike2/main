#!/usr/bin/env bash

# script de création des voitures en mode bump (abimées)

# cf /Volumes/Data/perso/image_magick/set_image_magick_env.sh
# afin d'avoir une version de ImageMagick qui gère les PNG

source /Volumes/Data/perso/image_magick/set_image_magick_env.sh

root_cars="/Volumes/Data/perso/dev/main/racing/data/cars/textures"

for car in `ls $root_cars`
do
	if [[ $car == *"bump"* ]]
	then
		continue
	fi

	echo "traitement ${car}"

	car_abs=${root_cars}/${car}
	car_bump="${car_abs%.*}_bump.png"
	#echo $car_abs
	#echo $car_bump

	# distortions; cf https://usage.imagemagick.org/warping/
	# le 1er param est l'amplitude, le 2eme la fréquence
	# on ne peut pas gérer directement l'angle il faut faire des rotations pré et post distortion
	magick $car_abs -wave 16x250 $car_bump
	magick $car_bump -rotate -90 -wave 9x150 -rotate +90 $car_bump
	magick $car_bump -wave 6x60 $car_bump
	magick $car_bump -rotate -90 -wave 2x30 -rotate +90 $car_bump

	# les distos rajoutent + ou - de pixels de bord que l'on supprime
	magick $car_bump -gravity South -chop 0x22 $car_bump
	magick $car_bump -gravity North -chop 0x22 $car_bump
	magick $car_bump -gravity East -chop 11x0 $car_bump
	magick $car_bump -gravity West -chop 11x0 $car_bump

	# il reste à mettre à transparent des zones rectangulaires aux bords de l'image
	# qui ont été mises à blanc par les distos
	magick $car_bump -fill magenta -draw "rectangle 0,0 1024,100 rectangle 0,924 1024,1024 rectangle 0,0 100,1024 rectangle 924,0 1024,1024" -transparent magenta $car_bump
	
	#break
done

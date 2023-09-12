#!/bin/bash

# en argument le fichier alti à traiter
alti=$1

dir_alti=`dirname $alti`
alti_f="${alti%.*}_f.tif"
obstacle="${alti%.*}_obstacle.shp"

TMP="${dir_alti}/tmp"
XING="xingng"
TRESH="2"
SIMPLIFY="3.0"

rm -r $TMP 2> /dev/null
mkdir $TMP
rm "${dir_alti}/obstacle.*" $alti_f 2> /dev/null

# code C++ veut du float
$XING -t f -i $alti -o $alti_f -n:

# calcul obstacle
gdaldem slope $alti ${TMP}/slope.tif > /dev/null
$XING -t uc -i ${TMP}/slope.tif -o ${TMP}/slope_uc.tif -n:
$XING -i ${TMP}/slope_uc.tif -e "I1<${TRESH}?0:255" -o ${TMP}/slope_uc_seuil.tif -n:
$XING -i ${TMP}/slope_uc_seuil.tif -XM:D/1/1/1:E/1/6/6:D/1/5/5 -o ${TMP}/slope_uc_seuil_ed.tif -n:
$XING -i ${TMP}/slope_uc_seuil_ed.tif -g2:2:4326:::1:1:0:0 -o ${TMP}/slope_uc_seuil_ed_geo.tif -n:
gdal_polygonize.py -mask ${TMP}/slope_uc_seuil_ed_geo.tif  ${TMP}/slope_uc_seuil_ed_geo.tif ${TMP}/polygons.shp > /dev/null
ogr2ogr -simplify $SIMPLIFY $obstacle ${TMP}/polygons.shp > /dev/null

#rm -r ${TMP}

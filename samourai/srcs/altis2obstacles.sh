#!/bin/bash

# en argument le dossier contenant altis.tif
cd $1

TMP="tmp"
XING="/Volumes/malenge/xingng"
TRESH=60

rm -r $TMP 2> /dev/null
mkdir $TMP
rm obstacle.* 2> /dev/null

gdaldem slope altis.tif ${TMP}/slope.tif > /dev/null
$XING -t uc -i ${TMP}/slope.tif -o ${TMP}/slope_uc.tif -n:
$XING -i ${TMP}/slope_uc.tif -e "I1<${TRESH}?0:255" -o ${TMP}/slope_uc_seuil.tif -n:
$XING -i ${TMP}/slope_uc_seuil.tif -XM:E/2/3/3:D/2/3/3 -o ${TMP}/slope_uc_seuil_ed.tif -n:
$XING -i ${TMP}/slope_uc_seuil_ed.tif -g2:2:4326:::1:1:0:0 -o ${TMP}/slope_uc_seuil_ed_geo.tif -n:
gdal_polygonize.py -mask ${TMP}/slope_uc_seuil_ed_geo.tif  ${TMP}/slope_uc_seuil_ed_geo.tif obstacle.shp > /dev/null
rm -r ${TMP}

cd

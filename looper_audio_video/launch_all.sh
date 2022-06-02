#!/bin/bash


# script gérant tout ; ex :
# launch_all.sh LOOPER VIDEO AUDIO


args="$@"


in_args () {
	match="$1"
	for e in $args
	do
		[[ "$e" == "$match" ]] && return 0
	done
	return 1
}

SCRIPT_DIR=`dirname $0`
BUILD_DIR="${SCRIPT_DIR}/build"
ROOT_DATA="${SCRIPT_DIR}/data"
ROOT_WEB="${SCRIPT_DIR}/../web"
CHROME="/Applications/ADD_SOFTS/WEB/Google Chrome.app/Contents/MacOS/Google Chrome"


# kill
if in_args "KILL"
then
	pkill node
	pkill looper_gl_main
	pkill video_sampler_main
	pkill audio_sampler_main
	pkill cv_in_main
	pkill cv_out_main
	pkill receiver_test_main
	exit
fi

# looper
if in_args "LOOPER"
then
	${BUILD_DIR}/looper_gl_main &
fi

# lit les entrées de la carte son pour déclencher les events. Remplace looper
if in_args "CV_IN"
then
	node ${ROOT_WEB}/cv_in_config_edit/server.js ${ROOT_DATA} &
	"${CHROME}" "http://localhost:3003" > /dev/null
	${BUILD_DIR}/cv_in_main ${ROOT_DATA}/cv_in_configs/cv_in_config_01.json &
fi

# play videos
if in_args "VIDEO"
then
	node ${ROOT_WEB}/video_config_edit/server.js ${ROOT_DATA} &
	"${CHROME}" "http://localhost:3000" > /dev/null
	${BUILD_DIR}/video_sampler_main ${ROOT_DATA}/video_configs/video_config_01.json &
fi

# play audio samples
if in_args "AUDIO"
then
	node ${ROOT_WEB}/audio_config_edit/server.js ${ROOT_DATA} &
	"${CHROME}" "http://localhost:3001" > /dev/null
	${BUILD_DIR}/audio_sampler_main ${ROOT_DATA}/audio_configs/audio_config_01.json &
fi

# émet des pulses sur les sorties de la carte son
if in_args "CV_OUT"
then
	node ${ROOT_WEB}/cv_out_config_edit/server.js ${ROOT_DATA} &
	"${CHROME}" "http://localhost:3002" > /dev/null
	${BUILD_DIR}/cv_out_main ${ROOT_DATA}/cv_out_configs/cv_out_config_01.json &
fi

# test receiver
if in_args "TEST"
then
	${BUILD_DIR}/receiver_test_main &
fi

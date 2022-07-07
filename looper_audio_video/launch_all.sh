#!/bin/bash


# script gérant tout ; ex :
# launch_all.sh LOOPER VIDEO AUDIO --audio-device 1


# constantes --------------------------------------------------------------
SCRIPT_DIR=`dirname $0`
BUILD_DIR="${SCRIPT_DIR}/build"
ROOT_DATA="${SCRIPT_DIR}/data"
ROOT_WEB="${SCRIPT_DIR}/../web"
CHROME="/Applications/ADD_SOFTS/WEB/Google Chrome.app/Contents/MacOS/Google Chrome"


# variables ---------------------------------------------------------------
positional_args=()
audio_device="undefined"


# functions ---------------------------------------------------------------
# test arg in positional args 
in_args() {
	match="$1"
	for e in ${positional_args[*]}
	do
		[ "$e" = "$match" ] && return 0
	done
	return 1
}


kill_all() {
	pkill node
	pkill looper_gl_main
	#pkill video_sampler_main # non ! sinon pas d'enregistrement mp4 valide
	pkill audio_sampler_main
	pkill cv_in_main
	pkill cv_out_main
	pkill receiver_test_main
}


# init options -------------------------------------------------------------
while [[ $# -gt 0 ]]
do
  case $1 in
    --audio-device)
      audio_device="$2"
      shift # past argument
      shift # past value
      ;;
    *)
      positional_args+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done


# actions -------------------------------------------------------------------
# kill
if in_args "KILL"
then
	kill_all
	exit
fi

# list audio devices
if in_args "LIST_AUDIO"
then
	${BUILD_DIR}/list_audio_devices
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
	if [ "$audio_device" = "undefined" ]
	then
		echo 'préciser --audio-device'
		kill_all
		exit
	fi
	node ${ROOT_WEB}/cv_in_config_edit/server.js ${ROOT_DATA} &
	"${CHROME}" "http://localhost:3003" > /dev/null
	${BUILD_DIR}/cv_in_main $audio_device ${ROOT_DATA}/cv_in_configs/cv_in_config_01.json &
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
	if [ "$audio_device" = "undefined" ]
	then
		echo 'préciser --audio-device'
		kill_all
		exit
	fi
	node ${ROOT_WEB}/audio_config_edit/server.js ${ROOT_DATA} &
	"${CHROME}" "http://localhost:3001" > /dev/null
	${BUILD_DIR}/audio_sampler_main $audio_device ${ROOT_DATA}/audio_configs/audio_config_01.json &
fi

# émet des pulses sur les sorties de la carte son
if in_args "CV_OUT"
then
	if [ "$audio_device" = "undefined" ]
	then
		echo 'préciser --audio-device'
		kill_all
		exit
	fi
	node ${ROOT_WEB}/cv_out_config_edit/server.js ${ROOT_DATA} &
	"${CHROME}" "http://localhost:3002" > /dev/null
	${BUILD_DIR}/cv_out_main $audio_device ${ROOT_DATA}/cv_out_configs/cv_out_config_01.json &
fi

# test receiver
if in_args "TEST"
then
	${BUILD_DIR}/receiver_test_main &
fi

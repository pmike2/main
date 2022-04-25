#!/bin/bash

# on ne met pas de & à audio_sampler_main et receiver_test_main car leurs fins sont gérées par un ctrl-C

make
if [ $? -ne 0 ]
then
	exit 1
fi

./looper_main &
./video_sampler_main ../data/video_sampler_01.json &
./audio_sampler_main ../data/audio_sampler_01.json
#./receiver_test_main

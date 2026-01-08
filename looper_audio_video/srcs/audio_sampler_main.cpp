#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <signal.h>

#include "portaudio.h"
#ifdef WIN32
#include <windows.h>
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif

#include "json.hpp"

#include "constantes.h"
#include "audio_sampler.h"
#include "pa_utils.h"
#include "sio_util.h"


using namespace std;
using json = nlohmann::json;


// nombre de frames dans 1s d'audio
const uint SAMPLE_RATE= 44100;

// nombre de frames a traiter a chaque appel du callback
const uint FRAMES_PER_BUFFER= 64;


PaStream * stream;
AudioSampler * audio_sampler;
SocketIOUtil * sio_util;


// callback PortAudio
int pa_callback(const void * input, void * output, unsigned long frame_count, const PaStreamCallbackTimeInfo * time_info, PaStreamCallbackFlags status_flags, void * user_data) {
	AudioSampler * s= (AudioSampler *)user_data;
	//float * in= (float *)input;
	float * out= (float *)output;

	for (uint i=0; i<frame_count; ++i) {
		out[2* i+ 0]= 0.0f;
		out[2* i+ 1]= 0.0f;

		for (uint idx_track=0; idx_track<N_TRACKS; ++idx_track) {
			if (s->_track_samples[idx_track]->_playing) {
				
				if (DEBUG) {
					if (s->_track_samples[idx_track]->_frame_idx== 0) {
						time_type t= chrono::system_clock::now()- s->_debug_start_point;
						s->_debug[s->_compt_debug++]= t;
						if (s->_compt_debug>= N_DEBUG) {
							s->_compt_debug= 0;
						}
					}
				}

				key_type key= s->_track_samples[idx_track]->_info._key;
				amplitude_type amplitude= s->_track_samples[idx_track]->_info._amplitude;
				AudioSubSample * sub_sample= audio_sampler->get_subsample(key);
				if (!sub_sample) {
					continue;
				}

				AudioSample * audio_sample= sub_sample->_sample;

				if (audio_sample->_n_channels== 1) {
					out[2* i+ 0]+= audio_sample->_data[s->_track_samples[idx_track]->_frame_idx]* amplitude;
					out[2* i+ 1]+= audio_sample->_data[s->_track_samples[idx_track]->_frame_idx]* amplitude;
				}
				else {
					out[2* i+ 0]+= audio_sample->_data[2* s->_track_samples[idx_track]->_frame_idx+ 0]* amplitude;
					out[2* i+ 1]+= audio_sample->_data[2* s->_track_samples[idx_track]->_frame_idx+ 1]* amplitude;
				}
				
				s->_track_samples[idx_track]->_frame_idx++;
				if (s->_track_samples[idx_track]->_frame_idx>= audio_sample->_n_frames) {
					s->note_off(idx_track);
				}
			}
		}
	}

	return 0;
}


void clean() {
	pa_close(stream);
	delete sio_util;
	delete audio_sampler;
}


void interruption_handler(sig_atomic_t s) {
	clean();
	exit(1); 
}


void init(int idx_device_output, string json_path) {
	signal(SIGINT, interruption_handler);

	audio_sampler= new AudioSampler(json_path);
	stream= pa_init(-1, idx_device_output, SAMPLE_RATE, FRAMES_PER_BUFFER, pa_callback, audio_sampler);
	sio_util= new SocketIOUtil("http://127.0.0.1:3001", "server2client_config_changed", 2000);
}


void main_loop() {
	while (true) {
		if (sio_util->update()) {
			audio_sampler->load_json(json::parse(sio_util->_last_msg));
		}
		audio_sampler->update();
	}
}


int main(int argc, char **argv) {
	if (argc!= 3) {
		cout << "donner 1- idx_device_output; 2- le chemin d'un json en entrée\n";
		return 1;
	}
	init(atoi(argv[1]), string(argv[2]));
	main_loop();
	clean();

	return 0;
}

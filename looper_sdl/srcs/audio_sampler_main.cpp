#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "portaudio.h"
#ifdef WIN32
#include <windows.h>
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif

#include "audio_sampler.h"
#include "pa_utils.h"

using namespace std;


// nombre de samples dans 1s d'audio
const unsigned int SAMPLE_RATE= 44100;

// nombre de samples a traiter a chaque appel du callback portaudio
const unsigned int FRAMES_PER_BUFFER= 64;


PaStream * stream;
AudioSampler * audio_sampler;


// callback PortAudio
int pa_callback(const void * input, void * output, unsigned long frame_count, const PaStreamCallbackTimeInfo * time_info, PaStreamCallbackFlags status_flags, void * user_data) {
	AudioSampler * s= (AudioSampler *)user_data;
	//float * in= (float *)input;
	float * out= (float *)output;

	for (unsigned int i=0; i<frame_count; ++i) {
		out[2* i+ 0]= 0.0f;
		out[2* i+ 1]= 0.0f;

		for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
			if (s->_track_samples[idx_track]->_playing) {
				key_type key= s->_track_samples[idx_track]->_info._key;
				AudioSubSample * sub_sample= s->_map[key];
				//float amplitude= (float)(s->_track_samples[idx_track]->_info._amplitude)/ 128.0f;
				float amplitude= 1.0f;

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


void init() {
	audio_sampler= new AudioSampler("../data/audio_sampler_01.json");

	int idx_device_input= -1;
	int idx_device_output= 2;
	stream= pa_init(idx_device_input, idx_device_output, SAMPLE_RATE, FRAMES_PER_BUFFER, pa_callback, audio_sampler);
}


void main_loop() {
	while (true) {
		audio_sampler->update();
	}
}


void clean() {
	pa_close(stream);
	delete audio_sampler;
}


int main() {
	//list_devices(); return 0;
	init();
	main_loop();
	clean();

	return 0;
}

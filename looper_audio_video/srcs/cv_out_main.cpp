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

#include "cv_out.h"
#include "pa_utils.h"
#include "sio_util.h"

using namespace std;
using json = nlohmann::json;


// nombre de frames dans 1s d'audio
const unsigned int SAMPLE_RATE= 44100;

// nombre de frames a traiter a chaque appel du callback
const unsigned int FRAMES_PER_BUFFER= 64;

// nombre de frames mis à 1.0 pour déclencher le modulaire
const unsigned int N_FRAMES_PULSE= 100;



PaStream * stream;
CVOut * cv_out;
SocketIOUtil * sio_util;


// callback PortAudio
int pa_callback(const void * input, void * output, unsigned long frame_count, const PaStreamCallbackTimeInfo * time_info, PaStreamCallbackFlags status_flags, void * user_data) {
	CVOut * cvout= (CVOut *)user_data;
	//float * in= (float *)input;
	float * out= (float *)output;


	for (unsigned int i=0; i<frame_count; ++i) {
		
		for (unsigned int idx_channel=0; idx_channel<cvout->_n_output_channels; ++idx_channel) {
			out[i* cvout->_n_output_channels+ idx_channel]= 0.0f;
		}

		for (unsigned int idx_track=0; idx_track<N_TRACKS; ++idx_track) {
			if (cvout->_cv_tracks[idx_track]->_playing) {
				
				/*if (DEBUG) {
					if (s->_track_samples[idx_track]->_frame_idx== 0) {
						time_type t= chrono::system_clock::now()- s->_debug_start_point;
						s->_debug[s->_compt_debug++]= t;
						if (s->_compt_debug>= N_DEBUG) {
							s->_compt_debug= 0;
						}
					}
				}*/

				key_type key= cvout->_cv_tracks[idx_track]->_info._key;
				unsigned int idx_channel= cvout->get_idx_channel(key);
				if (idx_channel== 0) {
					continue;
				}

				out[i* cvout->_n_output_channels+ idx_channel]= 1.0;
				
				cvout->_cv_tracks[idx_track]->_frame_idx++;
				if (cvout->_cv_tracks[idx_track]->_frame_idx>= N_FRAMES_PULSE) {
					cvout->note_off(idx_track);
				}
			}
		}
	}

	return 0;
}


void clean() {
	pa_close(stream);
	delete sio_util;
	delete cv_out;
}


void interruption_handler(sig_atomic_t s) {
	clean();
	exit(1); 
}


void init(string json_path) {
	signal(SIGINT, interruption_handler);


	int idx_device_input= -1;
	int idx_device_output= 1;
	unsigned int n_output_channels= get_n_output_channels(idx_device_output);
	cv_out= new CVOut(json_path, n_output_channels);
	stream= pa_init(idx_device_input, idx_device_output, SAMPLE_RATE, FRAMES_PER_BUFFER, pa_callback, cv_out);
	sio_util= new SocketIOUtil("http://127.0.0.1:3002", "server2client_config_changed", 2000);
}


void main_loop() {
	while (true) {
		if (sio_util->update()) {
			cv_out->load_json(json::parse(sio_util->_last_msg));
			cout << cv_out;
		}
		cv_out->update();
	}
}


int main(int argc, char **argv) {
	//list_devices(); return 0;

	if (argc!= 2) {
		cout << "donner le chemin d'un json en entrée\n";
		return 1;
	}
	init(string(argv[1]));
	main_loop();
	clean();

	return 0;
}

/*

Truc important :
Dans Les préférences Mac / Sécurité il faut autoriser l'application terminal à utiliser le micro !!!

*/

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

#include "cv_in.h"
#include "pa_utils.h"
#include "sio_util.h"


using namespace std;
using json = nlohmann::json;


// nombre de frames dans 1s d'audio
const unsigned int SAMPLE_RATE= 44100;

// nombre de frames a traiter a chaque appel du callback
const unsigned int FRAMES_PER_BUFFER= 64;

// seuil de déclenchement d'une channel
const float THRESHOLD= 0.5f;

// nombre de frames durant lesquels un event sera émis
// afin que la video puisse récupérer l'événement il faut que :
// 1000 / 60 = 17ms (écart entre 2 updates de video) < N_FRAMES_PULSE * 1000 / SAMPLE_RATE
const unsigned int N_FRAMES_PULSE= 1024;


PaStream * stream;
CVIn * cv_in;
SocketIOUtil * sio_util;


// callback PortAudio
int pa_callback(const void * input, void * output, unsigned long frame_count, const PaStreamCallbackTimeInfo * time_info, PaStreamCallbackFlags status_flags, void * user_data) {
	CVIn * cvin= (CVIn *)user_data;
	float * in= (float *)input;
	//float * out= (float *)output;

	for (unsigned int idx_channel=0; idx_channel<cvin->_n_input_channels; ++idx_channel) {
		if (!cvin->_mapping.count(idx_channel)) {
			continue;
		}

		bool is_channel_active= false;
		for (unsigned int i=0; i<frame_count; ++i) {
			float x= in[i* cvin->_n_input_channels+ idx_channel];
			if (x> THRESHOLD) {
				is_channel_active= true;
				break;
			}
		}

		if (is_channel_active) {
			//cout << idx_channel << "\n";
			if (!cvin->is_active(idx_channel)) {
				cvin->set_active(idx_channel);
			}
		}

		if (cvin->is_active(idx_channel)) {
			cvin->_input_channels[idx_channel]._frame_idx+= FRAMES_PER_BUFFER;
			if (cvin->_input_channels[idx_channel]._frame_idx>= N_FRAMES_PULSE) {
				cvin->set_inactive(idx_channel);
			}
		}
	}

	return 0;
}


void clean() {
	pa_close(stream);
	delete sio_util;
	delete cv_in;
}


void interruption_handler(sig_atomic_t s) {
	clean();
	exit(1); 
}


void init(int idx_device_input, string json_path) {
	signal(SIGINT, interruption_handler);

	unsigned int n_input_channels= get_n_input_channels(idx_device_input);
	cv_in= new CVIn(json_path, n_input_channels);
	stream= pa_init(idx_device_input, -1, SAMPLE_RATE, FRAMES_PER_BUFFER, pa_callback, cv_in);
	sio_util= new SocketIOUtil("http://127.0.0.1:3003", "server2client_config_changed", 2000);
}


void main_loop() {
	while (true) {
		if (sio_util->update()) {
			cv_in->load_json(json::parse(sio_util->_last_msg));
			cout << *cv_in;
		}
		//cv_in->update();
	}
}


int main(int argc, char **argv) {
	if (argc!= 3) {
		cout << "donner 1- idx_device_input; 2- le chemin d'un json en entrée\n";
		return 1;
	}
	init(atoi(argv[1]), string(argv[2]));
	main_loop();
	clean();

	return 0;
}

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


PaStream * stream;
CVIn * cv_in;
SocketIOUtil * sio_util;


// callback PortAudio
int pa_callback(const void * input, void * output, unsigned long frame_count, const PaStreamCallbackTimeInfo * time_info, PaStreamCallbackFlags status_flags, void * user_data) {
	CVIn * cvin= (CVIn *)user_data;
	float * in= (float *)input;
	//float * out= (float *)output;


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


void init(string json_path) {
	signal(SIGINT, interruption_handler);

	cv_in= new CVIn(json_path);

	int idx_device_input= -1;
	int idx_device_output= 0;
	stream= pa_init(idx_device_input, idx_device_output, SAMPLE_RATE, FRAMES_PER_BUFFER, pa_callback, cv_in);
	sio_util= new SocketIOUtil("http://127.0.0.1:3001", "server2client_config_changed", 2000);
}


void main_loop() {
	while (true) {
		if (sio_util->update()) {
			cv_in->load_json(json::parse(sio_util->_last_msg));
		}
		//cv_in->update();
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

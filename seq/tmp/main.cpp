#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <csignal>
#include "RtMidi.h"

using namespace std;


RtMidiIn  * midi_in;
RtMidiOut * midi_out;
vector<unsigned char> message;
ofstream output_file;


void init_midi(bool verbose) {
	midi_in= new RtMidiIn();
	midi_out= new RtMidiOut();
	
	if (verbose) {
		unsigned int n_ports_in= midi_in->getPortCount();
		cout << "There are " << n_ports_in << " MIDI input ports" << "\n";
		for (unsigned int i=0; i<n_ports_in; i++) {
			cout << "  Input Port #" << i+ 1 << " : " << midi_in->getPortName(i) << "\n";
		}
		
		unsigned int n_ports_out= midi_out->getPortCount();
		cout << "There are " << n_ports_out << " MIDI output ports" << "\n";
		for (unsigned int i=0; i<n_ports_out; i++) {
			cout << "  Output Port #" << i+ 1 << " : " << midi_out->getPortName(i) << "\n";
		}
	}
}


void clean() {
	delete midi_in;
	delete midi_out;
}


void sigint_handler(int signal) {
	cout << "sigint_handler : " << signal << "\n";

	output_file.close();
	clean();

	exit(1);
}


int main () {
	srand((unsigned)time(0));
	signal(SIGINT, sigint_handler);

	init_midi(false);
	midi_out->openPort(0);

	cout.precision(17);

	message.push_back(144); message.push_back(36); message.push_back(90);
	
	output_file.open("test.txt");

	//chrono::duration<double> n_ms= chrono::milliseconds{1000};
	auto n_ms= chrono::milliseconds{100};
	/*int max_timeline= 1000;
	chrono::duration<double> timelines[max_timeline];
	for (unsigned int i=0; i<max_timeline; ++i) {
		timelines[i]= i* n_ms;
	}*/
	
	int timeline= 0;
	//int jitter= 0;
	//int max_jitter= 20;

	auto start_time= chrono::steady_clock::now();
	//cout << "Start Time: " << start_time.time_since_epoch().count() << "\n";
	while(true) {
		//chrono::duration<double> diff= chrono::steady_clock::now()- start_time;
		auto diff= chrono::steady_clock::now()- start_time;

		//double diff_d= diff.count();
		//output_file << diff_d << "\n";

		//if (diff_d> 1.0) {
		//if (diff>= chrono::milliseconds{1000}) {
		//if (diff>= timeline* n_ms+ jitter* chrono::milliseconds{1}) {
		if (diff>= timeline* n_ms) {
		//if (diff>= timelines[timeline]) {

			// Note On: 144, 64, 90
			//message[0]= 144; message[1]= 36; message[2]= 90;
			midi_out->sendMessage(& message);
			// Note Off: 128, 64, 40
			//message[0]= 128; message[1]= 36; message[2]= 40;
			//midi_out->sendMessage(& message);

			//output_file << diff.count() << "\n";
			timeline++;
			/*if (timeline>= max_timeline) {
				break;
			}*/

			//jitter= -max_jitter+ rand() % (max_jitter* 2);
			//jitter= -max_jitter+ (timeline % 2)* max_jitter* 2;
		}
	}
}

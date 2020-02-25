#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <ctime>
#include <vector>
#include <map>
#include <thread>

#include "sndfile.h"
#include "portaudio.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>


using namespace std;


struct Marker {
	unsigned int _begin;
	unsigned int _end;
	
	Marker(unsigned int begin, unsigned int end) : _begin(begin), _end(end) { }
};


// ---------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
	SNDFILE *file;
	SF_INFO info;
	
	file= sf_open(argv[1], SFM_READ, &info);
	
	cout << "nframes=" << info.frames << endl;
	cout << "nchannels=" << info.channels << endl;
	cout << "samplerate=" << info.samplerate << endl;
	
	float *buf= new float[info.frames* info.channels];
	sf_count_t num_read= sf_read_float(file, buf, info.frames* info.channels);
	
	cout << "num_read=" << num_read << endl;
	
	sf_close(file);
	
	vector<Marker> markers;
	unsigned int window_size= 32;
	bool is_silence= true;
	float tresh= 1e-3;
	unsigned int marker_start= 0;
	
	for (unsigned int i=0; i<num_read; i+= window_size) {
		if (is_silence) {
			for (unsigned int j=0; j<window_size; ++j) {
				float amplitude= abs(buf[i+ j]);
				if (amplitude> tresh) {
					marker_start= i+ j;
					is_silence= false;
					break;
				}
			}
		}
		else {
			unsigned int n_silent= 0;
			unsigned int last_no_silent= i;
			//float tmp[32];
			for (unsigned int j=0; j<window_size; ++j) {
				float amplitude= abs(buf[i+ j]);
				//tmp[j]= amplitude;
				if (amplitude< tresh)
					n_silent++;
				else
					last_no_silent= i+ j;
			}
			if (n_silent> 20) {
				is_silence= true;
				/*cout << last_no_silent- i << endl;
				cout << "----------------\n";
				for (int k=0; k<32; ++k)
					cout << tmp[k] << " ; ";
				cout << endl;*/
				markers.push_back(Marker(marker_start, last_no_silent));
			}
		}
	}
	
	cout << "markers size=" << markers.size() << endl;
	for (unsigned int idx_marker=0; idx_marker<markers.size(); ++idx_marker) {
		cout << fixed << setprecision(4) << (float)(markers[idx_marker]._begin)/ info.samplerate
			<< "\t\t" << (float)(markers[idx_marker]._end)  / info.samplerate
			<< "\t\tdiff=" << (float)(markers[idx_marker]._end- markers[idx_marker]._begin)/ info.samplerate;
		if (idx_marker> 0)
			cout << fixed << setprecision(4) << "\t\tecart=" << (float)(markers[idx_marker]._begin- markers[idx_marker- 1]._begin)/ info.samplerate;
		cout << endl;
	}
	
	delete[] buf;
	
	return 0;
}


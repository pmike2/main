#include <sstream>

#include "wav_in.h"

using json = nlohmann::json;


WavIn::WavIn() {

}


WavIn::WavIn(std::string json_path) {

}


WavIn::~WavIn() {

}


void WavIn::new_envelope(std::string s) {
	std::stringstream ss(s);
	std::string s_split;

	while(std::getline(ss, s_split, '|')) {
		json envelope = json::parse(s_split);
		unsigned int idx_freq_group = envelope["idx_freq_group"];
		std::vector<float> amplitudes;
		for (auto a : envelope["amplitudes"]) {
			amplitudes.push_back(a);
		}
	}
}

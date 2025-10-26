#include <sstream>
#include <fstream>

#include "wav_in.h"

using json = nlohmann::json;


bool EventCmp::operator()(const WavEvent lhs, const WavEvent rhs) const {
	return lhs._t < rhs._t;
}


WavIn::WavIn() {

}


WavIn::WavIn(std::string json_path) : _delta_event(0.0) {
	load_json(json_path);
}


WavIn::~WavIn() {

}


void WavIn::load_json(std::string json_path) {
	std::ifstream istr(json_path);
	json js;
	istr >> js;
	load_json(js);
}


void WavIn::load_json(json js) {
	_mapping.clear();
	for (auto & mapping : js.items()) {
		unsigned int idx_channel_in= (unsigned int)(stoi(mapping.key()));
		json val= mapping.value();
		for (auto & event : val) {
			unsigned int idx_track= event["track"].get<unsigned int>();
			key_type key= (key_type)(event["key"].get<std::string>().c_str()[0]);
			float amplitude_factor= event["amplitude_factor"].get<float>();

			WavEventConfig cv_event= {idx_track, key, amplitude_factor};
			_mapping[idx_channel_in].push_back(cv_event);
		}
	}
}


void WavIn::new_envelope(std::string s) {
	std::stringstream ss(s);
	std::string s_split;
	time_point now= std::chrono::system_clock::now();

	//std::cout << "new_envelope : " << s << "\n";

	while(std::getline(ss, s_split, '|')) {
		json js = json::parse(s_split);

		if (js["delta_offset"]!= nullptr) {
			double delta_offset = js["delta_offset"];
			double samplerate = js["samplerate"];
			_delta_event = delta_offset / samplerate;
		}
		else {
			unsigned int idx_freq_group = js["idx_freq_group"];
			if (!_mapping.count(idx_freq_group)) {
				continue;
			}
			for (auto config : _mapping[idx_freq_group]) {
				unsigned int idx_amplitude = 0;
				for (auto amplitude : js["amplitudes"]) {
					unsigned int ms = (unsigned int)((double)(idx_amplitude) * _delta_event* 1000.0);
					time_point t = now + std::chrono::milliseconds(ms);
					_events.insert({config._idx_track, config._key, (float)(amplitude) * config._amplitude_factor, t});
					idx_amplitude++;
				}
				unsigned int ms = (unsigned int)((double)(idx_amplitude) * _delta_event* 1000.0);
				time_point t = now + std::chrono::milliseconds(ms);
				_events.insert({config._idx_track, NULL_KEY, NULL_AMPLITUDE, t});
			}
		}
	}
}


void WavIn::main_loop() {
	if (_events.empty()) {
		return;
	}

	time_point now= std::chrono::system_clock::now();
	std::set<WavEvent>::iterator it = _events.begin();
	//std::cout << now.time_since_epoch().count() << " ; " << it->_t.time_since_epoch().count() << "\n";
	while (it->_t <= now) {
		std::cout << "WavIn::main_loop : t = " << it->_t.time_since_epoch().count() << " ; idx_track = " << it->_idx_track << " ; key = " << it->_key << " ; amplitude = " << it->_amplitude << "\n";
		_data[it->_idx_track]._key= it->_key;
		_data[it->_idx_track]._amplitude= it->_amplitude;
		it = _events.erase(it);
		if (it == _events.end()) {
			break;
		}
	}
}

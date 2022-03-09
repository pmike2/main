#include "sampler.h"

using namespace std;
using json = nlohmann::json;


Sample::Sample() {

}


Sample::~Sample() {

}


void Sample::read_from_file(std::string file_path) {

}


// ----------------------------------------------------------------------
SubSample::SubSample() {

}


SubSample::~SubSample() {
	
}


// ----------------------------------------------------------------------
SamplePlaying::SamplePlaying() {

}


SamplePlaying::~SamplePlaying() {

}


// ----------------------------------------------------------------------
Sampler::Sampler() {

}


Sampler::Sampler(nlohmann::json js) {

}


Sampler::~Sampler() {

}


void Sampler::on_new_data(sharedata_type data) {

}


SamplePlaying * Sampler::get_first_not_playing() {

}


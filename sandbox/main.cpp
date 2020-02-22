
#include <SDL2/SDL.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>
#include <cstdlib>

#include <glm/glm.hpp>


using namespace std;


struct SIRandomConfig {
	string _path;
	unsigned int _compt;
	float _size_factor_min_xy, _size_factor_max_xy;
	float _size_factor_min_z, _size_factor_max_z;
	bool _conserve_ratio;
	float _rand_angle_min, _rand_angle_max;
};


// pareil que SIRandomConfig ?
struct AIRandomConfig {
	string _path;
	unsigned int _compt;
	float _size_factor_min_xy, _size_factor_max_xy;
	float _size_factor_min_z, _size_factor_max_z;
	bool _conserve_ratio;
	float _rand_angle_min, _rand_angle_max;
};


struct WorldRandomConfig {
	vector<SIRandomConfig> _si_random_configs;
	vector<AIRandomConfig> _ai_random_configs;
};


// --------------------------------------------------------------------------------------------------------
const vector<SIRandomConfig> SI_RANDOM_CONFIG_1 {
	{"./data/tree.xml", 10000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/tree2.xml", 10000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/tree3.xml", 10000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/tree_dead.xml", 10000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/rocks.xml", 1000, 2.0f, 20.0f, 2.0f, 20.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/house.xml", 1000, 5.0f, 10.0f, 5.0f, 10.0f, true, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/grass.xml", 50000, 5.0f, 10.0f, 1.0f, 5.0f, false, 0.0f, (float)(M_PI)* 2.0f}
	//{"./data/castle.xml", 1, 10.0f, 10.0f, 100.0f, 100.0f, true, 0.0f, (float)(M_PI)* 2.0f}
};

const vector<SIRandomConfig> SI_RANDOM_CONFIG_2 {
	{"./data/tree.xml", 1000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
};

// ------------------------------------
const vector<AIRandomConfig> AI_RANDOM_CONFIG_1 {
	{"./data/hero.xml", 1, 1.0f, 1.0f, 1.0f, 1.0f, true, 0.0f, 0.0f},
	{"./data/gadjo.xml", 1000, 0.8f, 1.2f, 0.8f, 1.2f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/gadjo.xml", 1, 50.0f, 50.0f, 50.0f, 50.0f, true, 0.0f, (float)(M_PI)* 2.0f}
};

const vector<AIRandomConfig> AI_RANDOM_CONFIG_2 {
	{"./data/hero.xml", 1, 1.0f, 1.0f, 1.0f, 1.0f, true, 0.0f, 0.0f},
	{"./data/gadjo.xml", 100, 0.8f, 1.2f, 0.8f, 1.2f, false, 0.0f, (float)(M_PI)* 2.0f},
};

// ------------------------------------

// ------------------------------------
const WorldRandomConfig WORLD_RAND_CONFIG_1 {SI_RANDOM_CONFIG_1, AI_RANDOM_CONFIG_1};
const WorldRandomConfig WORLD_RAND_CONFIG_2 {SI_RANDOM_CONFIG_2, AI_RANDOM_CONFIG_2};


// ------------------------------------------------------------------------
int main(int argc, char * argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);

	return 0;
}

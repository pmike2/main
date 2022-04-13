#ifndef MPEG_H
#define MPEG_H

#include <vector>
#include <string>
#include <utility>

#include "geom_2d.h"


const unsigned int N_MAX_TEXTURES= 4;

class MPEG {
public:
	MPEG();
	MPEG(std::string mpeg_path);
	~MPEG();
	void * get_frame(unsigned int frame_idx);


	unsigned int _n_frames;
	unsigned int _fps;
	unsigned char * _data;
	unsigned int _width;
	unsigned int _height;
	unsigned int _frame_size;
};


class MPEGTextures {
public:
	MPEGTextures();
	MPEGTextures(std::vector<std::string> mpeg_paths, int loc, int base_index);
	~MPEGTextures();
	void prepare2draw();


	unsigned int _ids[N_MAX_TEXTURES];
	int _loc;
	int _base_index;
	int _indices[N_MAX_TEXTURES];
};


class TimeConfig {
public:
	TimeConfig();
	TimeConfig(std::vector<std::pair<float, float> > time_checkpoints);
	~TimeConfig();

	std::vector<std::pair<float, float> > _time_checkpoints;
};


class AlphaConfig {
public:
	AlphaConfig();
	AlphaConfig(std::vector<float *> points);
	~AlphaConfig();

	std::vector<Polygon2D> _polygons;
};


class MPEGReaders {
public:
	MPEGReaders();
	MPEGReaders(unsigned int n_readers,
		unsigned int alpha_width, unsigned int alpha_height, unsigned int alpha_texture_index, unsigned int alpha_loc, std::vector<AlphaConfig> alpha_configs,
		unsigned int time_width, unsigned int time_texture_index, unsigned int time_loc, std::vector<TimeConfig> time_configs,
		unsigned int index_time_texture_index, unsigned int index_time_loc
	);
	~MPEGReaders();
	void prepare2draw();
	void update_alpha(unsigned int depth);
	void next_index_time(unsigned int depth);


	unsigned int _alpha_width, _alpha_height, _alpha_depth;
	float * _alpha_data;
	unsigned int _alpha_id;
	int _alpha_loc;
	unsigned int _alpha_texture_index;
	std::vector<AlphaConfig> _alpha_configs;

	unsigned int _time_width, _time_height;
	float * _time_data;
	unsigned int _time_id;
	int _time_loc;
	unsigned int _time_texture_index;
	std::vector<TimeConfig> _time_configs;

	unsigned int _index_time_width;
	float * _index_time_data;
	unsigned int _index_time_id;
	int _index_time_loc;
	unsigned int _index_time_texture_index;
};


#endif

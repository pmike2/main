#ifndef MPEG_H
#define MPEG_H

#include <vector>
#include <string>
#include <utility>

#include "geom_2d.h"


const unsigned int N_MAX_MOVIES= 8;
const unsigned int N_READERS= 8;


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

/*
class MPEGTextures {
public:
	MPEGTextures();
	MPEGTextures(std::vector<std::string> mpeg_paths, int loc, int base_index);
	~MPEGTextures();
	void prepare2draw();


	unsigned int _ids[N_READERS];
	int _loc;
	int _base_index;
	int _indices[N_READERS];
};
*/

class TimeConfig {
public:
	TimeConfig();
	TimeConfig(std::vector<std::pair<float, float> > time_checkpoints, float speed);
	TimeConfig(const TimeConfig & time_config);
	~TimeConfig();

	std::vector<std::pair<float, float> > _time_checkpoints;
	float _speed;
};


class AlphaPolygon {
public:
	AlphaPolygon();
	AlphaPolygon(float * points, unsigned int n_points, float fadeout, float curve, float alpha_max);
	AlphaPolygon(const AlphaPolygon & alpha_polygon);
	~AlphaPolygon();

	Polygon2D _polygon;
	float _fadeout;
	float _curve;
	float _alpha_max;
};


class AlphaConfig {
public:
	AlphaConfig();
	AlphaConfig(std::vector<AlphaPolygon> polygons, float decrease_speed);
	AlphaConfig(const AlphaConfig & alpha_config);
	~AlphaConfig();

	std::vector<AlphaPolygon> _polygons;
	float _decrease_speed;
};


class ReaderConfig {
public:
	ReaderConfig();
	ReaderConfig(AlphaConfig alpha_config, TimeConfig time_config, std::string mpeg_path, unsigned int key);
	ReaderConfig(const ReaderConfig & reader_config);
	~ReaderConfig();

	AlphaConfig _alpha_config;
	TimeConfig _time_config;
	std::string _mpeg_path;
	unsigned int _key;
};


class GlobalConfig {
public:
	GlobalConfig();
	GlobalConfig(
		unsigned int alpha_width, unsigned int alpha_height, unsigned int alpha_depth,
		unsigned int time_width, unsigned int time_height,
		unsigned int index_time_width, unsigned int index_movie_width,
		std::vector<ReaderConfig> reader_configs);
	~GlobalConfig();

	unsigned int _alpha_width;
	unsigned int _alpha_height;
	unsigned int _alpha_depth;
	unsigned int _time_width;
	unsigned int _time_height;
	unsigned int _index_time_width;
	unsigned int _index_movie_width;
	std::vector<ReaderConfig> _reader_configs;
};


class MPEGReaders {
public:
	MPEGReaders();
	MPEGReaders(unsigned int base_index, unsigned int movie_loc, unsigned int alpha_loc,
		unsigned int time_loc, unsigned int index_time_loc, unsigned int index_movie_loc);
	~MPEGReaders();
	void set_config(GlobalConfig config);
	void load_json(std::string json_path);
	void randomize();
	void load_mpegs();
	void init_arrays();
	void compute_alpha_data0();
	void compute_time_data();
	void init_alpha_texture();
	void init_time_texture();
	void init_index_time_texture();
	void init_movie_time_texture();
	void prepare2draw();
	void update();
	void update_alpha_texture(unsigned int depth);
	void update_index_time_texture(unsigned int depth);
	void decrease_alpha(unsigned int depth);
	void next_index_time(unsigned int depth);
	void note_on(unsigned int key);
	void note_off(unsigned int key);


	GlobalConfig _config;

	unsigned int _movies_ids[N_MAX_MOVIES];
	int _movie_loc;
	unsigned int _movie_textures_indices[N_MAX_MOVIES];
	//std::vector<std::string> _movies_paths;

	//unsigned int _alpha_width, _alpha_height, _alpha_depth;
	float * _alpha_data;
	float * _alpha_data0;
	unsigned int _alpha_id;
	int _alpha_loc;
	unsigned int _alpha_texture_index;
	//std::vector<AlphaConfig> _alpha_configs;

	//unsigned int _time_width, _time_height;
	float * _time_data;
	unsigned int _time_id;
	int _time_loc;
	unsigned int _time_texture_index;
	//std::vector<TimeConfig> _time_configs;

	//unsigned int _index_time_width;
	float * _index_time_data;
	unsigned int _index_time_id;
	int _index_time_loc;
	unsigned int _index_time_texture_index;

	//unsigned int _index_movie_width;
	unsigned int * _index_movie_data;
	unsigned int _index_movie_id;
	int _index_movie_loc;
	unsigned int _index_movie_texture_index;

	//std::vector<bool> _note_ons;
};


#endif

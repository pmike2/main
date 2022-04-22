#ifndef MPEG_H
#define MPEG_H

#include <vector>
#include <string>
#include <utility>

#include "geom_2d.h"


const unsigned int N_MAX_MOVIES= 8;
const unsigned int N_TRACKS= 8;


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
	ReaderConfig(AlphaConfig alpha_config, TimeConfig time_config, std::string mpeg_path= "", unsigned int key= 0);
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
		unsigned int alpha_width, unsigned int alpha_height, unsigned int alpha_depth, unsigned int alpha_depth0,
		unsigned int time_width, unsigned int time_height, unsigned int time_height0,
		unsigned int index_time_width, 
		unsigned int index_movie_width, unsigned int index_movie_width0,
		unsigned int global_alpha_width,
		std::vector<ReaderConfig> reader_configs);
	GlobalConfig(const GlobalConfig & config);
	~GlobalConfig();
	GlobalConfig & operator=(const GlobalConfig & rhs);

	unsigned int _alpha_width;
	unsigned int _alpha_height;
	unsigned int _alpha_depth;
	unsigned int _alpha_depth0;
	unsigned int _time_width;
	unsigned int _time_height;
	unsigned int _time_height0;
	unsigned int _index_time_width;
	unsigned int _index_movie_width;
	unsigned int _index_movie_width0;
	unsigned int _global_alpha_width;
	std::vector<ReaderConfig> _reader_configs;
};


class MPEGReaders {
public:
	MPEGReaders();
	MPEGReaders(unsigned int base_index, int movie_loc, int alpha_loc, int time_loc, int index_time_loc,
		int index_movie_loc, int global_alpha_loc);
	~MPEGReaders();
	void set_config(GlobalConfig config);
	void load_json(std::string json_path);
	void randomize();
	void load_mpegs();
	void init_arrays();
	void compute_alpha_data0();
	void compute_time_data0();
	void compute_index_movie_data0();
	void init_alpha_texture();
	void init_time_texture();
	void init_index_time_texture();
	void init_index_movie_texture();
	void init_global_alpha_texture();
	void prepare2draw();
	void update();
	void update_alpha_texture(unsigned int idx_track);
	void update_time_texture(unsigned int idx_track);
	void update_index_time_texture(unsigned int idx_track);
	void update_index_movie_texture(unsigned int idx_track);
	void update_global_alpha_texture(unsigned int idx_track);
	void decrease_alpha(unsigned int idx_track);
	void next_index_time(unsigned int idx_track);
	void note_on(unsigned int idx_track, unsigned int key, float amplitude=1.0f);
	void note_off(unsigned int idx_track);
	int get_idx_reader(unsigned int key);
	std::vector<std::string> get_mpegs_paths();


	GlobalConfig _config;
	//std::vector<unsigned int> _notes_ons;
	int _index_reader[N_TRACKS];

	unsigned int _movies_ids[N_MAX_MOVIES];
	int _movie_loc;
	int _movie_textures_indices[N_MAX_MOVIES];

	float * _alpha_data;
	float * _alpha_data0;
	unsigned int _alpha_id;
	int _alpha_loc;
	unsigned int _alpha_texture_index;

	float * _time_data;
	float * _time_data0;
	unsigned int _time_id;
	int _time_loc;
	unsigned int _time_texture_index;

	float * _index_time_data;
	unsigned int _index_time_id;
	int _index_time_loc;
	unsigned int _index_time_texture_index;

	unsigned int * _index_movie_data;
	unsigned int * _index_movie_data0;
	unsigned int _index_movie_id;
	int _index_movie_loc;
	unsigned int _index_movie_texture_index;

	float * _global_alpha_data;
	unsigned int _global_alpha_id;
	int _global_alpha_loc;
	unsigned int _global_alpha_texture_index;
};


#endif

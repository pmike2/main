#ifndef MPEG_READER_H
#define MPEG_READER_H

#include <vector>
#include <string>
#include <utility>

#include "json.hpp"

#include "mpeg.h"
#include "geom_2d.h"
#include "constantes.h"


const unsigned int N_MAX_MOVIES= 8;


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


class ModifierConfig {
public:
	ModifierConfig();
	ModifierConfig(const ModifierConfig & modifier_config);
	~ModifierConfig();

	float _movie_mult[4];
	float _movie_add[2];
	float _movie_speed;
	float _alpha_mult[4];
	float _alpha_add[2];
	float _alpha_speed;
	float _time_mult;
	float _time_add;
	float _time_speed;
	unsigned int _idx_track;
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
		unsigned int modifier_width, unsigned int modifier_height,
		std::vector<ReaderConfig> reader_configs,
		std::vector<ModifierConfig> modifier_configs);
	GlobalConfig(const GlobalConfig & config);
	~GlobalConfig();
	std::vector<std::string> get_mpegs_paths();
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
	unsigned int _modifier_width;
	unsigned int _modifier_height;
	std::vector<ReaderConfig> _reader_configs;
	std::vector<ModifierConfig> _modifier_configs;
};


class MPEGReaders {
public:
	MPEGReaders();
	MPEGReaders(unsigned int base_index, int movie_loc, int alpha_loc, int time_loc, int index_time_loc,
		int index_movie_loc, int global_alpha_loc, int modifier_loc);
	~MPEGReaders();
	void set_config(GlobalConfig config);
	void load_json(std::string json_path);
	void load_json(nlohmann::json js);
	void randomize();
	void load_mpegs();
	void init_arrays();
	void compute_alpha_data0();
	void compute_time_data0();
	void compute_index_movie_data0();
	void compute_modifier_data();
	void init_alpha_texture();
	void init_time_texture();
	void init_index_time_texture();
	void init_index_movie_texture();
	void init_global_alpha_texture();
	void init_modifier_texture();
	void prepare2draw();
	void update();
	void update_alpha_texture(unsigned int idx_track);
	void update_time_texture(unsigned int idx_track);
	void update_index_time_texture(unsigned int idx_track);
	void update_index_movie_texture(unsigned int idx_track);
	void update_global_alpha_texture(unsigned int idx_track);
	void update_modifier_texture(unsigned int idx_track);
	void decrease_alpha(unsigned int idx_track);
	void next_index_time(unsigned int idx_track);
	void next_index_modifier(unsigned int idx_track);
	void note_on(unsigned int idx_track, unsigned int key, float amplitude=1.0f);
	void note_off(unsigned int idx_track);
	int get_idx_reader(unsigned int key);
	int get_idx_modifier(unsigned int idx_track);
	int get_idx_modifier_by_key(unsigned int key);


	GlobalConfig _config;
	int _index_reader[N_TRACKS];
	bool _note_on[N_TRACKS];

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

	float * _modifier_data;
	unsigned int _modifier_id;
	int _modifier_loc;
	unsigned int _modifier_texture_index;
};

#endif

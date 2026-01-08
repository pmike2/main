#ifndef MPEG_READER_H
#define MPEG_READER_H

#include <vector>
#include <string>
#include <utility>

#include "json.hpp"

#include "mpeg.h"
#include "geom_2d.h"
#include "constantes.h"
#include "typedefs.h"



const uint N_MAX_MOVIES= 8;


class TimeConfig {
public:
	TimeConfig();
	TimeConfig(std::vector<std::pair<float, float> > time_checkpoints, float speed);
	TimeConfig(const TimeConfig & time_config);
	~TimeConfig();
	friend std::ostream & operator << (std::ostream & os, const TimeConfig & t);


	std::vector<std::pair<float, float> > _time_checkpoints;
	float _speed;
};


class AlphaPolygon {
public:
	AlphaPolygon();
	AlphaPolygon(number * points, uint n_points, float fadeout, float curve, float alpha_max);
	AlphaPolygon(const AlphaPolygon & alpha_polygon);
	~AlphaPolygon();
	friend std::ostream & operator << (std::ostream & os, const AlphaPolygon & a);


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
	friend std::ostream & operator << (std::ostream & os, const AlphaConfig & a);


	std::vector<AlphaPolygon> _polygons;
	float _decrease_speed;
};


class ReaderConfig {
public:
	ReaderConfig();
	ReaderConfig(AlphaConfig alpha_config, TimeConfig time_config, std::string mpeg_path= "", uint key= 0);
	ReaderConfig(const ReaderConfig & reader_config);
	~ReaderConfig();
	friend std::ostream & operator << (std::ostream & os, const ReaderConfig & r);
	

	AlphaConfig _alpha_config;
	TimeConfig _time_config;
	std::string _mpeg_path;
	uint _key;
};


class ModifierConfig {
public:
	ModifierConfig();
	ModifierConfig(const ModifierConfig & modifier_config);
	~ModifierConfig();
	friend std::ostream & operator << (std::ostream & os, const ModifierConfig & m);

	float _movie_mult[4];
	float _movie_add[2];
	float _movie_speed;
	float _alpha_mult[4];
	float _alpha_add[2];
	float _alpha_speed;
	float _time_mult;
	float _time_add;
	float _time_speed;
	uint _idx_track;
	uint _key;
};


class GlobalConfig {
public:
	GlobalConfig();
	GlobalConfig(
		uint alpha_width, uint alpha_height, uint alpha_depth, uint alpha_depth0,
		uint time_width, uint time_height, uint time_height0,
		uint index_time_width, 
		uint index_movie_width, uint index_movie_width0,
		uint global_alpha_width,
		uint modifier_width, uint modifier_height,
		std::vector<ReaderConfig> reader_configs,
		std::vector<ModifierConfig> modifier_configs);
	GlobalConfig(const GlobalConfig & config);
	~GlobalConfig();
	std::vector<std::string> get_mpegs_paths();
	GlobalConfig & operator=(const GlobalConfig & rhs);
	friend std::ostream & operator << (std::ostream & os, const GlobalConfig & g);


	uint _alpha_width;
	uint _alpha_height;
	uint _alpha_depth;
	uint _alpha_depth0;
	uint _time_width;
	uint _time_height;
	uint _time_height0;
	uint _index_time_width;
	uint _index_movie_width;
	uint _index_movie_width0;
	uint _global_alpha_width;
	uint _modifier_width;
	uint _modifier_height;
	std::vector<ReaderConfig> _reader_configs;
	std::vector<ModifierConfig> _modifier_configs;
};


class MPEGReaders {
public:
	MPEGReaders();
	MPEGReaders(uint base_index, int movie_loc, int alpha_loc, int time_loc, int index_time_loc,
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
	void update_alpha_texture(uint idx_track);
	void update_time_texture(uint idx_track);
	void update_index_time_texture(uint idx_track);
	void update_index_movie_texture(uint idx_track);
	void update_global_alpha_texture(uint idx_track);
	void update_modifier_texture(uint idx_track);
	void decrease_alpha(uint idx_track);
	void next_index_time(uint idx_track);
	void next_index_modifier(uint idx_track);
	void note_on(uint idx_track, uint key, float amplitude=1.0f);
	void note_off(uint idx_track);
	int get_idx_reader(uint key);
	int get_idx_modifier(uint idx_track);
	int get_idx_modifier_by_key(uint key);


	GlobalConfig _config;
	int _index_reader[N_TRACKS];
	bool _note_on[N_TRACKS];

	uint _movies_ids[N_MAX_MOVIES];
	int _movie_loc;
	int _movie_textures_indices[N_MAX_MOVIES];

	float * _alpha_data;
	float * _alpha_data0;
	uint _alpha_id;
	int _alpha_loc;
	uint _alpha_texture_index;

	float * _time_data;
	float * _time_data0;
	uint _time_id;
	int _time_loc;
	uint _time_texture_index;

	float * _index_time_data;
	uint _index_time_id;
	int _index_time_loc;
	uint _index_time_texture_index;

	uint * _index_movie_data;
	uint * _index_movie_data0;
	uint _index_movie_id;
	int _index_movie_loc;
	uint _index_movie_texture_index;

	float * _global_alpha_data;
	uint _global_alpha_id;
	int _global_alpha_loc;
	uint _global_alpha_texture_index;

	float * _modifier_data;
	uint _modifier_id;
	int _modifier_loc;
	uint _modifier_texture_index;
};

#endif

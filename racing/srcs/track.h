#ifndef TRACK_H
#define TRACK_H

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "geom_2d.h"
#include "static_object.h"


class TrackTile {
public:
	TrackTile();
	TrackTile(std::string json_path);
	~TrackTile();
	void clear();
	friend std::ostream & operator << (std::ostream & os, const TrackTile & tile);


	std::string _json_path;
	std::vector<Polygon2D *> _obstacles;
};


class Track {
public:
	Track();
	~Track();
	void load_models();
	void clear();
	void load_json(std::string json_path);
	unsigned int coord2idx(unsigned int col_idx, unsigned int row_idx);
	std::pair<unsigned int, unsigned int> idx2coord(unsigned int idx);
	TrackTile * get_tile(unsigned int col_idx, unsigned int row_idx);
	void set_tile(TrackTile * model_tile, unsigned int col_idx, unsigned int row_idx);
	void set_tile(TrackTile * model_tile, unsigned int idx);
	void set_all(TrackTile * model_tile);
	void set_size(unsigned int width, unsigned int height, number cell_size);
	friend std::ostream & operator << (std::ostream & os, const Track & track);


	std::map<std::string, TrackTile * > _model_tiles;
	std::vector<TrackTile * > _tiles;
	unsigned int _width;
	unsigned int _height;
	number _cell_size;
};



#endif

#ifndef TRACK_H
#define TRACK_H

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "geom_2d.h"
#include "static_object.h"
#include "car.h"
#include "typedefs.h"

/*
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
*/

const number CELL_SIZE= 1.0;


class StaticObjectGrid {
public:
	StaticObjectGrid();
	StaticObjectGrid(pt_type origin);
	~StaticObjectGrid();
	void clear();
	unsigned int coord2idx(unsigned int col_idx, unsigned int row_idx);
	std::pair<unsigned int, unsigned int> idx2coord(unsigned int idx);
	std::pair<int, int> number2coord(number x, number y);
	StaticObject * get_tile(unsigned int col_idx, unsigned int row_idx);
	void set_tile(StaticObjectModel * model, unsigned int col_idx, unsigned int row_idx);
	void set_tile(StaticObjectModel * model, unsigned int idx);


	std::vector<StaticObject *> _objects;
	unsigned int _width;
	unsigned int _height;
	pt_type _origin;
};


class Track {
public:
	Track();
	~Track();
	void load_models();
	void load_json(std::string json_path);

	Car * get_hero();
	void all_collision();
	void anim(number dt);
	void set_tile(std::string model_name, unsigned int col_idx, unsigned int row_idx);
	void set_tile(std::string model_name, unsigned int idx);
	void set_all(std::string model_name, unsigned int width, unsigned int height);

	friend std::ostream & operator << (std::ostream & os, const Track & track);


	/*std::map<std::string, TrackTile * > _model_tiles;
	std::vector<TrackTile * > _tiles;*/

	std::map<std::string, StaticObjectModel *> _models;
	StaticObjectGrid * _grid;
	std::vector<StaticObject *> _floating_objects;
};



#endif

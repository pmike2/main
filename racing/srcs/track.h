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


const number CELL_SIZE= 1.0;


enum GridType {VERTICAL_GRID, HORIZONTAL_GRID};


class StaticObjectGrid {
public:
	StaticObjectGrid();
	~StaticObjectGrid();
	void clear();
	unsigned int coord2idx(unsigned int col_idx, unsigned int row_idx);
	std::pair<unsigned int, unsigned int> idx2coord(unsigned int idx);
	std::pair<int, int> number2coord(pt_type pos);
	pt_type coord2number(unsigned int col_idx, unsigned int row_idx);
	pt_type idx2number(unsigned int idx);
	StaticObject * get_tile(unsigned int col_idx, unsigned int row_idx);
	void push_tile(StaticObjectModel * model);
	void set_tile(StaticObjectModel * model, unsigned int col_idx, unsigned int row_idx);
	void set_tile(StaticObjectModel * model, unsigned int idx);
	void set_all(StaticObjectModel * model, unsigned int width, unsigned int height);


	std::vector<StaticObject *> _objects;
	unsigned int _width;
	unsigned int _height;
	//pt_type _origin;
	GridType _type;
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
	StaticObject * get_floating_object(pt_type pos);
	void delete_floating_object(StaticObject * obj);

	friend std::ostream & operator << (std::ostream & os, const Track & track);


	std::map<std::string, StaticObjectModel *> _models;
	StaticObjectGrid * _grid;
	std::vector<StaticObject *> _floating_objects;
};



#endif

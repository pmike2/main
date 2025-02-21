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


enum GridType {VERTICAL_GRID, HORIZONTAL_GRID};


class StaticObjectGrid {
public:
	StaticObjectGrid();
	StaticObjectGrid(number cell_size, GridType type);
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
	void add_row(StaticObjectModel * model);
	void add_col(StaticObjectModel * model);
	void drop_row();
	void drop_col();


	std::vector<StaticObject *> _objects;
	unsigned int _width;
	unsigned int _height;
	number _cell_size;
	GridType _type;
};


class Track {
public:
	Track();
	Track(number cell_size, unsigned int width, unsigned int height);
	~Track();
	void load_models();
	void load_json(std::string json_path);

	Car * get_hero();
	CheckPoint * get_start();
	unsigned int get_checkpoint_index(CheckPoint * checkpoint);
	void all_collision();
	void checkpoints();
	void anim(number dt);
	void set_tile(std::string model_name, unsigned int col_idx, unsigned int row_idx);
	void set_tile(std::string model_name, unsigned int idx);
	void set_all(std::string model_name, unsigned int width, unsigned int height);
	void add_row(std::string model_name);
	void add_col(std::string model_name);
	void drop_row();
	void drop_col();
	StaticObject * get_floating_object(pt_type pos);
	void delete_floating_object(StaticObject * obj);

	friend std::ostream & operator << (std::ostream & os, const Track & track);


	std::map<std::string, StaticObjectModel *> _models;
	StaticObjectGrid * _grid;
	std::vector<StaticObject *> _floating_objects;
	unsigned int _n_laps;
	CheckPoint * _start;
};



#endif

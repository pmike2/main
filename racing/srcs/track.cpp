#include <fstream>
#include <sstream>

#include "json.hpp"

#include "track.h"
#include "utile.h"


using json = nlohmann::json;



// StaticObjectGrid ------------------------------------------------------------
StaticObjectGrid::StaticObjectGrid() : _width(0), _height(0), _origin(pt_type(0.0)), _type(VERTICAL_GRID) {

}


/*StaticObjectGrid::StaticObjectGrid(pt_type origin) : _width(0), _height(0), _origin(origin) {

}*/


StaticObjectGrid::~StaticObjectGrid() {
	clear();
}


void StaticObjectGrid::clear() {
	for (auto obj : _objects) {
		delete obj;
	}
	_objects.clear();

	_width= _height= 0;
}


unsigned int StaticObjectGrid::coord2idx(unsigned int col_idx, unsigned int row_idx) {
	if (_type== VERTICAL_GRID) {
		return col_idx+ row_idx* _width;
	}
	else {
		return row_idx+ col_idx* _height;
	}
}


std::pair<unsigned int, unsigned int> StaticObjectGrid::idx2coord(unsigned int idx) {
	if (_type== VERTICAL_GRID) {
		return std::make_pair(idx % _width, idx / _width);
	}
	else {
		return std::make_pair(idx / _height, idx % _height);
	}
}


std::pair<int, int> StaticObjectGrid::number2coord(number x, number y) {
	int col_idx= int((x- _origin.x)/ CELL_SIZE);
	int row_idx= int((y- _origin.y)/ CELL_SIZE);
	if (col_idx>=0 && col_idx<_width && row_idx>=0 && row_idx< _height) {
		return std::make_pair(col_idx, row_idx);
	}
	return std::make_pair(-1, -1);
}


pt_type StaticObjectGrid::coord2number(unsigned int col_idx, unsigned int row_idx) {
	return pt_type(_origin.x+ number(col_idx)* CELL_SIZE, _origin.y+ number(row_idx)* CELL_SIZE);
}


pt_type StaticObjectGrid::idx2number(unsigned int idx) {
	std::pair<int, int> coord= idx2coord(idx);
	return coord2number(coord.first, coord.second);
}


StaticObject * StaticObjectGrid::get_tile(unsigned int col_idx, unsigned int row_idx) {
	if (row_idx> _height- 1) {
		std::cerr << "Track::get_tile : row_idx=" << row_idx << " >= _height=" << _height << "\n";
		return NULL;
	}
	if (col_idx> _width- 1) {
		std::cerr << "Track::get_tile : col_idx=" << col_idx << " >= _width=" << _width << "\n";
		return NULL;
	}
	return _objects[coord2idx(col_idx, row_idx)];
}


void StaticObjectGrid::push_tile(StaticObjectModel * model) {
	unsigned int new_size= _objects.size()+ 1;
	if (_type== VERTICAL_GRID) {
		_height= new_size/ _width;
		if (new_size % _width!= 0) {
			_height++;
		}
	}
	else if (_type== HORIZONTAL_GRID) {
		_width= new_size/ _height;
		if (new_size % _height!= 0) {
			_width++;
		}
	}
	_objects.push_back(new StaticObject(model, idx2number(_objects.size()), 0.0));
}


void StaticObjectGrid::set_tile(StaticObjectModel * model, unsigned int col_idx, unsigned int row_idx) {
	StaticObject * object= get_tile(col_idx, row_idx);
	object->_model= model;
	object->reinit(coord2number(col_idx, row_idx), 0.0);
}


void StaticObjectGrid::set_tile(StaticObjectModel * model, unsigned int idx) {
	std::pair<unsigned int, unsigned int> coord= idx2coord(idx);
	set_tile(model, coord.first, coord.second);
}


void StaticObjectGrid::set_all(StaticObjectModel * model, unsigned int width, unsigned int height) {
	clear();
	_width= width;
	_height= height;
	for (unsigned int row_idx=0; row_idx<height; ++row_idx) {
		for (unsigned int col_idx=0; col_idx<width; ++col_idx) {
			_objects.push_back(new StaticObject(model, coord2number(col_idx, row_idx), 0.0));
		}
	}
}


// Track -------------------------------------------------------------------------------------
Track::Track() {
	load_models();
	_grid= new StaticObjectGrid();
	set_all("empty", 10, 10);
}


Track::~Track() {
	delete _grid;
}


void Track::load_models() {

	std::vector<std::string> jsons_floating= list_files("../data/static_objects/floating_objects", "json");
	for (auto json_path : jsons_floating) {
		//std::cout << "chgmt floating : " << json_path << "\n";
		_models[basename(json_path)]= new StaticObjectModel(json_path);
	}

	std::vector<std::string> jsons_tiles= list_files("../data/static_objects/tiles", "json");
	for (auto json_path : jsons_tiles) {
		//std::cout << "chgmt tile : " << json_path << "\n";
		_models[basename(json_path)]= new StaticObjectModel(json_path);
	}

	std::vector<std::string> jsons_cars= list_files("../data/cars", "json");
	for (auto json_path : jsons_cars) {
		//std::cout << "chgmt car : " << json_path << "\n";
		_models[basename(json_path)]= new CarModel(json_path);
	}
}


void Track::load_json(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	set_all("empty", js["width"], js["height"]);
	unsigned int compt= 0;
	for (auto tilename : js["tiles"]) {
		set_tile(tilename, compt);
		compt++;
	}

	for (auto object : js["floating_objects"]) {
		std::string model_name= object["name"];
		pt_type position= pt_type(object["position"][0], object["position"][1]);
		number alpha= object["alpha"];
		StaticObject * floating_object= new StaticObject(_models[model_name], position, alpha);
		_floating_objects.push_back(floating_object);
	}
}


Car * Track::get_hero() {
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== HERO_CAR) {
			return (Car *)(obj);
		}
	}
	return NULL;
}


void Track::all_collision() {
	for (unsigned int idx_obj_1=0; idx_obj_1<_floating_objects.size()- 1; ++idx_obj_1) {
		for (unsigned int idx_obj_2=idx_obj_1+ 1; idx_obj_2<_floating_objects.size(); ++idx_obj_2) {
			collision(_floating_objects[idx_obj_1], _floating_objects[idx_obj_2]);
		}
	}
	for (auto object : _floating_objects) {
		for (auto tile : _grid->_objects) {
			collision(object, tile);
		}
	}
}


void Track::anim(number dt) {
	for (auto obj : _floating_objects) {
		obj->anim(dt);
	}

	all_collision();
}


void Track::set_tile(std::string model_name, unsigned int col_idx, unsigned int row_idx) {
	_grid->set_tile(_models[model_name], col_idx, row_idx);
}


void Track::set_tile(std::string model_name, unsigned int idx) {
	_grid->set_tile(_models[model_name], idx);
}


void Track::set_all(std::string model_name, unsigned int width, unsigned int height) {
	_grid->set_all(_models[model_name], width, height);
}


std::ostream & operator << (std::ostream & os, const Track & track) {
	os << "width=" << track._grid->_width << " ; height=" << track._grid->_height;
	return os;
}

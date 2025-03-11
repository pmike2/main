#include <fstream>
#include <sstream>

#include "json.hpp"

#include "track.h"
#include "utile.h"


using json = nlohmann::json;



// StaticObjectGrid ------------------------------------------------------------
StaticObjectGrid::StaticObjectGrid() : _width(0), _height(0), _type(VERTICAL_GRID) {

}


StaticObjectGrid::StaticObjectGrid(number cell_size, GridType type) : _width(0), _height(0), _cell_size(cell_size), _type(type) {

}


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


std::pair<int, int> StaticObjectGrid::number2coord(pt_type pos) {
	int col_idx= int(floor(pos.x/ _cell_size));
	int row_idx= int(floor(pos.y/ _cell_size));
	if (col_idx>=0 && col_idx<_width && row_idx>=0 && row_idx< _height) {
		return std::make_pair(col_idx, row_idx);
	}
	return std::make_pair(-1, -1);
}


// renvoie le centre de la cellule
pt_type StaticObjectGrid::coord2number(unsigned int col_idx, unsigned int row_idx) {
	return pt_type((number(col_idx)+ 0.5)* _cell_size, (number(row_idx)+ 0.5)* _cell_size);
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
	_objects.push_back(new StaticObject(model, idx2number(_objects.size()), 0.0, pt_type(_cell_size)));
}


void StaticObjectGrid::set_tile(StaticObjectModel * model, unsigned int col_idx, unsigned int row_idx) {
	StaticObject * object= get_tile(col_idx, row_idx);
	object->set_model(model);
	//std::cout << "in grid : " << *model << "\n";
	object->reinit(coord2number(col_idx, row_idx), 0.0, pt_type(_cell_size));
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
			_objects.push_back(new StaticObject(model, coord2number(col_idx, row_idx), 0.0, pt_type(_cell_size)));
		}
	}
}


void StaticObjectGrid::add_row(StaticObjectModel * model) {
	for (unsigned int col_idx=0; col_idx<_width; ++col_idx) {
		StaticObject * obj= new StaticObject(model, coord2number(col_idx, _height), 0.0, pt_type(_cell_size));
		_objects.push_back(obj);
	}
	_height++;
}


void StaticObjectGrid::add_col(StaticObjectModel * model) {
	for (unsigned int row_idx=_height; row_idx>0; --row_idx) {
		StaticObject * obj= new StaticObject(model, coord2number(_width, row_idx), 0.0, pt_type(_cell_size));
		_objects.insert(_objects.begin()+ row_idx* _width, obj);
	}
	_width++;
}


void StaticObjectGrid::drop_row() {
	if (_height== 0) {
		return;
	}
	_objects.erase(_objects.begin()+ _width* (_height- 1), _objects.end());
	_height--;
}


void StaticObjectGrid::drop_col() {
	if (_width== 0) {
		return;
	}
	for (unsigned int row_idx=_height; row_idx>0; --row_idx) {
		_objects.erase(_objects.begin()+ row_idx* _width- 1);
	}
	_width--;
}


// Track -------------------------------------------------------------------------------------
Track::Track() : _start(NULL), _n_laps(0) {
	load_models();
	_grid= new StaticObjectGrid(DEFAULT_CELL_SIZE, VERTICAL_GRID);
}


/*Track::Track(number cell_size, unsigned int width, unsigned int height) : _start(NULL), _n_laps(0) {
	load_models();
	_grid= new StaticObjectGrid(cell_size, VERTICAL_GRID);
	set_all("empty", width, height);
}*/


Track::~Track() {
	delete _grid;
	for (auto obj : _floating_objects) {
		delete obj;
	}
	_floating_objects.clear();
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

	_grid->_cell_size= js["cell_size"];
	set_all("empty", js["width"], js["height"]);
	unsigned int compt= 0;
	for (auto tilename : js["tiles"]) {
		set_tile(tilename, compt);
		compt++;
	}
	_n_laps= js["n_laps"];

	for (auto obj : _floating_objects) {
		delete obj;
	}
	_floating_objects.clear();

	std::vector<std::pair<CheckPoint *, unsigned int> > checkpoints;

	for (auto object : js["floating_objects"]) {
		std::string model_name= object["name"];
		pt_type position= pt_type(object["position"][0], object["position"][1]);
		number alpha= object["alpha"];
		pt_type scale= pt_type(object["scale"][0], object["scale"][1]);
		if (_models[model_name]->_type== HERO_CAR || _models[model_name]->_type== ENNEMY_CAR) {
			Car * car= new Car((CarModel *)(_models[model_name]), position, alpha, scale);
			_floating_objects.push_back(car);
		}
		else if (_models[model_name]->_type== CHECKPOINT || _models[model_name]->_type== START) {
			CheckPoint * checkpoint= new CheckPoint(_models[model_name], position, alpha, scale);
			if (_models[model_name]->_type== START) {
				_start= checkpoint;
			}
			checkpoints.push_back(std::make_pair(checkpoint, object["idx_checkpoint"]));
			_floating_objects.push_back(checkpoint);
		}
		else {
			StaticObject * floating_object= new StaticObject(_models[model_name], position, alpha, scale);
			_floating_objects.push_back(floating_object);
		}
	}

	std::sort(checkpoints.begin(), checkpoints.end(), [](std::pair<CheckPoint *, unsigned int> a, std::pair<CheckPoint *, unsigned int> b) {
		return a.second< b.second; 
	});
	for (unsigned int i=0; i<checkpoints.size(); ++i) {
		if (i< checkpoints.size()- 1) {
			checkpoints[i].first->_next= checkpoints[i+ 1].first;
			checkpoints[i+ 1].first->_previous= checkpoints[i].first;
		}
		else {
			checkpoints[i].first->_next= checkpoints[0].first;
			checkpoints[0].first->_previous= checkpoints[i].first;
		}
	}

	for (auto obj : _floating_objects) {
		if (obj->_model->_type== HERO_CAR || obj->_model->_type== ENNEMY_CAR) {
			Car * car= (Car *)(obj);
			car->_next_checkpoint= _start;
		}
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


std::vector<Car *> Track::get_sorted_cars() {
	std::vector<Car *> sorted_cars;
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== HERO_CAR || obj->_model->_type== ENNEMY_CAR) {
			sorted_cars.push_back((Car *)(obj));
		}
	}
	
	std::sort(sorted_cars.begin(), sorted_cars.end(), [this](Car * a, Car * b) {
		if (a->_n_laps< b->_n_laps) {
			return 0;
		}
		else if (a->_n_laps> b->_n_laps) {
			return 1;
		}
		else {
			unsigned int idx_a= get_checkpoint_index(a->_next_checkpoint->_previous);
			unsigned int idx_b= get_checkpoint_index(b->_next_checkpoint->_previous);
			if (idx_a< idx_b) {
				return 0;
			}
			else if (idx_a> idx_b) {
				return 1;
			}
			else {
				number dist_a= (a->_com.x- a->_next_checkpoint->_com.x)* (a->_com.x- a->_next_checkpoint->_com.x)+ (a->_com.y- a->_next_checkpoint->_com.y)* (a->_com.y- a->_next_checkpoint->_com.y);
				number dist_b= (b->_com.x- b->_next_checkpoint->_com.x)* (b->_com.x- b->_next_checkpoint->_com.x)+ (b->_com.y- b->_next_checkpoint->_com.y)* (b->_com.y- b->_next_checkpoint->_com.y);
				if (dist_a> dist_b) {
					return 0;
				}
				else {
					return 1;
				}
			}
		}
	});
	
	return sorted_cars;
}


unsigned int Track::get_checkpoint_index(CheckPoint * checkpoint) {
	unsigned int idx= 0;
	CheckPoint * start= _start;
	while (checkpoint!= start) {
		idx++;
		start= start->_next;
	}
	return idx;
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


void Track::checkpoints() {
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== HERO_CAR || obj->_model->_type== ENNEMY_CAR) {
			Car * car= (Car *)(obj);
			if (!aabb_intersects_aabb(car->_bbox->_aabb, car->_next_checkpoint->_bbox->_aabb)) {
				continue;
			}
			pt_type axis(0.0, 0.0);
			number overlap= 0.0;
			unsigned int idx_pt= 0;
			bool is_pt_in_poly1= false;
			bool is_inter= poly_intersects_poly(car->_footprint, car->_next_checkpoint->_footprint, &axis, &overlap, &idx_pt, &is_pt_in_poly1);
			if (is_inter) {
				if (car->_next_checkpoint== _start) {
					car->_n_laps++;
					if (car->_n_laps== _n_laps) {
						if (car->_model->_type== HERO_CAR) {
							std::cout << "you win\n";
						}
						else {
							std::cout << "you lose\n";
						}
					}
				}
				car->_next_checkpoint= car->_next_checkpoint->_next;
			}
		}
	}
}


void Track::checkpoint_ia(Car * car) {
	CheckPoint * checkpoint= car->_next_checkpoint;
	pt_type direction(checkpoint->_com- car->_com);
	number dist= glm::length(direction);
	number scalprod= scal(car->_forward, direction/ dist);
	number sign= 1.0;
	CarModel * model= car->get_model();

	car->_thrust= 1.0+ 0.9* dist;
	if (car->_thrust< -1.0* model->_max_brake) {
		car->_thrust= -1.0* model->_max_brake;
	}
	if (car->_thrust> model->_max_thrust) {
		car->_thrust= model->_max_thrust;
	}

	if (scalprod< -0.8 && abs(car->_wheel)> 0.5) {

	}
	else {
		// wheel est >0 à gauche, <0 à droite
		if (is_left(car->_com, car->_forward, checkpoint->_com)) {
			sign= 1.0;
		}
		else {
			sign= -1.0;
		}
		car->_wheel= 1.0* sign* (1.0- scalprod);
		if (car->_wheel> model->_max_wheel) {
			car->_wheel= model->_max_wheel;
		}
		if (car->_wheel< -1.0* model->_max_wheel) {
			car->_wheel= -1.0* model->_max_wheel;
		}
	}

	/*std::cout << "car=" << car->_com.x << " ; " << car->_com.y;
	std::cout << " ; checkpoint=" << checkpoint->_com.x << " ; " << checkpoint->_com.y;
	std::cout << " ; direction=" << direction.x << " ; " << direction.y;
	std::cout << " ; sign=" << sign;
	std::cout << " ; dist=" << dist;
	std::cout << " ; scalprod=" << scalprod;
	std::cout << " ; thrust=" << car->_thrust;
	std::cout << " ; wheel=" << car->_wheel;
	std::cout << "\n";*/
}


void Track::anim(number dt, bool key_left, bool key_right, bool key_up, bool key_down, bool is_joystick, bool joystick_a, bool joystick_b, glm::vec2 joystick) {
	if (is_joystick) {
		get_hero()->preanim_joystick(joystick_a, joystick_b, joystick);
	}
	else {
		get_hero()->preanim_keys(key_left, key_right, key_down, key_up);
	}

	for (auto obj : _floating_objects) {
		if (obj->_model->_type== ENNEMY_CAR) {
			Car * car= (Car *)(obj);
			//car->random_ia();
			checkpoint_ia(car);
		}

		if (obj->_model->_type== HERO_CAR || obj->_model->_type== ENNEMY_CAR) {
			Car * car= (Car *)(obj);
			car->anim(dt);
		}
		else {
			obj->anim(dt);
		}
	}
	all_collision();
	checkpoints();
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


void Track::add_row(std::string model_name) {
	_grid->add_row(_models[model_name]);
}


void Track::add_col(std::string model_name) {
	_grid->add_col(_models[model_name]);
}


void Track::drop_row() {
	_grid->drop_row();
}


void Track::drop_col() {
	_grid->drop_col();
}


StaticObject * Track::get_floating_object(pt_type pos) {
	for (auto obj : _floating_objects) {
		if (is_pt_inside_poly(pos, obj->_footprint)) {
			return obj;
		}
	}
	return NULL;
}


void Track::delete_floating_object(StaticObject * obj) {
	_floating_objects.erase(find(_floating_objects.begin(), _floating_objects.end(), obj));
	delete obj;
}


std::ostream & operator << (std::ostream & os, const Track & track) {
	os << "width=" << track._grid->_width << " ; height=" << track._grid->_height;
	return os;
}

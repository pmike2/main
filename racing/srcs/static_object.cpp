#include <fstream>
#include <sstream>

#include "json.hpp"

#include "static_object.h"
#include "geom_2d.h"
#include "utile.h"


using json = nlohmann::json;


// StaticObjectModel --------------------------------------------------------------
StaticObjectModel::StaticObjectModel() {

}


StaticObjectModel::StaticObjectModel(std::string json_path) : _texture_idx(0) {
	load(json_path);
}


StaticObjectModel::~StaticObjectModel() {

}


void StaticObjectModel::load(std::string json_path) {
	_json_path= json_path;
	_name= basename(_json_path);

	std::ifstream ifs(_json_path);
	json js= json::parse(ifs);
	ifs.close();

	// type d'objet
	if (js["type"]== "obstacle_tile") {
		_type= OBSTACLE_TILE;
	}
	else if (js["type"]== "obstacle_floating") {
		_type= OBSTACLE_FLOATING;
	}
	else if (js["type"]== "hero_car") {
		_type= HERO_CAR;
	}
	else if (js["type"]== "ennemy_car") {
		_type= ENNEMY_CAR;
	}
	else if (js["type"]== "checkpoint") {
		_type= CHECKPOINT;
	}
	else if (js["type"]== "start") {
		_type= START;
	}
	else if (js["type"]== "material_tile") {
		_type= MATERIAL_TILE;
	}
	else if (js["type"]== "material_floating") {
		_type= MATERIAL_FLOATING;
	}
	else if (js["type"]== "tire_tracks") {
		_type= TIRE_TRACKS;
	}
	else if (js["type"]== "repair") {
		_type= REPAIR;
	}
	else {
		std::cerr << "Type " << js["type"] << " non supporté\n";
	}

	// empreinte au sol
	_footprint= new Polygon2D();
	std::vector<pt_type> pts;
	if (js["footprint"]!= nullptr) {
		for (auto coord : js["footprint"]) {
			pt_type pt(coord[0], coord[1]);
			pts.push_back(pt);
		}
	}
	else {
		// emprise totale
		pts.push_back(pt_type(-0.5, -0.5));
		pts.push_back(pt_type(0.5, -0.5));
		pts.push_back(pt_type(0.5, 0.5));
		pts.push_back(pt_type(-0.5, 0.5));
	}
	_footprint->set_points(pts);
	_footprint->update_all();

	// vecteur centre de masse -> centre bbox
	_com2bbox_center= -1.0* _footprint->_centroid;
	
	if (_type== OBSTACLE_TILE) {
		_fixed= true;
		_solid= true;
		_restitution= js["restitution"];
		_mass= _linear_friction= _angular_friction= 0.0;
	}
	
	else if (_type== OBSTACLE_FLOATING) {
		// on met le com (center of mass) à 0,0 afin de faciliter les rotations autour du com
		_footprint->centroid2zero();
		_footprint->update_all();
		_fixed= js["fixed"];
		_solid= true;
		_restitution= js["restitution"];
		if (_fixed) {
			_mass= _linear_friction= _angular_friction= 0.0;
		}
		else {
			_mass= js["mass"];
			_linear_friction= js["linear_friction"];
			_angular_friction= js["angular_friction"];
		}
	}
	
	else if (_type== HERO_CAR || _type== ENNEMY_CAR) {
		// on met le com (center of mass) à 0,0 afin de faciliter les rotations autour du com
		_footprint->centroid2zero();
		_footprint->update_all();
		_fixed= false;
		_solid= true;
		_mass= js["mass"];
		_angular_friction= js["angular_friction"];
		_restitution= js["restitution"];
	}

	else if (_type== CHECKPOINT || _type== START || _type== TIRE_TRACKS || _type== REPAIR) {
		_fixed= true;
		_solid= false;
		_mass= _linear_friction= _angular_friction= _restitution= 0.0;
	}

	else if (_type== MATERIAL_FLOATING || _type== MATERIAL_TILE) {
		_fixed= true;
		_solid= false;
		_mass= _restitution= 0.0;
		// dans ce contexte ce sont des facteurs multiplicatifs qui vont affecter les cars
		_linear_friction= js["linear_friction"];
		_angular_friction= js["angular_friction"];
	}
}


std::ostream & operator << (std::ostream & os, const StaticObjectModel & model) {
	os << "json_path=" << model._json_path;
	os << " ; type=";
	if (model._type== OBSTACLE_TILE){
		os << "OBSTACLE_TILE";
	}
	else if (model._type== OBSTACLE_FLOATING){
		os << "OBSTACLE_FLOATING";
	}
	else if (model._type== HERO_CAR){
		os << "HERO_CAR";
	}
	else if (model._type== ENNEMY_CAR){
		os << "ENNEMY_CAR";
	}
	else if (model._type== CHECKPOINT){
		os << "CHECKPOINT";
	}
	else if (model._type== START){
		os << "START";
	}
	else if (model._type== MATERIAL_FLOATING){
		os << "MATERIAL_FLOATING";
	}
	else if (model._type== MATERIAL_TILE){
		os << "MATERIAL_TILE";
	}
	else if (model._type== TIRE_TRACKS){
		os << "TIRE_TRACKS";
	}
	os << " ; footprint=" << *model._footprint;
	os << " ; mass=" << model._mass << " ; fixed=" << model._fixed << " ; solid=" << model._solid;
	return os;
}


// StaticObject --------------------------------------------------------------------
StaticObject::StaticObject() {

}


StaticObject::StaticObject(StaticObjectModel * model, pt_type position, number alpha, pt_type scale) {
	_bbox= new BBox_2D(pt_type(0.0), pt_type(0.0));
	_footprint= new Polygon2D();
	set_model(model);
	reinit(position, alpha, scale);

	if (model->_type== OBSTACLE_TILE || model->_type== MATERIAL_TILE) {
		_z= -100.0f;
	}
	else if (model->_type== START || model->_type== CHECKPOINT || model->_type== MATERIAL_FLOATING || model->_type== REPAIR) {
		_z= -90.0f;
	}
	else if (model->_type== TIRE_TRACKS) {
		_z= -80.0f;
	}
	else if (model->_type== OBSTACLE_FLOATING || model->_type== HERO_CAR || model->_type== ENNEMY_CAR) {
		_z= -70.0f;
	}
	else {
		std::cerr << "Type " << model->_type << " non supporté\n";
	}
}


StaticObject::~StaticObject() {
	delete _footprint;
}


void StaticObject::set_model(StaticObjectModel * model) {
	_model= model;
	_footprint->set_points(_model->_footprint->_pts);
	_footprint->triangulate(); // on a besoin de trianguler pour l'affichage par des triangles du footprint
	_footprint->update_all();
}


void StaticObject::reinit(pt_type position, number alpha, pt_type scale) {
	_com= position;
	_alpha= alpha;
	_scale= scale;

	_velocity= pt_type(0.0);
	_acceleration= pt_type(0.0);
	_force= pt_type(0.0);
	_angular_velocity= 0.0;
	_angular_acceleration= 0.0;
	_torque= 0.0;

	for (int i=0; i<9; ++i) {
		_bumps[i]= 0.0;
	}

	update();
}


void StaticObject::update() {
	// maj du footprint
	_footprint->set_points(_model->_footprint->_pts);
	_footprint->scale(_scale);
	_footprint->rotate(pt_type(0.0), _alpha);
	_footprint->translate(_com);
	_footprint->update_all(); // peut-être pas besoin de tout updater

	// maj de la bbox
	_bbox->_alpha= _alpha;
	if (_model->_type== OBSTACLE_TILE || _model->_type== MATERIAL_TILE) {
		_bbox->_center= _com;
		_bbox->_half_size= 0.5* _scale;
	}
	else {
		_bbox->_center= _com+ rot(pt_type(_model->_com2bbox_center.x* _scale.x, _model->_com2bbox_center.y* _scale.y), _alpha);
		_bbox->_half_size= 0.5* _scale;
	}
	_bbox->update();

	// maj masse et inertie
	_mass= _model->_mass* _scale.x* _scale.y;
	// !!!
	// obligé de rajouter * facteur sinon tout pète lors des collisions
	// !!!
	_inertia= _mass* _footprint->_inertia* INERTIA_FACTOR;
}


void StaticObject::anim(number anim_dt) {
	if (_model->_fixed) {
		return;
	}

	// calcul force
	_force= pt_type(0.0);
	_force-= _model->_linear_friction* _velocity; // friction
	
	// force -> acceleration -> vitesse -> position
	_acceleration= _force/ _mass;
	_velocity+= _acceleration* anim_dt;
	_com+= _velocity* anim_dt;

	// calcul torque == équivalent force pour les rotations
	_torque= 0.0;
	_torque-= _model->_angular_friction* _angular_velocity; // friction
	
	// torque -> acc angulaire -> vitesse angulaire -> angle
	_angular_acceleration= _torque/ _inertia;
	_angular_velocity+= _angular_acceleration* anim_dt;
	_alpha+= _angular_velocity* anim_dt;

	// on veut un angle toujours compris entre 0 et 2pi
	while (_alpha> M_PI* 2.0) {
		_alpha-= M_PI* 2.0;
	}
	while (_alpha< 0.0) {
		_alpha+= M_PI* 2.0;
	}

	update();
}


std::ostream & operator << (std::ostream & os, const StaticObject & obj) {
	os << "model = " << obj._model->_json_path;
	os << " ; bbox=[" << *obj._bbox << "] ; ";
	os << " ; footprint=[" << *obj._footprint << "] ; ";
	os << " ; mass=" << obj._mass << " ; inertia=" << obj._inertia;
	return os;
}


// CheckPoint -----------------------------------------------
CheckPoint::CheckPoint() {

}


CheckPoint::CheckPoint(StaticObjectModel * model, pt_type position, number alpha, pt_type scale) :
	StaticObject(model, position, alpha, scale), _next(NULL), _previous(NULL)
{

}


CheckPoint::~CheckPoint() {

}


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
	for (int row_idx=_height- 1; row_idx>=0; --row_idx) {
		StaticObject * obj= new StaticObject(model, coord2number(_width, row_idx), 0.0, pt_type(_cell_size));
		_objects.insert(_objects.begin()+ row_idx* _width+ _width, obj);
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


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


StaticObjectModel::StaticObjectModel(std::string json_path) : _json_path(json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	if (js["type"]== "obstacle") {
		_type= OBSTACLE;
	}
	else if (js["type"]== "hero_car") {
		_type= HERO_CAR;
	}
	else if (js["type"]== "ennemy_car") {
		_type= ENNEMY_CAR;
	}
	else {
		std::cerr << "Type " << js["type"] << " non supporté\n";
	}

	if (js["type"]== "obstacle") {
		_fixed= js["fixed"];
		if (_fixed) {
			_com2bbox_center= pt_type(0.0);
			_halfsize= pt_type(0.0);
			_mass= _linear_friction= _angular_friction= 0.0;
		}
		else {
			_com2bbox_center= pt_type(js["com2bbox_center"][0], js["com2bbox_center"][1]);
			_halfsize= pt_type(js["halfsize"][0], js["halfsize"][1]);
			_mass= js["mass"];
			_linear_friction= js["linear_friction"];
			_angular_friction= js["angular_friction"];
			_footprint= new Polygon2D();
			std::vector<pt_type> pts;
			for (auto coord : js["footprint"]) {
				pt_type pt(coord[0], coord[1]);
				pts.push_back(pt);
			}
			_footprint->set_points(pts);
		}
	}
	else {
		_fixed= false;
		_com2bbox_center= pt_type(js["com2bbox_center"][0], js["com2bbox_center"][1]);
		_halfsize= pt_type(js["halfsize"][0], js["halfsize"][1]);
		_mass= js["mass"];
		_angular_friction= js["angular_friction"];
		_footprint= new Polygon2D();
		std::vector<pt_type> pts;
		for (auto coord : js["footprint"]) {
			pt_type pt(coord[0], coord[1]);
			pts.push_back(pt);
		}
		_footprint->set_points(pts);
	}
	
	// https://en.wikipedia.org/wiki/List_of_moments_of_inertia
	// normalement c'est / 12.0 mais tout explose...
	_inertia= _mass* (4.0* _halfsize.x* _halfsize.x+ 4.0* _halfsize.y* _halfsize.y)/ 3.0;
}


StaticObjectModel::~StaticObjectModel() {

}


// StaticObject --------------------------------------------------------------------
StaticObject::StaticObject() {

}


StaticObject::StaticObject(StaticObjectModel * model) : 
	_model(model), _com(pt_type(0.0)), _alpha(0.0), _velocity(pt_type(0.0)), _acceleration(pt_type(0.0)), _force(pt_type(0.0)),
	_angular_velocity(0.0), _angular_acceleration(0.0), _torque(0.0)
{
	_bbox= new BBox_2D(pt_type(0.0), pt_type(0.0));
}


StaticObject::StaticObject(StaticObjectModel * model, pt_type position, number alpha) : _model(model) {
	_bbox= new BBox_2D(position, model->_halfsize);
	reinit(position, alpha);
}


StaticObject::~StaticObject() {

}


void StaticObject::reinit(pt_type position, number alpha) {
	_com= position;
	_alpha= alpha;

	_velocity= pt_type(0.0);
	_acceleration= pt_type(0.0);
	_force= pt_type(0.0);
	_angular_velocity= 0.0;
	_angular_acceleration= 0.0;
	_torque= 0.0;

	update_direction();
	update_bbox();
}


void StaticObject::update_direction() {
	_com2bbox_center= rot(_model->_com2bbox_center, _alpha);
}


void StaticObject::update_bbox() {
	_bbox->_alpha= _alpha;
	_bbox->_center= _com+ _com2bbox_center;
	_bbox->update();
}


void StaticObject::anim(number anim_dt) {
	if (_model->_fixed) {
		return;
	}

	_force= pt_type(0.0);
	_force-= _model->_linear_friction* _velocity;

	_acceleration= _force/ _model->_mass;
	_velocity+= _acceleration* anim_dt;
	_com+= _velocity* anim_dt;

	_torque= 0.0;
	//_torque+= _com2force_fwd.x* _force_fwd.y- _com2force_fwd.y* _force_fwd.x;
	//_torque+= _com2force_bwd.x* _force_bwd.y- _com2force_bwd.y* _force_bwd.x;
	_torque-= _model->_angular_friction* _angular_velocity;
	
	_angular_acceleration= _torque/ _model->_inertia;
	_angular_velocity+= _angular_acceleration* anim_dt;
	_alpha+= _angular_velocity* anim_dt;
	while (_alpha> M_PI* 2.0) {
		_alpha-= M_PI* 2.0;
	}
	while (_alpha< 0.0) {
		_alpha+= M_PI* 2.0;
	}

	update_direction();
	update_bbox();
}


std::ostream & operator << (std::ostream & os, const StaticObject & obj) {
	os << "model = " << obj._model->_json_path;
	os << " ; bbox=[" << *obj._bbox << "] ; ";
	return os;
}


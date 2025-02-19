#include <fstream>
#include <sstream>

#include "json.hpp"

#include "static_object.h"
#include "geom_2d.h"
#include "utile.h"


using json = nlohmann::json;


void collision(StaticObject * obj1, StaticObject * obj2) {
	if (obj1->_model->_fixed && obj2->_model->_fixed) {
		return;
	}

	if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
		return;
	}

	pt_type axis(0.0, 0.0);
	number overlap= 0.0;
	unsigned int idx_pt= 0;
	bool is_pt_in_poly1= false;
	//bool is_inter= bbox_intersects_bbox(car1->_bbox, car2->_bbox, &axis, &overlap, &idx_pt, &is_pt_in_poly1);
	bool is_inter= poly_intersects_poly(obj1->_footprint, obj2->_footprint, &axis, &overlap, &idx_pt, &is_pt_in_poly1);

	// on se place comme dans le cas https://en.wikipedia.org/wiki/Collision_response
	// où la normale est celle de body1 et le point dans body2
	if (is_pt_in_poly1) {
		StaticObject * obj_tmp= obj1;
		obj1= obj2;
		obj2= obj_tmp;
	}

	if (is_inter) {
		// on écarte un peu plus que de 0.5 de chaque coté ou de 1.0 dans le cas fixed
		// est-ce utile ?
		if (obj1->_model->_fixed) {
			obj2->_com+= overlap* 1.05* axis;
		}
		else if (obj2->_model->_fixed) {
			obj1->_com-= overlap* 1.05* axis;
		}
		else {
			obj1->_com-= overlap* 0.55* axis;
			obj2->_com+= overlap* 0.55* axis;
		}

		pt_type r1, r2;
		r1= obj2->_footprint->_pts[idx_pt]- obj1->_com;
		r2= obj2->_footprint->_pts[idx_pt]- obj2->_com;
		
		pt_type r1_norm= normalized(r1);
		pt_type r1_norm_perp(-1.0* r1_norm.y, r1_norm.x);
		pt_type contact_pt_velocity1= obj1->_velocity+ obj1->_angular_velocity* r1_norm_perp;

		pt_type r2_norm= normalized(r2);
		pt_type r2_norm_perp(-1.0* r2_norm.y, r2_norm.x);
		pt_type contact_pt_velocity2= obj2->_velocity+ obj2->_angular_velocity* r2_norm_perp;

		pt_type vr= contact_pt_velocity2- contact_pt_velocity1;

		// https://en.wikipedia.org/wiki/Coefficient_of_restitution
		// restitution doit etre entre 0 et 1 ; proche de 0 -> pas de rebond ; proche de 1 -> beaucoup de rebond
		// TODO : faire des matériaux avec des valeurs de restitution différentes
		number restitution= 0.2;


		number impulse;
		if (obj1->_model->_fixed) {
			pt_type v= (cross2d(r2, axis)/ obj2->_model->_inertia)* r2;
			impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj2->_model->_mass+ dot(v, axis));
		}
		else if (obj2->_model->_fixed) {
			pt_type v= (cross2d(r1, axis)/ obj1->_model->_inertia)* r1;
			impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj1->_model->_mass+ dot(v, axis));
		}
		else {
			pt_type v= (cross2d(r1, axis)/ obj1->_model->_inertia)* r1+ (cross2d(r2, axis)/ obj2->_model->_inertia)* r2;
			impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj1->_model->_mass+ 1.0/ obj2->_model->_mass+ dot(v, axis));
		}

		if (abs(impulse)> 10.0) {
			std::cout << "impulse=" << impulse << "\n";
			//std::cout << "dot(vr, axis)=" << dot(vr, axis) << " ; dot(v, axis)=" << dot(v, axis) << "\n";
			//save_json("../data/test/big_impulse.json");
		}

		if (!obj1->_model->_fixed) {
			obj1->_velocity-= (impulse/ obj1->_model->_mass)* axis;
			obj1->_angular_velocity-= (impulse/ obj1->_model->_inertia)* cross2d(r1, axis);
		}

		if (!obj2->_model->_fixed) {
			obj2->_velocity+= (impulse/ obj2->_model->_mass)* axis;
			obj2->_angular_velocity+= (impulse/ obj2->_model->_inertia)* cross2d(r2, axis);
		}

		// peut-être pas nécessaire
		obj1->_acceleration= pt_type(0.0);
		obj1->_angular_acceleration= 0.0;
		obj2->_acceleration= pt_type(0.0);
		obj2->_angular_acceleration= 0.0;
	}
}


// StaticObjectModel --------------------------------------------------------------
StaticObjectModel::StaticObjectModel() {

}


StaticObjectModel::StaticObjectModel(std::string json_path) : _json_path(json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	if (js["type"]== "obstacle_setting") {
		_type= OBSTACLE_SETTING;
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
	else {
		std::cerr << "Type " << js["type"] << " non supporté\n";
	}

	_footprint= new Polygon2D();
	std::vector<pt_type> pts;
	for (auto coord : js["footprint"]) {
		pt_type pt(coord[0], coord[1]);
		pts.push_back(pt);
	}
	_footprint->set_points(pts);
	
	if (_type== OBSTACLE_SETTING) {
		_fixed= true;
		_com2bbox_center= pt_type(0.0);
		_halfsize= pt_type(0.0);
		_mass= _linear_friction= _angular_friction= 0.0;
	}
	
	else if (_type== OBSTACLE_FLOATING) {
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
		}
	}
	
	else if (_type== HERO_CAR || _type== ENNEMY_CAR) {
		_fixed= false;
		_com2bbox_center= pt_type(js["com2bbox_center"][0], js["com2bbox_center"][1]);
		_halfsize= pt_type(js["halfsize"][0], js["halfsize"][1]);
		_mass= js["mass"];
		_angular_friction= js["angular_friction"];
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


/*StaticObject::StaticObject(StaticObjectModel * model) : 
	_model(model), _com(pt_type(0.0)), _alpha(0.0), _velocity(pt_type(0.0)), _acceleration(pt_type(0.0)), _force(pt_type(0.0)),
	_angular_velocity(0.0), _angular_acceleration(0.0), _torque(0.0)
{
	_bbox= new BBox_2D(pt_type(0.0), pt_type(0.0));
}*/


StaticObject::StaticObject(StaticObjectModel * model, pt_type position, number alpha) : _model(model) {
	_bbox= new BBox_2D(position, model->_halfsize);
	_footprint= new Polygon2D();
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

	update();
}


void StaticObject::update() {
	_com2bbox_center= rot(_model->_com2bbox_center, _alpha);
	_footprint->set_points(_model->_footprint->_pts);
	_footprint->translate(_com);
	_footprint->rotate(_com, _alpha);

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

	update();
}


std::ostream & operator << (std::ostream & os, const StaticObject & obj) {
	os << "model = " << obj._model->_json_path;
	os << " ; bbox=[" << *obj._bbox << "] ; ";
	return os;
}


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

	if (!obj1->_model->_solid || !obj2->_model->_solid) {
		return;
	}

	// 1er test - cher sur les AABB
	if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
		return;
	}

	// axis est le vecteur le long duquel les 2 objets s'intersectent le plus
	// overlap est la taille de l'intersection le long de cet axe
	// idx_pt est l'indice du pt de l'objet qui pénètre l'autre
	// is_pt_in_poly1 indique si c'est poly1 qui est pénétrant ou pénétré
	// is_inter vaudra true s'il y a intersection
	pt_type axis(0.0, 0.0);
	number overlap= 0.0;
	unsigned int idx_pt= 0;
	bool is_pt_in_poly1= false;
	bool is_inter= poly_intersects_poly(obj1->_footprint, obj2->_footprint, &axis, &overlap, &idx_pt, &is_pt_in_poly1);

	// pas d'intersection : on sort
	if (!is_inter) {
		return;
	}

	// on se place comme dans le cas https://en.wikipedia.org/wiki/Collision_response
	// où la normale est celle de body1 et le point dans body2
	if (is_pt_in_poly1) {
		StaticObject * obj_tmp= obj1;
		obj1= obj2;
		obj2= obj_tmp;
	}

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

	// voir doc/collision.png récupéré de https://en.wikipedia.org/wiki/Collision_response
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

	// sera la norme de la nouvelle vitesse des objets
	number impulse;

	// https://en.wikipedia.org/wiki/Coefficient_of_restitution
	// restitution doit etre entre 0 et 1 ; proche de 0 -> pas de rebond ; proche de 1 -> beaucoup de rebond
	// en pratique j'ai mis des restitution > 1 pour plus de fun
	// on prend la moyenne
	number restitution= 0.5* (obj1->_model->_restitution+ obj2->_model->_restitution);
	
	/*number restitution= 0.1;
	obj1->_mass= 1.0;
	obj2->_mass= 1.0;
	obj1->_inertia= 1.0;
	obj2->_inertia= 1.0;*/
	
	// dans le cas où 1 des 2 objets est fixe on considère que sa masse et son inertie sont infinies
	if (obj1->_model->_fixed) {
		pt_type v= (cross2d(r2, axis)/ obj2->_inertia)* r2;
		impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj2->_mass+ dot(v, axis));
	}
	else if (obj2->_model->_fixed) {
		pt_type v= (cross2d(r1, axis)/ obj1->_inertia)* r1;
		impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj1->_mass+ dot(v, axis));
	}
	else {
		pt_type v= (cross2d(r1, axis)/ obj1->_inertia)* r1+ (cross2d(r2, axis)/ obj2->_inertia)* r2;
		impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj1->_mass+ 1.0/ obj2->_mass+ dot(v, axis));
	}

	if (abs(impulse)> 6.0) {
		std::cout << "impulse=" << impulse << "\n";
	}

	// on modifie directement la vitesse et la vitesse angulaire
	if (!obj1->_model->_fixed) {
		obj1->_velocity-= (impulse/ obj1->_mass)* axis;
		obj1->_angular_velocity-= (impulse/ obj1->_inertia)* cross2d(r1, axis);
	}

	if (!obj2->_model->_fixed) {
		obj2->_velocity+= (impulse/ obj2->_mass)* axis;
		obj2->_angular_velocity+= (impulse/ obj2->_inertia)* cross2d(r2, axis);
	}

	// peut-être pas nécessaire
	obj1->_acceleration= pt_type(0.0);
	obj1->_angular_acceleration= 0.0;
	obj2->_acceleration= pt_type(0.0);
	obj2->_angular_acceleration= 0.0;
}


// StaticObjectModel --------------------------------------------------------------
StaticObjectModel::StaticObjectModel() {

}


StaticObjectModel::StaticObjectModel(std::string json_path) {
	load(json_path);
}


StaticObjectModel::~StaticObjectModel() {

}


void StaticObjectModel::load(std::string json_path) {
	_json_path= json_path;

	std::ifstream ifs(_json_path);
	json js= json::parse(ifs);
	ifs.close();

	// type d'objet
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
	else if (js["type"]== "checkpoint") {
		_type= CHECKPOINT;
	}
	else if (js["type"]== "start") {
		_type= START;
	}
	else {
		std::cerr << "Type " << js["type"] << " non supporté\n";
	}

	// empreinte au sol
	_footprint= new Polygon2D();
	std::vector<pt_type> pts;
	for (auto coord : js["footprint"]) {
		pt_type pt(coord[0], coord[1]);
		pts.push_back(pt);
	}
	_footprint->set_points(pts);
	_footprint->update_all();

	// vecteur centre de masse -> centre bbox
	_com2bbox_center= -1.0* _footprint->_centroid;
	
	// les autres valeurs dépendent du type d'objet
	if (_type== OBSTACLE_SETTING) {
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

	else if (_type== CHECKPOINT || _type== START) {
		_fixed= true;
		_solid= false;
		_mass= _linear_friction= _angular_friction= _restitution= 0.0;
	}
}


std::ostream & operator << (std::ostream & os, const StaticObjectModel & model) {
	os << "json_path=" << model._json_path;
	os << " ; type=";
	if (model._type== OBSTACLE_SETTING){
		os << "OBSTACLE_SETTING";
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
	if (_model->_type== OBSTACLE_SETTING) {
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


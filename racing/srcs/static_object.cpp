#include <fstream>
#include <sstream>

#include "json.hpp"

#include "static_object.h"
#include "utile.h"
#include "gl_utils.h"


using json = nlohmann::json;


// StaticObjectModel --------------------------------------------------------------
StaticObjectModel::StaticObjectModel() : ActionableObjectModel() {

}


StaticObjectModel::StaticObjectModel(std::string json_path) :
	ActionableObjectModel(json_path),
	_material_name(""), _material(NULL), _translation_constraint(pt_type(0.0))
{
	_footprint= new Polygon2D();
	load(json_path);
}


StaticObjectModel::~StaticObjectModel() {
	delete _footprint;
}


void StaticObjectModel::load(std::string json_path) {
	_json_path= json_path;
	_name= basename(_json_path);

	std::ifstream ifs(_json_path);
	json js= json::parse(ifs);
	ifs.close();

	// type d'objet
	if (STR2OBJTYPE.count(js["type"])> 0) {
		_type= STR2OBJTYPE.at(js["type"]); // [] ne fonctionne pas ici
	}
	else {
		std::cerr << "Type " << js["type"] << " non supporté\n";
	}

	// empreinte au sol
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

	if (js["material"]!= nullptr) {
		_material_name= js["material"];
	}
	else {
		_material_name= "no_material";
	}
	
	if (_type== OBSTACLE_TILE || _type== CHECKPOINT || _type== START || _type== REPAIR ||
		_type== SURFACE_FLOATING || _type== SURFACE_TILE || _type== BOOST || _type== DIRECTION_HELP) {
		_movement= MVMT_FIXED;
	}
	else if (_type== OBSTACLE_FLOATING || _type== DECORATION) {
		// on met le com (center of mass) à 0,0 afin de faciliter les rotations autour du com
		_footprint->centroid2zero();
		_footprint->update_all();
		if (js["movement"]!= nullptr) {
			_movement= STR2OBJMVMTTYPE.at(js["movement"]);
			if (_movement== MVMT_TRANSLATE_CONSTRAINED) {
				_translation_constraint.x= js["translation_constraint"][0];
				_translation_constraint.y= js["translation_constraint"][1];
			}
		}
		else {
			_movement= MVMT_FIXED;
		}
	}
	else if (_type== CAR) {
		// on met le com (center of mass) à 0,0 afin de faciliter les rotations autour du com
		_footprint->centroid2zero();
		_footprint->update_all();
		_movement= MVMT_ALL;
	}
}


std::ostream & operator << (std::ostream & os, const StaticObjectModel & model) {
	std::map<ObjectType, std::string> obj2str;
	for (auto x : STR2OBJTYPE) {
		obj2str[x.second]= x.first;
	}
	os << "json_path=" << model._json_path;
	os << " ; type=" << obj2str[model._type];
	os << " ; footprint=" << *model._footprint;
	return os;
}


// StaticObject --------------------------------------------------------------------
StaticObject::StaticObject() {

}


StaticObject::StaticObject(StaticObjectModel * model, pt_type position, number alpha, pt_type scale) :
 	ActionableObject((ActionableObjectModel *)(model)),
	_model(model), _scale(pt_type(0.0)), _mass(0.0), _inertia(0.0), _com(pt_type(0.0)), _velocity(pt_type(0.0)),
	_acceleration(pt_type(0.0)), _force(pt_type(0.0)), _alpha(0.0), _angular_velocity(0.0), _angular_acceleration(0.0),
	_torque(0.0), _current_surface(NULL), _previous_surface(NULL),
	_linear_friction_surface(1.0), _angular_friction_surface(1.0), _car_contact(false)
{
	_bbox= new BBox_2D(pt_type(0.0), pt_type(0.0));
	_footprint= new Polygon2D();
	_footprint->set_points(_model->_footprint->_pts);
	_footprint->triangulate(); // on a besoin de trianguler pour l'affichage par des triangles du footprint
	_footprint->update_all();
	reinit(position, alpha, scale);
	//_z= Z_OBJECTS[_model->_type]; // non ; cf https://stackoverflow.com/questions/39085520/cant-i-use-map-operator-with-const-function-keyword
	_z= Z_OBJECTS.at(_model->_type);
}


StaticObject::~StaticObject() {
	delete _footprint;
	delete _bbox;
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

	for (unsigned int i=0; i<N_BUMPS; ++i) {
		_bumps[i]= 0.0;
	}

	update();
}


void StaticObject::update() {
	// maj du footprint
	_footprint->set_points(_model->_footprint->_pts);
	_footprint->scale(_scale);
	_footprint->rotate(pt_type(0.0), _alpha); // on a recentré sur 0 le footprint des objets susceptibles de tourner
	_footprint->translate(_com);
	_footprint->update_all(); // peut-être pas besoin de tout updater

	// maj de la bbox
	_bbox->_alpha= _alpha;
	if (_model->_type== OBSTACLE_TILE || _model->_type== SURFACE_TILE) {
		_bbox->_center= _com;
		_bbox->_half_size= 0.5* _scale;
	}
	else {
		_bbox->_center= _com+ rot(pt_type(_model->_com2bbox_center.x* _scale.x, _model->_com2bbox_center.y* _scale.y), _alpha);
		_bbox->_half_size= 0.5* _scale;
	}
	_bbox->update();

	// maj masse et inertie
	if (_model->_material!= NULL) {
		_mass= _model->_material->_density* _scale.x* _scale.y;
		// !!!
		// obligé de rajouter * facteur sinon tout pète lors des collisions
		// !!!
		_inertia= _mass* _footprint->_inertia* INERTIA_FACTOR;
	}
	else {
		_mass= _inertia= 0.0;
	}

	// maj contrainte
	_translation_constraint= rot(_model->_translation_constraint, _alpha);
}


void StaticObject::set_current_surface(Material * material, time_point t) {
	_current_surface= material;
	_last_change_surface_t= t;
}


// on renvoie true lorsqu'il y a changement de surface; ce bool est récupéré dans car.cpp pour mettre à jour le tire_track
bool StaticObject::anim_surface(time_point t) {
	// ajustement _linear_friction_material / _angular_friction_material
	if (_previous_surface!= _current_surface) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_change_surface_t).count();
		if (dt> _previous_surface->_surface_change_ms) {
			_linear_friction_surface= _current_surface->_linear_friction;
			_angular_friction_surface= _current_surface->_angular_friction;
			_previous_surface= _current_surface;
			return true;
		}
		else {
			// interpolation linéaire des frictions
			_linear_friction_surface= _previous_surface->_linear_friction+ (number(dt)/ number(_previous_surface->_surface_change_ms))* (_current_surface->_linear_friction- _previous_surface->_linear_friction);
			_angular_friction_surface= _previous_surface->_angular_friction+ (number(dt)/ number(_previous_surface->_surface_change_ms))* (_current_surface->_angular_friction- _previous_surface->_angular_friction);
		}
	}
	return false;
}


void StaticObject::anim(number anim_dt, time_point t) {
	anim_sequence(t);
	anim_action(t);
	anim_texture(t);

	if (_model->_movement== MVMT_FIXED) {
		return;
	}

	anim_surface(t);
	anim_force(t);

	Action * action= get_current_action();

	if (_model->_movement== MVMT_ALL || _model->_movement== MVMT_TRANSLATE || _model->_movement== MVMT_TRANSLATE_CONSTRAINED) {
		_force= pt_type(0.0);
		if (!action->_forces.empty() && action->_forces[_current_action_force_idx]->_type== TRANSLATION) {
			_force+= rot(action->_forces[_current_action_force_idx]->_force, _alpha);
		}
		_force-= _model->_material->_linear_friction* _linear_friction_surface* _velocity; // friction

		// force -> acceleration -> vitesse -> position
		_acceleration= _force/ _mass;
		_velocity+= _acceleration* anim_dt;
		_com+= _velocity* anim_dt;
	}

	if (_model->_movement== MVMT_ALL || _model->_movement== MVMT_ROTATE) {
		// calcul torque == équivalent force pour les rotations
		_torque= 0.0;
		if (!action->_forces.empty() && action->_forces[_current_action_force_idx]->_type== ROTATION) {
			_torque+= action->_forces[_current_action_force_idx]->_torque;
		}
		_torque-= _model->_material->_angular_friction* _angular_friction_surface* _angular_velocity; // friction
		
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


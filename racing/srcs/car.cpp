#include <fstream>
#include <sstream>

#include "json.hpp"

#include "car.h"
#include "geom_2d.h"
#include "utile.h"


using json = nlohmann::json;


// CarModel ------------------------------------------------------
CarModel::CarModel() {

}


CarModel::CarModel(std::string json_path) {
	load(json_path);
}


CarModel::~CarModel() {
	
}


void CarModel::load(std::string json_path) {
	StaticObjectModel::load(json_path);

	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	// on suppose qu'initialement le véhicule est dirigé vers y positif
	_forward= pt_type(0.0, 1.0);
	_right= pt_type(1.0, 0.0);
	_com2force_fwd= pt_type(js["com2force_fwd"][0], js["com2force_fwd"][1]);
	_com2force_bwd= pt_type(js["com2force_bwd"][0], js["com2force_bwd"][1]);
	_max_wheel= js["max_wheel"];
	_wheel_increment= js["wheel_increment"];
	_wheel_decrement= js["wheel_decrement"];
	_max_brake= js["max_brake"];
	_max_thrust= js["max_thrust"];
	_thrust_increment= js["thrust_increment"];
	_brake_increment= js["brake_increment"];
	_thrust_decrement= js["thrust_decrement"];
	_forward_static_friction= js["forward_static_friction"];
	_backward_static_friction= js["backward_static_friction"];
	_backward_dynamic_friction= js["backward_dynamic_friction"];
	_friction_threshold= js["friction_threshold"];
	_angular_friction= js["angular_friction"];
}


// Car ------------------------------------------------------------
Car::Car() {

}


Car::Car(CarModel * model, pt_type position, number alpha, pt_type scale) : 
	StaticObject(model, position, alpha, scale), _next_checkpoint(NULL), _n_laps(0), 
	_linear_friction_material(1.0), _angular_friction_material(1.0), _rank(0), _finished(false)
{
	reinit(position, alpha, scale);
}


Car::~Car() {
	delete _bbox;
}


CarModel * Car::get_model() {
	return (CarModel *)(_model);
}


void Car::reinit(pt_type position, number alpha, pt_type scale) {
	StaticObject::reinit(position, alpha, scale);

	_force_fwd= pt_type(0.0);
	_force_bwd= pt_type(0.0);
	_wheel= 0.0;
	_thrust= 0.0;
	_drift= false;

	update();
}


void Car::update() {
	StaticObject::update();

	CarModel * model= get_model();
	_com2force_fwd= rot(model->_com2force_fwd, _alpha);
	_com2force_bwd= rot(model->_com2force_bwd, _alpha);
	_right= rot(model->_right, _alpha);
	_forward= rot(model->_forward, _alpha);
}


void Car::preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up) {
	CarModel * model= get_model();

	// volant ------------------------------------------------
	// a gauche == positif (rotation sens trigo)
	if (key_left) {
		_wheel+= model->_wheel_increment;
		if (_wheel> model->_max_wheel) {
			_wheel= model->_max_wheel;
		}
	}
	if (key_right) {
		_wheel-= model->_wheel_increment;
		if (_wheel< -1.0* model->_max_wheel) {
			_wheel= -1.0* model->_max_wheel;
		}
	}
	if (!key_left && !key_right) {
		if (_wheel< -1.0* model->_wheel_decrement) {
			_wheel+= model->_wheel_decrement;
		}
		else if (_wheel> model->_wheel_decrement) {
			_wheel-= model->_wheel_decrement;
		}
		else {
			_wheel= 0.0;
		}
	}

	// accélération --------------------------------------------
	if (key_down) {
		_thrust-= model->_brake_increment;
		if (_thrust< -1.0* model->_max_brake) {
			_thrust= -1.0* model->_max_brake;
		}
	}
	if (key_up) {
		_thrust+= model->_thrust_increment;
		if (_thrust> model->_max_thrust) {
			_thrust= model->_max_thrust;
		}
	}
	if (!key_down && !key_up) {
		if (_thrust< -1.0* model->_thrust_decrement) {
			_thrust+= model->_thrust_decrement;
		}
		else if (_thrust> model->_thrust_decrement) {
			_thrust-= model->_thrust_decrement;
		}
		else {
			_thrust= 0.0;
		}
	}
}


void Car::preanim_joystick(bool joystick_a, bool joystick_b, glm::vec2 joystick) {
	CarModel * model= get_model();

	if (abs(joystick.x)> 0.3) {
		// a gauche == positif (rotation sens trigo)
		_wheel-= model->_wheel_increment* joystick.x;
		if (_wheel> model->_max_wheel) {
			_wheel= model->_max_wheel;
		}
		if (_wheel< -1.0* model->_max_wheel) {
			_wheel= -1.0* model->_max_wheel;
		}
	}
	else {
		if (_wheel< -1.0* model->_wheel_decrement) {
			_wheel+= model->_wheel_decrement;
		}
		else if (_wheel> model->_wheel_decrement) {
			_wheel-= model->_wheel_decrement;
		}
		else {
			_wheel= 0.0;
		}
	}

	if (joystick_a) {
		_thrust-= model->_brake_increment;
		if (_thrust< -1.0* model->_max_brake) {
			_thrust= -1.0* model->_max_brake;
		}
	}
	if (joystick_b) {
		_thrust+= model->_thrust_increment;
		if (_thrust> model->_max_thrust) {
			_thrust= model->_max_thrust;
		}
	}
	if (!joystick_a && !joystick_b) {
		if (_thrust< -1.0* model->_thrust_decrement) {
			_thrust+= model->_thrust_decrement;
		}
		else if (_thrust> model->_thrust_decrement) {
			_thrust-= model->_thrust_decrement;
		}
		else {
			_thrust= 0.0;
		}
	}
}


void Car::random_ia() {
	CarModel * model= get_model();

	if (rand_int(0, 100)> 20) {
		_thrust+= model->_thrust_increment;
		if (_thrust> model->_max_thrust) {
			_thrust= model->_max_thrust;
		}
	}
	else if (rand_int(0, 100)> 50) {
		_thrust-= model->_brake_increment;
		if (_thrust< -1.0* model->_max_brake) {
			_thrust= -1.0* model->_max_brake;
		}
	}
	else {
		if (_thrust< -1.0* model->_thrust_decrement) {
			_thrust+= model->_thrust_decrement;
		}
		else if (_thrust> model->_thrust_decrement) {
			_thrust-= model->_thrust_decrement;
		}
		else {
			_thrust= 0.0;
		}
	}
	
	if (rand_int(0, 100)> 50) {
		_wheel+= model->_wheel_increment;
		if (_wheel> model->_max_wheel) {
			_wheel= model->_max_wheel;
		}
	}
	else if (rand_int(0, 100)> 20) {
		_wheel-= model->_wheel_increment;
		if (_wheel< -1.0* model->_max_wheel) {
			_wheel= -1.0* model->_max_wheel;
		}
	}
	else {
		if (_wheel< -1.0* model->_wheel_decrement) {
			_wheel+= model->_wheel_decrement;
		}
		else if (_wheel> model->_wheel_decrement) {
			_wheel-= model->_wheel_decrement;
		}
		else {
			_wheel= 0.0;
		}
	}
}


void Car::set_current_surface(Material * material, std::chrono::system_clock::time_point t) {
	StaticObject::set_current_surface(material);
	_last_change_surface_t= t;
}


void Car::anim(number anim_dt, std::chrono::system_clock::time_point t) {
	CarModel * model= get_model();

	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_change_surface_t).count();
	if (dt> SURFACE_CHANGE_DT) {
		_linear_friction_material= _current_surface->_linear_friction;
		_angular_friction_material= _current_surface->_angular_friction;
		_tire_track_texture_idx= _current_surface->_tire_track_texture_idx;
	}
	else {
		if (_linear_friction_material> _current_surface->_linear_friction+ LINEAR_FRICTION_MATERIAL_INCREMENT) {
			_linear_friction_material-= LINEAR_FRICTION_MATERIAL_INCREMENT;
		}
		else if (_linear_friction_material< _current_surface->_linear_friction- LINEAR_FRICTION_MATERIAL_INCREMENT) {
			_linear_friction_material+= LINEAR_FRICTION_MATERIAL_INCREMENT;
		}
		if (_angular_friction_material> _current_surface->_angular_friction+ ANGULAR_FRICTION_MATERIAL_INCREMENT) {
			_angular_friction_material-= ANGULAR_FRICTION_MATERIAL_INCREMENT;
		}
		else if (_angular_friction_material< _current_surface->_angular_friction- ANGULAR_FRICTION_MATERIAL_INCREMENT) {
			_angular_friction_material+= ANGULAR_FRICTION_MATERIAL_INCREMENT;
		}
	}

	// plus Car a des bumps plus il est difficile d'accélerer et de tourner
	// les bumps 4 et 5 sont à l'avant ; 3 et 6 sont à l'avant sur les côtés
	_thrust*= 1.0- BUMP_THRUST_FACTOR* _bumps[4]/ BUMP_MAX- BUMP_THRUST_FACTOR* _bumps[5]/ BUMP_MAX;
	_wheel*= 1.0- BUMP_WHEEL_FACTOR* _bumps[3]/ BUMP_MAX- BUMP_WHEEL_FACTOR* _bumps[6]/ BUMP_MAX;

	// calcul force appliquée à l'avant
	_force_fwd= pt_type(0.0);
	_force_fwd+= _thrust* rot(_forward, _wheel); // accélération dans la direction du volant
	_force_fwd-= model->_forward_static_friction* _linear_friction_material* scal(_forward, _velocity)* _forward; // friction statique avant

	// calcul force appliquée à l'arrière
	_force_bwd= pt_type(0.0);
	// + la vitesse est grande et + on tourne abruptement, + on a de chance d'être en dérapage
	// les bumps 2 et 7 (roues arrières) influent sur _drift
	number right_turn= scal(_right, _velocity);
	if (abs(right_turn)> model->_friction_threshold || _bumps[2]> BUMP_DRIFT_THRESHOLD || _bumps[7]> BUMP_DRIFT_THRESHOLD) {
		// dérapage
		_drift= true;
		_force_bwd-= model->_backward_dynamic_friction* _linear_friction_material* right_turn* _right;
		//_force_bwd-= model->_backward_dynamic_friction* right_turn* _right;
	}
	else {
		_drift= false;
		_force_bwd-= model->_backward_static_friction* _linear_friction_material* right_turn* _right;
		//_force_bwd-= model->_backward_static_friction* right_turn* _right;
	}

	// force -> acceleration -> vitesse -> position
	_acceleration= (_force_fwd+ _force_bwd)/ _mass;
	_velocity+= _acceleration* anim_dt;
	_com+= _velocity* anim_dt;

	// calcul torque == équivalent force pour les rotations
	_torque= 0.0;
	_torque+= _com2force_fwd.x* _force_fwd.y- _com2force_fwd.y* _force_fwd.x; // torque avant
	_torque+= _com2force_bwd.x* _force_bwd.y- _com2force_bwd.y* _force_bwd.x; // torque arrière
	_torque-= model->_angular_friction* _angular_friction_material* _angular_velocity; // friction angulaire

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


std::ostream & operator << (std::ostream & os, const Car & car) {
	os << "model = " << car._model->_json_path;
	os << " ; bbox=[" << *car._bbox << "] ; ";
	return os;
}


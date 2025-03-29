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


CarModel::CarModel(std::string json_path) : StaticObjectModel(json_path) {
	load(json_path);
}


CarModel::~CarModel() {
	
}


void CarModel::load(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	// on suppose qu'initialement le véhicule est dirigé vers y positif
	_forward= pt_type(0.0, 1.0);
	_right= pt_type(1.0, 0.0);
	_com2force_fwd= pt_type(js["com2force_fwd"][0], js["com2force_fwd"][1]);
	_com2force_bwd= pt_type(js["com2force_bwd"][0], js["com2force_bwd"][1]);
	_max_wheel= js["max_wheel"];
	_max_wheel_reverse= js["max_wheel_reverse"];
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
	_friction_threshold_braking= js["friction_threshold_braking"];
	_angular_friction= js["angular_friction"];
	_speed_wheel_factor= js["speed_wheel_factor"];
}


// Car ------------------------------------------------------------
Car::Car() {

}


Car::Car(CarModel * model, pt_type position, number alpha, pt_type scale) : 
	StaticObject(model, position, alpha, scale),
	_com2force_fwd(pt_type(0.0)), _com2force_bwd(pt_type(0.0)), _forward(pt_type(0.0)), _right(pt_type(0.0)),
	_force_fwd(pt_type(0.0)), _force_bwd(pt_type(0.0)), _wheel(0.0), _thrust(0.0), _drift(false),
	_name(""), _next_checkpoint(NULL), _n_laps(0), _rank(0), _finished(false), _tire_track_texture_idx(0),
	_total_time(999.0)
{
	reinit(position, alpha, scale);
}


Car::~Car() {
	
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

	_speed= sqrt(_velocity.x* _velocity.x+ _velocity.y* _velocity.y);
}


void Car::preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up) {
	CarModel * model= get_model();

	// + on va vite moins on peut tourner; évite de faire des gros virages à haute vitesse
	number wheel_modifier= (1.0- _speed/ model->_speed_wheel_factor);
	if (wheel_modifier< 0.0) {
		wheel_modifier= 0.0;
	}

	// en marche arrière on peut moins tourner le volant
	number max_wheel= model->_max_wheel;
	if (_thrust< 0.0) {
		max_wheel= model->_max_wheel_reverse;
	}

	// volant ------------------------------------------------
	// a gauche == positif (rotation sens trigo)
	if (key_left) {
		_wheel+= model->_wheel_increment* wheel_modifier;
		if (_wheel> max_wheel) {
			_wheel= max_wheel;
		}
	}

	if (key_right) {
		_wheel-= model->_wheel_increment* wheel_modifier;
		if (_wheel< -1.0* max_wheel) {
			_wheel= -1.0* max_wheel;
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

	// accélération / freinage --------------------------------------------
	if (key_down) {
		_thrust-= model->_brake_increment;
		if (_thrust< -1.0* model->_max_brake) {
			_thrust= -1.0* model->_max_brake;
		}
		_braking= true;
	}
	else {
		_braking= false;
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
		number wheel_modifier= (1.0- _speed/ model->_speed_wheel_factor);
		if (wheel_modifier< 0.0) {
			wheel_modifier= 0.0;
		}
		// en marche arrière on peut moins tourner le volant
		number max_wheel= model->_max_wheel;
		if (_thrust< 0.0) {
			max_wheel= model->_max_wheel_reverse;
		}

		// a gauche == positif (rotation sens trigo)
		_wheel-= model->_wheel_increment* joystick.x* JOYSTICK_FACTOR* wheel_modifier;
		if (_wheel> max_wheel) {
			_wheel= max_wheel;
		}
		if (_wheel< -1.0* max_wheel) {
			_wheel= -1.0* max_wheel;
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
		_braking= true;
		_thrust-= model->_brake_increment;
		if (_thrust< -1.0* model->_max_brake) {
			_thrust= -1.0* model->_max_brake;
		}
	}
	else {
		_braking= false;
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


/*void Car::random_ia() {
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
}*/


void Car::anim(number anim_dt, std::chrono::system_clock::time_point t) {
	CarModel * model= get_model();

	if (StaticObject::anim_surface(t)) {
		_tire_track_texture_idx= _current_surface->_tire_track_texture_idx;	
	}
	StaticObject::anim_texture(t);

	// plus Car a des bumps plus il est difficile d'accélerer et de tourner
	// les bumps 4 et 5 sont à l'avant ; 3 et 6 sont à l'avant sur les côtés
	_thrust*= 1.0- BUMP_THRUST_FACTOR* _bumps[4]/ BUMP_MAX- BUMP_THRUST_FACTOR* _bumps[5]/ BUMP_MAX;
	_wheel*= 1.0- BUMP_WHEEL_FACTOR* _bumps[3]/ BUMP_MAX- BUMP_WHEEL_FACTOR* _bumps[6]/ BUMP_MAX;

	// sur une surface glissante les roues dérapent et thrust baisse
	_thrust*= _current_surface->_slippery;

	// calcul force appliquée à l'avant
	_force_fwd= pt_type(0.0);
	_force_fwd+= _thrust* rot(_forward, _wheel); // accélération dans la direction du volant
	// friction statique avant
	//_force_fwd-= model->_forward_static_friction* _linear_friction_surface* scal(_forward, _velocity)* _forward;
	if (_thrust> 0.0) {
		_force_fwd-= model->_forward_static_friction* _linear_friction_surface* _velocity;
	}
	else {
		_force_fwd-= 1.0* model->_forward_static_friction* _linear_friction_surface* _velocity;
		//_force_fwd-= 0.1* model->_forward_static_friction* _linear_friction_surface* scal(_forward, _velocity)* _forward;
	}

	// calcul force appliquée à l'arrière
	_force_bwd= pt_type(0.0);
	// + la vitesse est grande et + on tourne abruptement, + on a de chance d'être en dérapage
	// les bumps 2 et 7 (roues arrières) influent sur _drift
	number right_turn= scal(_right, _velocity);
	if (_thrust> 0.0 && (  (!_braking && abs(right_turn)> model->_friction_threshold)
		|| (_braking && abs(right_turn)> model->_friction_threshold_braking)
		|| _bumps[2]> BUMP_DRIFT_THRESHOLD || _bumps[7]> BUMP_DRIFT_THRESHOLD)) {
		// dérapage
		_drift= true;
		_force_bwd-= model->_backward_dynamic_friction* _linear_friction_surface* right_turn* _right;
	}
	else {
		_drift= false;
		if (_thrust> 0.0) {
			_force_bwd-= model->_backward_static_friction* _linear_friction_surface* right_turn* _right;
		}
		else {
			_force_bwd-= 1.0* model->_backward_static_friction* _linear_friction_surface* right_turn* _right;
		}
	}

	// force -> acceleration -> vitesse -> position
	_acceleration= (_force_fwd+ _force_bwd)/ _mass;
	_velocity+= _acceleration* anim_dt;
	_com+= _velocity* anim_dt;

	// calcul torque == équivalent force pour les rotations
	_torque= 0.0;
	_torque+= _com2force_fwd.x* _force_fwd.y- _com2force_fwd.y* _force_fwd.x; // torque avant
	_torque+= _com2force_bwd.x* _force_bwd.y- _com2force_bwd.y* _force_bwd.x; // torque arrière
	_torque-= model->_angular_friction* _angular_friction_surface* _angular_velocity; // friction angulaire

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


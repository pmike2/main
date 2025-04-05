#include <fstream>
#include <sstream>

#include "json.hpp"

#include "static_object.h"
#include "utile.h"
#include "gl_utils.h"


using json = nlohmann::json;


// ActionTexture -----------------------------------------------------------------
ActionTexture::ActionTexture() {

}


ActionTexture::ActionTexture(std::string texture_path, unsigned int n_ms) : _texture_path(texture_path), _n_ms(n_ms) {
	std::string bump= splitext(_texture_path).first+ "_bump.png";
	if (file_exists(bump)) {
		_texture_path_bump= bump;
	}
	else {
		// si la version bump n'existe pas on sautera dans fill_texture_array l'indice correspondant
		_texture_path_bump= NO_PNG;
	}
}


ActionTexture::~ActionTexture() {

}


// ActionForce --------------------------------------------------------------------
ActionForce::ActionForce() {

}


ActionForce::ActionForce(pt_type direction, pt_type direction_rand, unsigned int n_ms) : 
	_direction(direction), _direction_rand(direction_rand), _n_ms(n_ms) 
{

}


ActionForce::~ActionForce() {

}


// Action -------------------------------------------------------------------------
Action::Action() {

}


Action::~Action() {
	for (auto tex : _textures) {
		delete tex;
	}
	_textures.clear();
	for (auto force : _forces) {
		delete force;
	}
	_forces.clear();
}


// ActionSequence -----------------------------------------------------------------
ActionSequence::ActionSequence() {

}


ActionSequence::~ActionSequence() {
	
}


// Actiontransition ---------------------------------------------------------------
SequenceTransition::SequenceTransition() {

}


SequenceTransition::SequenceTransition(std::string from, std::string to, unsigned int n_ms) : _from(from), _to(to), _n_ms(n_ms) {

}


SequenceTransition::~SequenceTransition() {

}


// StaticObjectModel --------------------------------------------------------------
StaticObjectModel::StaticObjectModel() {

}


StaticObjectModel::StaticObjectModel(std::string json_path) :
	_material_name(""), _material(NULL) 
{
	_footprint= new Polygon2D();
	load(json_path);
}


StaticObjectModel::~StaticObjectModel() {
	delete _footprint;
	/*for (auto action : _actions) {
		delete action.second;
	}
	_actions.clear();*/
	for (auto transition : _transitions) {
		delete transition;
	}
	_transitions.clear();
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
		_type== SURFACE_FLOATING || _type== SURFACE_TILE || _type== BOOST) {
		_fixed= true;
		_no_rotation= true;
	}
	else if (_type== OBSTACLE_FLOATING || _type== DECORATION) {
		// on met le com (center of mass) à 0,0 afin de faciliter les rotations autour du com
		_footprint->centroid2zero();
		_footprint->update_all();
		_fixed= js["fixed"];
		if (js["no_rotation"]!= nullptr) {
			_no_rotation= js["no_rotation"];
		}
		else {
			_no_rotation= false;
		}
	}
	else if (_type== CAR) {
		// on met le com (center of mass) à 0,0 afin de faciliter les rotations autour du com
		_footprint->centroid2zero();
		_footprint->update_all();
		_fixed= false;
		_no_rotation= false;
	}

	std::map<std::string, Action *> actions;
	if (js["actions"]!= nullptr) {
		for (json::iterator it = js["actions"].begin(); it != js["actions"].end(); ++it) {
			auto & action_name= it.key();
			auto & l_ac= it.value();
			actions[action_name]= new Action();
			for (auto ac : l_ac) {
				if (ac["texture"]!= nullptr) {
					std::string texture_rel_path= ac["texture"];
					std::string texture_path= dirname(_json_path)+ "/textures/"+ texture_rel_path;
					unsigned int n_ms= 0;
					if (ac["n_ms"]!= nullptr) {
						n_ms= ac["n_ms"];
					}
					actions[action_name]->_textures.push_back(new ActionTexture(texture_path, n_ms));
				}
				else if (ac["force"]!= nullptr) {
					pt_type direction= pt_type(ac["force"][0], ac["force"][1]);
					pt_type direction_rand(0.0);
					if (ac["force_rand"]!= nullptr) {
						direction_rand= pt_type(ac["force_rand"][0], ac["force_rand"][1]);
					}
					unsigned int n_ms= 0;
					if (ac["n_ms"]!= nullptr) {
						n_ms= ac["n_ms"];
					}
					actions[action_name]->_forces.push_back(new ActionForce(direction, direction_rand, n_ms));
				}
			}
		}
	}
	else {
		actions[MAIN_ACTION_NAME]= new Action();
		std::string texture_path= dirname(_json_path)+ "/textures/"+ _name+ ".png";
		actions[MAIN_ACTION_NAME]->_textures.push_back(new ActionTexture(texture_path, 0));
	}

	if (js["sequences"]!= nullptr) {
		for (json::iterator it = js["sequences"].begin(); it != js["sequences"].end(); ++it) {
			auto & sequence_name= it.key();
			auto & l_actions= it.value();
			_sequences[sequence_name]= new ActionSequence();
			for (auto ac : l_actions) {
				_sequences[sequence_name]->_actions.push_back(std::make_pair(actions[ac[0]], ac[1]));
			}
		}
	}
	else {
		_sequences[MAIN_SEQUENCE_NAME]= new ActionSequence();
		_sequences[MAIN_SEQUENCE_NAME]->_actions.push_back(std::make_pair(actions.begin()->second, -1));
	}

	if (js["transitions"]!= nullptr) {
		for (auto transition : js["transitions"]) {
			_transitions.push_back(new SequenceTransition(transition["from"], transition["to"], transition["n_ms"]));
		}
	}
}


unsigned int StaticObjectModel::get_transition(std::string from, std::string to) {
	for (auto transition : _transitions) {
		if (transition->_from== from && transition->_to== to) {
			return transition->_n_ms;
		}
	}
	return 0;
}


std::vector<Action *> StaticObjectModel::get_unduplicated_actions() {
	std::vector<Action *> result;
	for (auto sequence : _sequences) {
		for (auto ac : sequence.second->_actions) {
			if (std::find(result.begin(), result.end(), ac.first)== result.end()) {
				result.push_back(ac.first);
			}
		}
	}
	return result;
}


std::ostream & operator << (std::ostream & os, const StaticObjectModel & model) {
	std::map<ObjectType, std::string> obj2str;
	for (auto x : STR2OBJTYPE) {
		obj2str[x.second]= x.first;
	}
	os << "json_path=" << model._json_path;
	os << " ; type=" << obj2str[model._type];
	os << " ; footprint=" << *model._footprint;
	os << " ; fixed=" << model._fixed;
	return os;
}


// StaticObject --------------------------------------------------------------------
StaticObject::StaticObject() {

}


StaticObject::StaticObject(StaticObjectModel * model, pt_type position, number alpha, pt_type scale) :
	_model(NULL), _scale(pt_type(0.0)), _mass(0.0), _inertia(0.0), _com(pt_type(0.0)), _velocity(pt_type(0.0)),
	_acceleration(pt_type(0.0)), _force(pt_type(0.0)), _alpha(0.0), _angular_velocity(0.0), _angular_acceleration(0.0),
	_torque(0.0), _current_surface(NULL), _previous_surface(NULL),
	_linear_friction_surface(1.0), _angular_friction_surface(1.0),
	_current_action_texture_idx(0), _current_action_force_idx(0),
	_action_force_active(false), _current_sequence_name(MAIN_SEQUENCE_NAME), _next_sequence_name(MAIN_SEQUENCE_NAME),
	_current_action_idx(0), _car_contact(false)
{
	_bbox= new BBox_2D(pt_type(0.0), pt_type(0.0));
	_footprint= new Polygon2D();
	set_model(model);
	reinit(position, alpha, scale);
	//_z= Z_OBJECTS[_model->_type]; // non ; cf https://stackoverflow.com/questions/39085520/cant-i-use-map-operator-with-const-function-keyword
	_z= Z_OBJECTS.at(_model->_type);
}


StaticObject::~StaticObject() {
	delete _footprint;
	delete _bbox;
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

	for (unsigned int i=0; i<N_BUMPS; ++i) {
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
}


Action * StaticObject::get_current_action() {
	//std::cout << _model->_name << " ; " << _current_sequence_name << " ; " << _current_action_idx << "\n";
	return _model->_sequences[_current_sequence_name]->_actions[_current_action_idx].first;
}


void StaticObject::set_current_surface(Material * material, time_point t) {
	_current_surface= material;
	_last_change_surface_t= t;
}


void StaticObject::set_current_sequence(std::string sequence_name, time_point t) {
	if (_next_sequence_name== sequence_name) {
		return;
	}

	else if (_current_sequence_name== sequence_name) {
		_next_sequence_name= _current_sequence_name;
		return;
	}

	else {
		_next_sequence_name= sequence_name;
		_last_sequence_change_t= t;
	}
}


/*void StaticObject::set_current_action(std::string action_name, time_point t) {
	if (get_current_action_name()== action_name) {
		_next_action_name= get_current_action_name();
		return;
	}
	if (_next_action_name== action_name) {
		return;
	}

	_next_action_name= action_name;
	_last_action_change_t= t;
}*/


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


void StaticObject::anim_texture(time_point t) {
	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_action_texture_t).count();
	if (dt> get_current_action()->_textures[_current_action_texture_idx]->_n_ms) {
		_last_action_texture_t= t;
		_current_action_texture_idx++;
		if (_current_action_texture_idx>= get_current_action()->_textures.size()) {
			_current_action_texture_idx= 0;
		}
	}
}


void StaticObject::anim_force(time_point t) {
	if (_action_force_active) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_action_force_t).count();
		if (dt> get_current_action()->_forces[_current_action_force_idx]->_n_ms) {
			_last_action_force_t= t;
			_current_action_force_idx++;
			if (_current_action_force_idx>= get_current_action()->_forces.size()) {
				_current_action_force_idx= 0;
				/*if (!_model->_actions[_current_action_name]->_loop_forces) {
					_action_force_active= false;
				}*/
			}
		}
	}
}


void StaticObject::anim_action(time_point t) {
	/*if (_model->_name== "start") {
		std::cout << _model->_sequences[_current_sequence_name]->_actions[_current_action_idx].second << "\n";
	}*/

	if (_model->_sequences[_current_sequence_name]->_actions[_current_action_idx].second< 0) {
		return;
	}

	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_action_change_t).count();
	if (dt> _model->_sequences[_current_sequence_name]->_actions[_current_action_idx].second) {
		_current_action_idx++;
		if (_current_action_idx>= _model->_sequences[_current_sequence_name]->_actions.size()) {
			_current_action_idx= 0;
		}

		_last_action_change_t= t;
		_current_action_texture_idx= 0;
		_last_action_texture_t= t;
		_current_action_force_idx= 0;
		_last_action_force_t= t;
		_action_force_active= false;
		if (get_current_action()->_forces.size()> 0) {
			_action_force_active= true;
		}
	}
}


void StaticObject::anim_sequence(time_point t) {
	if (_next_sequence_name!= _current_sequence_name) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_sequence_change_t).count();
		
		/*if (_model->_name== "boost") {
			std::cout << _model->_name << " ; " << _current_sequence_name << " -> " << _next_sequence_name << "\n";
			std::cout << _model->get_transition(_current_sequence_name, _next_sequence_name) << "\n";
		}*/
		if (dt> _model->get_transition(_current_sequence_name, _next_sequence_name)) {
			_last_sequence_change_t= t;
			_current_sequence_name= _next_sequence_name;
			_current_action_idx= 0;

			_last_action_change_t= t;
			_current_action_texture_idx= 0;
			_last_action_texture_t= t;
			_current_action_force_idx= 0;
			_last_action_force_t= t;
			_action_force_active= false;
			if (get_current_action()->_forces.size()> 0) {
				_action_force_active= true;
			}
		}
	}
}


void StaticObject::anim(number anim_dt, time_point t) {
	anim_sequence(t);
	anim_action(t);
	anim_texture(t);

	if (_model->_fixed) {
		return;
	}

	anim_surface(t);
	anim_force(t);

	_force= pt_type(0.0);
	if (_action_force_active) {
		_force+= get_current_action()->_forces[_current_action_force_idx]->_direction;
		_force.x+= rand_number(0.0, get_current_action()->_forces[_current_action_force_idx]->_direction_rand.x);
		_force.y+= rand_number(0.0, get_current_action()->_forces[_current_action_force_idx]->_direction_rand.y);
	}
	_force-= _model->_material->_linear_friction* _linear_friction_surface* _velocity; // friction

	// force -> acceleration -> vitesse -> position
	_acceleration= _force/ _mass;
	_velocity+= _acceleration* anim_dt;
	_com+= _velocity* anim_dt;

	// calcul torque == équivalent force pour les rotations
	_torque= 0.0;
	_torque-= _model->_material->_angular_friction* _angular_friction_surface* _angular_velocity; // friction
	
	// torque -> acc angulaire -> vitesse angulaire -> angle
	_angular_acceleration= _torque/ _inertia;
	_angular_velocity+= _angular_acceleration* anim_dt;

	if (!_model->_no_rotation) {
		_alpha+= _angular_velocity* anim_dt;
	}

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


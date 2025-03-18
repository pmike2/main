#include <fstream>
#include <sstream>

#include "json.hpp"

#include "track.h"
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

	// on veut éviter le cas où c'est un angle du décor qui pénètre la voiture car sinon réactions bizarres
	// on attend du coup la situation inverse où un angle de la voiture pénètre le décor, ce qui fera plus naturel
	if (is_pt_in_poly1 && obj1->_model->_type== OBSTACLE_TILE) {
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

	if (abs(impulse)> 10.0) {
		std::cout << "impulse=" << impulse << "\n";
	}

	// on modifie directement la vitesse et la vitesse angulaire
	if (!obj1->_model->_fixed) {
		obj1->_velocity-= (impulse/ obj1->_mass)* axis;
		// facteur multiplicatif pour _angular_velocity pour que ce soit plus dynamique...
		obj1->_angular_velocity-= 2.0* (impulse/ obj1->_inertia)* cross2d(r1, axis);
	}

	if (!obj2->_model->_fixed) {
		obj2->_velocity+= (impulse/ obj2->_mass)* axis;
		obj2->_angular_velocity+= 2.0* (impulse/ obj2->_inertia)* cross2d(r2, axis);
	}

	// peut-être pas nécessaire
	obj1->_acceleration= pt_type(0.0);
	obj1->_angular_acceleration= 0.0;
	obj2->_acceleration= pt_type(0.0);
	obj2->_angular_acceleration= 0.0;

	for (auto obj_pair : std::vector<std::pair<StaticObject *, StaticObject *> >{{obj1, obj2}, {obj2, obj1}}) {
		if (obj_pair.first->_model->_type== HERO_CAR || obj_pair.first->_model->_type== ENNEMY_CAR) {
			Car * car= (Car *)(obj_pair.first);
			if (obj_pair.second->_model->_type== HERO_CAR || obj_pair.second->_model->_type== ENNEMY_CAR) {
				if (car->_thrust> CAR_CAR_COLLISION_THRUST) {
					car->_thrust= CAR_CAR_COLLISION_THRUST;
				}
			}
			else {
				if (car->_thrust> CAR_OBSTACLE_COLLISION_THRUST) {
					car->_thrust= CAR_OBSTACLE_COLLISION_THRUST;
				}
			}
		}
	}

	const number BUMP_INCREMENT= 0.05;
	const number BUMP_MAX= 0.3;
	for (auto obj : std::vector<StaticObject *>{obj1, obj2}) {
		if (obj->_model->_fixed || !obj->_model->_solid) {
			continue;
		}

		std::pair<BBOX_SIDE, BBOX_CORNER>p= get_side(obj->_bbox, obj2->_footprint->_pts[idx_pt]);
		
		int obj_bump_idx= -1;
		if (p.first== BOTTOM_SIDE) {
			if (p.second== BOTTOMLEFT_CORNER) {
				obj_bump_idx= 0;
			}
			else if (p.second== BOTTOMRIGHT_CORNER) {
				obj_bump_idx= 1;
			}
		}
		else if (p.first== RIGHT_SIDE) {
			if (p.second== BOTTOMRIGHT_CORNER) {
				obj_bump_idx= 2;
			}
			else if (p.second== TOPRIGHT_CORNER) {
				obj_bump_idx= 3;
			}
		}
		else if (p.first== TOP_SIDE) {
			if (p.second== TOPRIGHT_CORNER) {
				obj_bump_idx= 4;
			}
			else if (p.second== TOPLEFT_CORNER) {
				obj_bump_idx= 5;
			}
		}
		else if (p.first== LEFT_SIDE) {
			if (p.second== TOPLEFT_CORNER) {
				obj_bump_idx= 6;
			}
			else if (p.second== BOTTOMLEFT_CORNER) {
				obj_bump_idx= 7;
			}
		}
		if (obj_bump_idx< 0) {
			std::cerr << p.first << " ; " << p.second << "\n";
		}
		else {
			obj->_bumps[obj_bump_idx]+= BUMP_INCREMENT* impulse;
			if (obj->_bumps[obj_bump_idx]> BUMP_MAX) {
				obj->_bumps[obj_bump_idx]= BUMP_MAX;
			}
		}
	}
}


// Track -------------------------------------------------------------------------------------
Track::Track() : _start(NULL), _n_laps(0), _mode(TRACK_WAITING), _precount(0), _tire_track_idx(0) {
	load_models();
	_grid= new StaticObjectGrid(DEFAULT_CELL_SIZE, VERTICAL_GRID);

	_tire_tracks= new StaticObject *[N_MAX_TIRE_TRACKS];
	for (int i=0; i<N_MAX_TIRE_TRACKS; ++i) {
		_tire_tracks[i]= new StaticObject(_models["tracks"], pt_type(-1000.0, -1000.0), 0.0, pt_type(1.0, 1.0));
	}
}


Track::~Track() {
	delete _grid;
	for (auto obj : _floating_objects) {
		delete obj;
	}
	_floating_objects.clear();
	delete[] _tire_tracks;
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

	/*for (auto model : _models) {
		std::cout << model.first << " ; ";
	}
	std::cout << "\n";*/
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

	std::sort(_floating_objects.begin(), _floating_objects.end(), [](StaticObject * obj1, StaticObject * obj2) {
		return obj1->_z< obj2->_z;
	});

	sort_cars();
	set_car_names();
}


void Track::set_car_names() {
	std::ifstream ifs("../data/names.txt");
	std::string name;
	std::vector<std::string> names;
	while (ifs >> name) {
		names.push_back(name);
	}

	for (unsigned int idx_car=0; idx_car<_sorted_cars.size(); ++idx_car) {
		Car * car= _sorted_cars[idx_car];
		while (true) {
			int idx_name= rand_int(0, names.size()- 1);
			bool already_taken= false;
			for (unsigned int idx_car_2=0; idx_car_2<idx_car; ++idx_car_2) {
				Car * car_2= _sorted_cars[idx_car_2];
				if (car_2->_name== names[idx_name]) {
					already_taken= true;
					break;
				}
			}
			if (!already_taken) {
				car->_name= names[idx_name];
				break;
			}
		}
	}
}


void Track::start(std::chrono::system_clock::time_point t) {
	_last_precount_t= t;
	_mode= TRACK_PRECOUNT;
	_precount= 3;
}


Car * Track::get_hero() {
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== HERO_CAR) {
			return (Car *)(obj);
		}
	}
	return NULL;
}


void Track::sort_cars() {
	_sorted_cars.clear();

	for (auto obj : _floating_objects) {
		if (obj->_model->_type== HERO_CAR || obj->_model->_type== ENNEMY_CAR) {
			_sorted_cars.push_back((Car *)(obj));
		}
	}
	
	std::sort(_sorted_cars.begin(), _sorted_cars.end(), [this](Car * a, Car * b) {
		if (a->_n_laps< b->_n_laps) {
			return 0;
		}
		else if (a->_n_laps> b->_n_laps) {
			return 1;
		}

		if (a->_finished && !b->_finished) {
			return 0;
		}
		else if (!a->_finished && b->_finished) {
			return 1;
		}
		else if (a->_finished && b->_finished) {
			if (a->_rank< b->_rank) {
				return 1;
			}
			else {
				return 0;
			}
		}

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
	});

	for (unsigned int idx_car=0; idx_car<_sorted_cars.size(); ++idx_car) {
		Car * car= _sorted_cars[idx_car];
		if (!car->_finished) {
			car->_rank= idx_car+ 1;
		}
	}
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


void Track::materials(std::chrono::system_clock::time_point t) {
	for (auto obj1 : _floating_objects) {
		if (obj1->_model->_type!= HERO_CAR && obj1->_model->_type!= ENNEMY_CAR) {
			continue;
		}
		Car * car= (Car*)(obj1);
		
		bool car_in_abnormal_material= false;
		
		for (auto obj2 : _floating_objects) {
			if (obj2->_model->_type!= MATERIAL_FLOATING) {
				continue;
			}
			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
			}
			if (is_pt_inside_poly(obj1->_com, obj2->_footprint)) {
				car->_linear_friction_material= obj2->_model->_linear_friction;
				car->_angular_friction_material= obj2->_model->_angular_friction;
				if (obj2->_model->_name== "puddle") {
					car->_current_tracks= "tracks_water";
					car->_last_track_t= t;
					car_in_abnormal_material= true;
				}
				break;
			}
		}

		for (auto obj2 : _grid->_objects) {
			if (obj2->_model->_type!= MATERIAL_TILE) {
				continue;
			}
			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
			}
			if (is_pt_inside_poly(obj1->_com, obj2->_footprint)) {
				car->_linear_friction_material= obj2->_model->_linear_friction;
				car->_angular_friction_material= obj2->_model->_angular_friction;
				if (obj2->_model->_name.find("sand")!= std::string::npos) {
					car->_current_tracks= "tracks_sand";
					car->_last_track_t= t;
					car_in_abnormal_material= true;
				}
				break;
			}
		}

		if (!car_in_abnormal_material && car->_current_tracks!= "tracks") {
			auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- car->_last_track_t).count();
			if (dt> ABNORMAL_TRACKS_DT) {
				car->_current_tracks= "tracks";
				car->_linear_friction_material= 1.0;
				car->_angular_friction_material= 1.0;
			}
			else {
				const number LINEAR_FRICTION_MATERIAL_INCREMENT= 0.1;
				const number ANGULAR_FRICTION_MATERIAL_INCREMENT= 0.1;
				if (car->_linear_friction_material> 1.0+ LINEAR_FRICTION_MATERIAL_INCREMENT) {
					car->_linear_friction_material-= LINEAR_FRICTION_MATERIAL_INCREMENT;
				}
				else if (car->_linear_friction_material< 1.0- LINEAR_FRICTION_MATERIAL_INCREMENT) {
					car->_linear_friction_material+= LINEAR_FRICTION_MATERIAL_INCREMENT;
				}
				if (car->_angular_friction_material> 1.0+ ANGULAR_FRICTION_MATERIAL_INCREMENT) {
					car->_angular_friction_material-= ANGULAR_FRICTION_MATERIAL_INCREMENT;
				}
				else if (car->_angular_friction_material< 1.0- ANGULAR_FRICTION_MATERIAL_INCREMENT) {
					car->_angular_friction_material+= ANGULAR_FRICTION_MATERIAL_INCREMENT;
				}
			}
		}
	}
}


void Track::tire_tracks(std::chrono::system_clock::time_point t) {
	std::vector<Car * > drifting_cars;
	for (auto obj : _floating_objects) {
		if (obj->_model->_type!= HERO_CAR && obj->_model->_type!= ENNEMY_CAR) {
			continue;
		}
		
		Car * car= (Car *)(obj);

		if (car->_drift || car->_current_tracks!= "tracks") {
			drifting_cars.push_back(car);
		}
	}
	
	for (auto car : drifting_cars) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- car->_last_drift_t).count();
		if (dt> TIRE_TRACKS_DELTA_T) {
			_tire_tracks[_tire_track_idx]->set_model(_models[car->_current_tracks]);
			_tire_tracks[_tire_track_idx]->reinit(car->_com, car->_alpha, car->_scale);
			_tire_track_idx++;
			if (_tire_track_idx>= N_MAX_TIRE_TRACKS) {
				_tire_track_idx= 0;
			}

			car->_last_drift_t= t;
		}
	}
}


void Track::checkpoints(std::chrono::system_clock::time_point t) {
	for (auto obj : _floating_objects) {
		if (obj->_model->_type!= HERO_CAR && obj->_model->_type!= ENNEMY_CAR) {
			continue;
		}
		
		Car * car= (Car *)(obj);

		if (car->_finished) {
			continue;
		}

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
				
				if (car->_n_laps== _n_laps+ 1) {
					car->_finished= true;
					if (car->_model->_type== HERO_CAR) {
						_mode= TRACK_FINISHED;
					}
				}
				else {
					car->_last_start_t= t;
					if (car->_n_laps> 1) {
						car->_lap_times.push_back(0.0);
					}
				}
			}
			car->_next_checkpoint= car->_next_checkpoint->_next;
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

	if (dist> 1.0) {
		car->_thrust+= model->_thrust_increment;
		if (car->_thrust> model->_max_thrust) {
			car->_thrust= model->_max_thrust;
		}
	}
	/*else if (dist< 0.5) {
		car->_thrust-= model->_brake_increment;
		if (car->_thrust< -1.0* model->_max_brake) {
			car->_thrust= -1.0* model->_max_brake;
		}
	}*/
	else {
		if (car->_thrust< -1.0* model->_thrust_decrement) {
			car->_thrust+= model->_thrust_decrement;
		}
		else if (car->_thrust> model->_thrust_decrement) {
			car->_thrust-= model->_thrust_decrement;
		}
		else {
			car->_thrust= 0.0;
		}
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


void Track::lap_time(std::chrono::system_clock::time_point t) {
	for (auto car  : _sorted_cars) {
		if (car->_n_laps== 0) {
			// car n'a pas encore passé le 1er start
			if (car->_lap_times.size()== 0) {
				car->_lap_times.push_back(0.0);
			}
			continue;
		}
		
		if (car->_finished) {
			continue;
		}

		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- car->_last_start_t).count();
		car->_lap_times[car->_lap_times.size()- 1]= number(dt)/ 1000.0;
	}
}


void Track::anim(std::chrono::system_clock::time_point t, InputState * input_state) {
	if (_mode== TRACK_PRECOUNT) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_precount_t).count();
		if (dt> 1000) {
			_precount--;
			_last_precount_t= t;
			if (_precount== 0) {
				_mode= TRACK_LIVE;
			}
		}
	}

	Car * hero= get_hero();

	if (_mode== TRACK_LIVE) {
		if (input_state->_is_joystick) {
			hero->preanim_joystick(input_state->_joystick_a, input_state->_joystick_b, input_state->_joystick);
		}
		else {
			hero->preanim_keys(input_state->_keys[SDLK_LEFT], input_state->_keys[SDLK_RIGHT], input_state->_keys[SDLK_DOWN], input_state->_keys[SDLK_UP]);
		}
	}
	else if (_mode== TRACK_FINISHED) {
		hero->_thrust= hero->_wheel= 0.0;
	}

	// finalement un dt fixe fait plus propre
	/*auto dt_n_ms= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
	number dt= number(dt_n_ms)/ 1000.0;
	_last_anim_t= t;*/
	number dt= 0.05;

	if (_mode== TRACK_LIVE || _mode== TRACK_FINISHED) {
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
		checkpoints(t);
		materials(t);
		tire_tracks(t);
		sort_cars();
		lap_time(t);
	}
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

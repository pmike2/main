#include <fstream>
#include <sstream>
#include <iomanip>

#include "json.hpp"

#include "utile.h"
#include "collision.h"
#include "track.h"


using json = nlohmann::json;


// Track -------------------------------------------------------------------------------------
Track::Track() : _start(NULL), _n_laps(0), _mode(TRACK_WAITING), _precount(0) {
	load_models();
	_grid= new StaticObjectGrid(DEFAULT_CELL_SIZE, VERTICAL_GRID);
}


Track::~Track() {
	delete _grid;
	for (auto obj : _floating_objects) {
		delete obj;
	}
	_floating_objects.clear();
	for (auto model : _models) {
		delete model.second;
	}
	_models.clear();
	for (auto material : _materials) {
		delete material.second;
	}
	_materials.clear();
}


void Track::load_models() {
	bool verbose= false;

	std::vector<std::string> jsons_material= list_files("../data/materials", "json");
	for (auto json_path : jsons_material) {
		if (verbose) {std::cout << "chgmt material : " << json_path << "\n";}
		_materials[basename(json_path)]= new Material(json_path);
	}

	std::vector<std::string> jsons_floating= list_files("../data/static_objects/floating_objects", "json");
	for (auto json_path : jsons_floating) {
		if (verbose) {std::cout << "chgmt floating : " << json_path << "\n";}
		_models[basename(json_path)]= new StaticObjectModel(json_path);
	}

	std::vector<std::string> jsons_tiles= list_files("../data/static_objects/tiles", "json");
	for (auto json_path : jsons_tiles) {
		if (verbose) {std::cout << "chgmt tile : " << json_path << "\n";}
		_models[basename(json_path)]= new StaticObjectModel(json_path);
	}

	std::vector<std::string> jsons_cars= list_files("../data/cars", "json");
	for (auto json_path : jsons_cars) {
		if (verbose) {std::cout << "chgmt car : " << json_path << "\n";}
		_models[basename(json_path)]= new CarModel(json_path);
	}

	// assignation des matériaux aux modèles
	for (auto model : _models) {
		std::string matname= model.second->_material_name;
		if (_materials.find(matname)== _materials.end()) {
			std::cerr << "matériau " << matname << " inexistant\n";
			continue;
		}
		if (verbose) {std::cout << "modele " << model.second->_name << " -> material " << matname << "\n";}
		model.second->_material= _materials[matname];
	}
}


void Track::load_json(std::string json_path) {
	_current_json_path= json_path;

	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	// nombre de tours
	_n_laps= js["n_laps"];

	// temps records
	_best_lap.clear();
	for (auto record : js["best_lap"]) {
		_best_lap.push_back(std::make_pair(record["name"], record["time"]));
	}
	std::sort(_best_lap.begin(), _best_lap.end(), [](std::pair<std::string, number> a, std::pair<std::string, number> b) {
		return a.second< b.second; 
	});
	
	_best_overall.clear();
	for (auto record : js["best_overall"]) {
		_best_overall.push_back(std::make_pair(record["name"], record["time"]));
	}
	std::sort(_best_overall.begin(), _best_overall.end(), [](std::pair<std::string, number> a, std::pair<std::string, number> b) {
		return a.second< b.second; 
	});

	// création grille et remplissage des tiles
	_grid->_cell_size= js["cell_size"];
	set_all("empty", js["width"], js["height"]);
	unsigned int compt= 0;
	for (auto tilename : js["tiles"]) {
		set_tile(tilename, compt);
		compt++;
	}

	// nettoyage des objets flottants
	for (auto obj : _floating_objects) {
		delete obj;
	}
	_floating_objects.clear();

	// reinit du héros
	_hero= NULL;

	// ajout des objets flottants
	std::vector<std::pair<CheckPoint *, unsigned int> > checkpoints;
	for (auto object : js["floating_objects"]) {
		std::string model_name= object["name"];
		pt_type position= pt_type(object["position"][0], object["position"][1]);
		number alpha= object["alpha"];
		pt_type scale= pt_type(object["scale"][0], object["scale"][1]);
		if (_models[model_name]->_type== HERO_CAR || _models[model_name]->_type== ENNEMY_CAR) {
			Car * car= new Car((CarModel *)(_models[model_name]), position, alpha, scale);
			_floating_objects.push_back(car);
			if (_models[model_name]->_type== HERO_CAR) {
				_hero= car;
			}
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

	// tri des checkpoints en fonction de leurs indices
	std::sort(checkpoints.begin(), checkpoints.end(), [](std::pair<CheckPoint *, unsigned int> a, std::pair<CheckPoint *, unsigned int> b) {
		return a.second< b.second; 
	});
	
	// liens entre checkpoints
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
	
	// assignation start à toutes les voitures
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== HERO_CAR || obj->_model->_type== ENNEMY_CAR) {
			Car * car= (Car *)(obj);
			car->_next_checkpoint= _start;
		}
	}

	// tri des objets par leurs z pour affichage
	std::sort(_floating_objects.begin(), _floating_objects.end(), [](StaticObject * obj1, StaticObject * obj2) {
		return obj1->_z< obj2->_z;
	});

	sort_cars();
	set_car_names();
}


void Track::save_json(std::string json_path) {
	std::ofstream ofs(json_path);
	json js;

	js["width"]= _grid->_width;
	js["height"]= _grid->_height;
	js["cell_size"]= _grid->_cell_size;

	js["n_laps"]= _n_laps;

	js["tiles"]= json::array();
	for (auto tile : _grid->_objects) {
		js["tiles"].push_back(basename(tile->_model->_json_path));
	}
	
	js["floating_objects"]= json::array();
	for (auto obj : _floating_objects) {
		json js_obj;
		js_obj["name"]= basename(obj->_model->_json_path);
		js_obj["position"]= json::array();
		js_obj["position"].push_back(obj->_com.x);
		js_obj["position"].push_back(obj->_com.y);
		js_obj["alpha"]= obj->_alpha;
		js_obj["scale"]= json::array();
		js_obj["scale"].push_back(obj->_scale.x);
		js_obj["scale"].push_back(obj->_scale.y);

		if (obj->_model->_type== CHECKPOINT || obj->_model->_type== START) {
			CheckPoint * checkpoint= (CheckPoint *)(obj);
			js_obj["idx_checkpoint"]= get_checkpoint_index(checkpoint);
		}
		
		js["floating_objects"].push_back(js_obj);
	}

	ofs << std::setw(4) << js << "\n";
	ofs.close();

	_current_json_path= json_path;
	write_records();
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

	for (auto obj : _floating_objects) {
		// au début tous les objets sont sur la surface par défaut road
		obj->set_current_surface(_materials["road"], t);
		obj->_previous_surface= _materials["road"];
		// au début tous les objets sont dans leur action par défaut
		obj->set_current_action(MAIN_ACTION_NAME, t);
	}
	// set des tire_tracks à road
	for (auto car : _sorted_cars) {
		car->_tire_track_texture_idx= _materials["road"]->_tire_track_texture_idx;
	}

	_start->set_current_action("active", t);
}


void Track::end() {
	_mode= TRACK_FINISHED;

	// record au tour
	for (auto car : _sorted_cars) {
		for (auto t : car->_lap_times) {
			_best_lap.push_back(std::make_pair(car->_name, t));
		}
	}
	std::sort(_best_lap.begin(), _best_lap.end(), [](std::pair<std::string, number> a, std::pair<std::string, number> b) {
		return a.second< b.second; 
	});
	_best_lap.resize(3);

	_new_best_lap= false;
	for (auto t : _hero->_lap_times) {
		if (t<= _best_lap[0].second) {
			_new_best_lap= true;
			break;
		}
	}

	// record au total
	for (auto car : _sorted_cars) {
		_best_overall.push_back(std::make_pair(car->_name, car->_total_time));
	}
	std::sort(_best_overall.begin(), _best_overall.end(), [](std::pair<std::string, number> a, std::pair<std::string, number> b) {
		return a.second< b.second; 
	});
	_best_overall.resize(3);

	_new_best_overall= false;
	if (_hero->_total_time<= _best_overall[0].second) {
		_new_best_overall= true;
	}

	// écriture json
	write_records();
}


void Track::write_records() {
	std::ifstream ifs(_current_json_path);
	json js= json::parse(ifs);
	ifs.close();

	js["best_lap"]= json::array();
	std::cout << "ok\n";
	for (auto best : _best_lap) {
		std::cout << best.first << " ; " << best.second << "\n";
		json j;
		j["name"]= best.first;
		j["time"]= best.second;
		js["best_lap"].push_back(j);
	}

	js["best_overall"]= json::array();
	for (auto best : _best_overall) {
		json j;
		j["name"]= best.first;
		j["time"]= best.second;
		js["best_overall"].push_back(j);
	}

	std::ofstream ofs(_current_json_path);
	ofs << std::setw(4) << js << "\n";
	ofs.close();
}


void Track::sort_cars() {
	_sorted_cars.clear();

	// ajout des objets de type voiture
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== HERO_CAR || obj->_model->_type== ENNEMY_CAR) {
			_sorted_cars.push_back((Car *)(obj));
		}
	}
	
	// tri des voitures
	std::sort(_sorted_cars.begin(), _sorted_cars.end(), [this](Car * a, Car * b) {
		// sur le nombre de tours
		if (a->_n_laps< b->_n_laps) {
			return 0;
		}
		else if (a->_n_laps> b->_n_laps) {
			return 1;
		}

		// sur le fait qu'une voiture ait fini ou non, et sur son rank
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

		// sur le checkpoint courant
		unsigned int idx_a= get_checkpoint_index(a->_next_checkpoint->_previous);
		unsigned int idx_b= get_checkpoint_index(b->_next_checkpoint->_previous);
		if (idx_a< idx_b) {
			return 0;
		}
		else if (idx_a> idx_b) {
			return 1;
		}
		else {
			// sur la distance au prochain checkpoint
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

	// on assigne le rank pour toutes les voitures encore en course
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


void Track::surfaces(std::chrono::system_clock::time_point t) {
	for (auto obj1 : _floating_objects) {
		if (obj1->_model->_type!= HERO_CAR && obj1->_model->_type!= ENNEMY_CAR) {
			continue;
		}
		Car * car= (Car*)(obj1);
		bool abnormal_surface= false;
		
		for (auto obj2 : _floating_objects) {
			if (obj2->_model->_type!= SURFACE_FLOATING) {
				continue;
			}
			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
			}
			if (is_pt_inside_poly(obj1->_com, obj2->_footprint)) {
				if (car->_current_surface!= obj2->_model->_material) {
					car->set_current_surface(obj2->_model->_material, t);
				}
				abnormal_surface= true;
				break;
			}
		}

		for (auto obj2 : _grid->_objects) {
			if (obj2->_model->_type!= SURFACE_TILE) {
				continue;
			}
			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
			}
			if (is_pt_inside_poly(obj1->_com, obj2->_footprint)) {
				if (car->_current_surface!= obj2->_model->_material) {
					car->set_current_surface(obj2->_model->_material, t);
				}
				abnormal_surface= true;
				break;
			}
		}

		if (!abnormal_surface && car->_current_surface->_name!= "road") {
			car->set_current_surface(_materials["road"], t);
		}
	}
}


void Track::repair(std::chrono::system_clock::time_point t) {
	std::map<StaticObject *, bool> repairs;
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== REPAIR) {
			repairs[obj]= false;
		}
	}

	for (auto obj1 : _floating_objects) {
		if (obj1->_model->_type!= HERO_CAR && obj1->_model->_type!= ENNEMY_CAR) {
			continue;
		}
		Car * car= (Car*)(obj1);
		
		for (auto obj2 : _floating_objects) {
			if (obj2->_model->_type!= REPAIR) {
				continue;
			}
			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
			}
			if (is_pt_inside_poly(obj1->_com, obj2->_footprint)) {
				repairs[obj2]= true;
				for (int i=0; i<N_BUMPS; ++i) {
					car->_bumps[i]-= REPAIR_INCREMENT;
					if (car->_bumps[i]< 0.0) {
						car->_bumps[i]= 0.0;
					}
				}
				break;
			}
		}
	}

	for (auto x: repairs) {
		if (x.second && x.first->_current_action_name!= "active") {
			x.first->set_current_action("active", t);
		}
		else if (!x.second && x.first->_current_action_name!= MAIN_ACTION_NAME) {
			x.first->set_current_action(MAIN_ACTION_NAME, t);
		}
	}
}


void Track::boost(std::chrono::system_clock::time_point t) {
	std::map<StaticObject *, bool> boosts;
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== BOOST) {
			boosts[obj]= false;
		}
	}

	for (auto obj1 : _floating_objects) {
		if (obj1->_model->_type!= HERO_CAR && obj1->_model->_type!= ENNEMY_CAR) {
			continue;
		}
		Car * car= (Car*)(obj1);
		
		for (auto obj2 : _floating_objects) {
			if (obj2->_model->_type!= BOOST) {
				continue;
			}
			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
			}
			if (is_pt_inside_poly(obj1->_com, obj2->_footprint)) {
				boosts[obj2]= true;
				auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- car->_last_boost_t).count();
				if (dt> BOOST_DELTA_T_MS) {
					car->_last_boost_t= t;
					car->_velocity+= rot(pt_type(0.0, BOOST_AMPLITUDE), obj2->_alpha);
				}
				break;
			}
		}
	}

	for (auto x : boosts) {
		if (x.second && x.first->_current_action_name!= "active") {
			x.first->set_current_action("active", t);
		}
		else if (!x.second && x.first->_current_action_name!= MAIN_ACTION_NAME) {
			x.first->set_current_action(MAIN_ACTION_NAME, t);
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
					car->_total_time= 0.0;
					for (auto lap_time : car->_lap_times) {
						car->_total_time+= lap_time;
					}
					if (car->_model->_type== HERO_CAR) {
						end();
					}
				}
				else {
					car->_last_start_t= t;
					if (car->_n_laps> 1) {
						car->_lap_times.push_back(0.0);
					}
				}
			}

			if (car->_model->_type== HERO_CAR) {
				if (car->_next_checkpoint!= _start) {
					car->_next_checkpoint->set_current_action(MAIN_ACTION_NAME, t);
				}
				car->_next_checkpoint->_next->set_current_action("active", t);
			}
			
			car->_next_checkpoint= car->_next_checkpoint->_next;
		}
	}
}


void Track::checkpoint_ia(Car * car) {
	if (car->_finished) {
		car->_thrust= car->_wheel= 0.0;
		return;
	}

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

	if (_mode== TRACK_LIVE) {
		if (input_state->_is_joystick) {
			_hero->preanim_joystick(input_state->_joystick_a, input_state->_joystick_b, input_state->_joystick);
		}
		else {
			_hero->preanim_keys(input_state->_keys[SDLK_LEFT], input_state->_keys[SDLK_RIGHT], input_state->_keys[SDLK_DOWN], input_state->_keys[SDLK_UP]);
		}
	}
	else if (_mode== TRACK_FINISHED) {
		_hero->_thrust= _hero->_wheel= 0.0;
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
				car->anim(dt, t);
			}
			else {
				obj->anim(dt, t);
			}
		}

		all_collision();
		checkpoints(t);
		surfaces(t);
		repair(t);
		boost(t);
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


void Track::clear_floating_objects() {
	for (auto obj : _floating_objects) {
		delete obj;
	}
	_floating_objects.clear();
}


std::ostream & operator << (std::ostream & os, const Track & track) {
	os << "width=" << track._grid->_width << " ; height=" << track._grid->_height;
	return os;
}

#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>

#include "json.hpp"

#include "utile.h"
#include "collision.h"
#include "track.h"


using json = nlohmann::json;

// TrackInfo ---------------------------------------------------------------------------------
TrackInfo::TrackInfo() {

}


/*TrackInfo::TrackInfo(std::string json_path) {
	parse_json(json_path);
}*/


TrackInfo::~TrackInfo() {

}


void TrackInfo::parse_json(std::string json_path) {
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
}


// Track -------------------------------------------------------------------------------------
Track::Track() : _start(NULL), _mode(TRACK_WAITING), _precount(0) {
	load_models();
	_grid= new StaticObjectGrid(DEFAULT_CELL_SIZE, VERTICAL_GRID);
	_info= new TrackInfo();
}


Track::~Track() {
	delete _info;
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

	std::vector<std::string> jsons_drivers= list_files("../data/drivers", "json");
	for (auto json_driver : jsons_drivers) {
		if (verbose) {std::cout << "chgmt driver : " << json_driver << "\n";}
		_drivers.push_back(new Driver(json_driver));
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

	_info->parse_json(json_path);

	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	// création grille et remplissage des tiles
	_grid->_cell_size= js["cell_size"];
	set_all("empty", js["width"], js["height"]);
	unsigned int compt= 0;
	for (auto tilename : js["tiles"]) {
		set_tile(tilename, compt);
		compt++;
	}

	// reinit du héros
	_hero= NULL;

	// reinit des voitures
	_sorted_cars.clear();

	// nettoyage des objets flottants
	for (auto obj : _floating_objects) {
		delete obj;
	}
	_floating_objects.clear();

	// ajout des objets flottants
	std::vector<std::pair<CheckPoint *, unsigned int> > checkpoints;
	for (auto object : js["floating_objects"]) {
		std::string model_name= object["name"];
		pt_type position= pt_type(object["position"][0], object["position"][1]);
		number alpha= object["alpha"];
		pt_type scale= pt_type(object["scale"][0], object["scale"][1]);
		if (_models[model_name]->_type== CHECKPOINT || _models[model_name]->_type== START) {
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

	// placement des voitures au start
	place_cars();

	_floating_objects.push_back(new StaticObject(_models["direction_help"], pt_type(0.0), 0.0, pt_type(DIRECTION_HELP_SCALE)));
}


void Track::save_json(std::string json_path) {
	std::ofstream ofs(json_path);
	json js;

	js["width"]= _grid->_width;
	js["height"]= _grid->_height;
	js["cell_size"]= _grid->_cell_size;

	js["n_laps"]= _info->_n_laps;

	js["tiles"]= json::array();
	for (auto tile : _grid->_objects) {
		js["tiles"].push_back(basename(tile->_model->_json_path));
	}
	
	js["floating_objects"]= json::array();
	for (auto obj : _floating_objects) {
		// on ne sauvegarde pas les voitures car elles sont replacées automatiquement par rapport au start
		if (obj->_model->_type== CAR) {
			continue;
		}

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


/*void Track::set_car_names() {
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
}*/


void Track::set_hero(unsigned int idx_driver) {
	if (idx_driver>= _drivers.size()) {
		std::cerr << "Track::set_hero : " << idx_driver << ">=" << _drivers.size() << "\n";
		return;
	}
	_hero= NULL;
	for (auto car : _sorted_cars) {
		if (car->_model->_name== _drivers[idx_driver]->_name) {
			_hero= car;
			break;
		}
	}
	if (_hero== NULL) {
		std::cerr << "set_hero : " << _drivers[idx_driver]->_name << " non trouvé\n";
	}
}


void Track::place_cars() {
	// calcul des positions par rapport à la ligne de départ
	pt_type start_dir= rot(pt_type(0.0, 1.0), _start->_alpha);
	pt_type start_right= rot(start_dir, -0.5* M_PI);
	pt_type first_row_center= _start->_com- (0.5* _start->_scale.y+ CAR_PLACEMENT._first_row_dist_start)* start_dir;
	std::vector<pt_type> positions;
	
	bool positions_filled= false;
	for (int idx_row=0; idx_row< CAR_PLACEMENT._n_max_rows; ++idx_row) {
		if (positions_filled) {
			break;
		}
		for (int i=0; i<CAR_PLACEMENT._n_cars_per_row; ++i) {
			pt_type pos= first_row_center- number(idx_row)* CAR_PLACEMENT._row_dist* start_dir
				+ (-0.5* number(CAR_PLACEMENT._n_cars_per_row- 1)+ i)* CAR_PLACEMENT._neighbour_dist* start_right;
			positions.push_back(pos);
			if (positions.size()== _drivers.size()) {
				positions_filled= true;
				break;
			}
		}
	}

	// randomize des positions pour varier (_drivers est classé par nom)
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(positions.begin(), positions.end(), g);
	
	// création des voitures
	for (unsigned int idx_driver=0; idx_driver<_drivers.size(); ++idx_driver) {
		CarModel * model= (CarModel *)(_models[_drivers[idx_driver]->_name]);
		Car * car= new Car(model, positions[idx_driver], _start->_alpha, pt_type(1.0));
		car->_driver= _drivers[idx_driver];
		_floating_objects.push_back(car);
		_sorted_cars.push_back(car);
	}
	
	// assignation start à toutes les voitures
	for (auto car : _sorted_cars) {
		car->_next_checkpoint= _start;
	}

	// tri avant début
	sort_cars();
}


void Track::reinit_drivers(time_point t, bool set_normal_expression) {
	for (auto driver : _drivers) {
		driver->reinit(t, set_normal_expression);
	}
}


void Track::start(time_point t) {
	_last_precount_t= t;
	_mode= TRACK_PRECOUNT;
	_precount= 3;

	// tri des objets par leurs z pour affichage
	std::sort(_floating_objects.begin(), _floating_objects.end(), [](StaticObject * obj1, StaticObject * obj2) {
		return obj1->_z< obj2->_z;
	});

	for (auto obj : _floating_objects) {
		// au début tous les objets sont sur la surface par défaut road
		obj->set_current_surface(_materials["road"], t);
		obj->_previous_surface= _materials["road"];
		// au début tous les objets sont dans leur action par défaut
		obj->set_current_sequence(MAIN_SEQUENCE_NAME, t);
	}
	
	// set des tire_tracks à road
	for (auto car : _sorted_cars) {
		car->_tire_track_texture_idx= _materials["road"]->_tire_track_texture_idx;
	}

	// reinit expressions drivers
	reinit_drivers(t, true);
}


void Track::end() {
	_mode= TRACK_FINISHED;

	// record au tour
	for (auto car : _sorted_cars) {
		//for (auto t : car->_lap_times) {
		for (unsigned int i=0; i<car->_lap_times.size(); ++i) {
			if (i< car->_lap_times.size()- 1 || car->_finished) {
				_info->_best_lap.push_back(std::make_pair(car->_driver->_name, car->_lap_times[i]));
			}
		}
	}
	std::sort(_info->_best_lap.begin(), _info->_best_lap.end(), [](std::pair<std::string, number> a, std::pair<std::string, number> b) {
		return a.second< b.second; 
	});
	_info->_best_lap.resize(3);

	_new_best_lap= false;
	for (auto t : _hero->_lap_times) {
		if (t<= _info->_best_lap[0].second) {
			_new_best_lap= true;
			break;
		}
	}

	// record au total
	for (auto car : _sorted_cars) {
		if (car->_finished) {
			_info->_best_overall.push_back(std::make_pair(car->_driver->_name, car->_total_time));
		}
	}
	std::sort(_info->_best_overall.begin(), _info->_best_overall.end(), [](std::pair<std::string, number> a, std::pair<std::string, number> b) {
		return a.second< b.second; 
	});
	_info->_best_overall.resize(3);

	_new_best_overall= false;
	if (_hero->_total_time<= _info->_best_overall[0].second) {
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
	for (auto best : _info->_best_lap) {
		json j;
		j["name"]= best.first;
		j["time"]= best.second;
		js["best_lap"].push_back(j);
	}

	js["best_overall"]= json::array();
	for (auto best : _info->_best_overall) {
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
	const int A_WINS_OVER_B= 1;
	const int B_WINS_OVER_A= 0;

	// tri des voitures
	std::sort(_sorted_cars.begin(), _sorted_cars.end(), [this](Car * a, Car * b) {

		// si 1 des 2 a fini et pas l'autre c'est clair
		if (a->_finished && !b->_finished) {
			return A_WINS_OVER_B;
		}
		else if (!a->_finished && b->_finished) {
			return B_WINS_OVER_A;
		}
		// si les 2 ont fini c'est sur leur classement
		else if (a->_finished && b->_finished) {
			if (a->_rank< b->_rank) {
				return A_WINS_OVER_B;
			}
			else {
				return B_WINS_OVER_A;
			}
		}

		// si les 2 viennent de finir c'est sur celui qui a un overlap + grand avec la ligne d'arrivée
		if (a->_just_finished && b->_just_finished) {
			if (a->_overlap_finish> b->_overlap_finish) {
				return A_WINS_OVER_B;
			}
			else {
				return B_WINS_OVER_A;
			}
		}
		
		// à partir de là les 2 cars n'ont pas fini
		// sur le nombre de tours
		if (a->_n_laps> b->_n_laps) {
			return A_WINS_OVER_B;
		}
		else if (a->_n_laps< b->_n_laps) {
			return B_WINS_OVER_A;
		}

		// à partir de là les 2 cars en sont au même tour
		// sur le checkpoint courant
		unsigned int idx_a= get_checkpoint_index(a->_next_checkpoint->_previous);
		unsigned int idx_b= get_checkpoint_index(b->_next_checkpoint->_previous);
		if (idx_a> idx_b) {
			return A_WINS_OVER_B;
		}
		else if (idx_a< idx_b) {
			return B_WINS_OVER_A;
		}

		// s'ils en sont au même chkpt, on regarde la distance au prochain checkpoint
		number dist_a= (a->_com.x- a->_next_checkpoint->_com.x)* (a->_com.x- a->_next_checkpoint->_com.x)+ (a->_com.y- a->_next_checkpoint->_com.y)* (a->_com.y- a->_next_checkpoint->_com.y);
		number dist_b= (b->_com.x- b->_next_checkpoint->_com.x)* (b->_com.x- b->_next_checkpoint->_com.x)+ (b->_com.y- b->_next_checkpoint->_com.y)* (b->_com.y- b->_next_checkpoint->_com.y);
		if (dist_a< dist_b) {
			return A_WINS_OVER_B;
		}
		else {
			return B_WINS_OVER_A;
		}
	});

	// on assigne le rank pour toutes les voitures encore en course
	for (unsigned int idx_car=0; idx_car<_sorted_cars.size(); ++idx_car) {
		Car * car= _sorted_cars[idx_car];
		if (!car->_finished) {
			car->_rank= idx_car+ 1;
		}
		if (idx_car== 0) {
			car->_driver->_happy= true;
		}
		else if (idx_car>= 3) {
			car->_driver->_sad= true;
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


void Track::anim_objects(time_point t) {
	// finalement un dt fixe fait plus propre
	/*auto dt_n_ms= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
	number dt= number(dt_n_ms)/ 1000.0;
	_last_anim_t= t;*/
	number dt= 0.05;

	for (auto obj : _floating_objects) {
		if (obj->_model->_type== CAR) {
			Car * car= (Car *)(obj);

			// IA
			if (car!= _hero) {
				//car->random_ia();
				checkpoint_ia(car);
			}

			// animation des objets
			car->anim(dt, t);
		}
		else {
			obj->anim(dt, t);
		}
	}
}


void Track::reinit_car_contact() {
	for (auto obj : _floating_objects) {
		obj->_car_contact= false;
	}
}


void Track::collisions(time_point t) {
	_collisions.clear();
	pt_type position(0.0);

	std::vector<Car *> collided_cars;

	for (unsigned int idx_obj_1=0; idx_obj_1<_floating_objects.size()- 1; ++idx_obj_1) {
		for (unsigned int idx_obj_2=idx_obj_1+ 1; idx_obj_2<_floating_objects.size(); ++idx_obj_2) {
			StaticObject * obj1= _floating_objects[idx_obj_1];
			StaticObject * obj2= _floating_objects[idx_obj_2];
			if (collision(obj1, obj2, position)) {
				_collisions.push_back(position);
				
				if (obj1->_model->_type== CAR) {
					collided_cars.push_back((Car *)(obj1));
					obj2->_car_contact= true;
				}
				if (obj2->_model->_type== CAR) {
					collided_cars.push_back((Car *)(obj2));
					obj1->_car_contact= true;
				}
			}
		}
	}
	
	for (auto object : _floating_objects) {
		for (auto tile : _grid->_objects) {
			if (collision(object, tile, position)) {
				_collisions.push_back(position);
				if (object->_model->_type== CAR) {
					collided_cars.push_back((Car *)(object));
				}
			}
		}
	}

	for (auto car : collided_cars) {
		car->_driver->_angry= true;
	}

	for (auto car : _sorted_cars) {
		number average_bump= 0.0;
		for (int i=0; i<N_BUMPS; ++i) {
			average_bump+= car->_bumps[i];
		}
		average_bump/= N_BUMPS;
		if (average_bump> BUMP_TIRED_THRESHOLD) {
			car->_driver->_tired= true;
		}
	}
}


void Track::surfaces(time_point t) {
	for (auto obj1 : _floating_objects) {
		if (obj1->_model->_type!= CAR) {
			continue;
		}
		Car * car= (Car*)(obj1);
		bool abnormal_surface= false;
		
		// est-ce que Car est sur une surface flottante genre flaque
		for (auto obj2 : _floating_objects) {
			if (obj2->_model->_type!= SURFACE_FLOATING) {
				continue;
			}
			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
			}
			if (is_pt_inside_poly(obj1->_com, obj2->_footprint)) {
				obj2->_car_contact= true;
				if (car->_current_surface!= obj2->_model->_material) {
					car->set_current_surface(obj2->_model->_material, t);
				}
				abnormal_surface= true;
				break;
			}
		}

		// est-ce que Car est sur un tile différent de road
		for (auto obj2 : _grid->_objects) {
			if (obj2->_model->_type!= SURFACE_TILE) {
				continue;
			}
			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
			}
			if (is_pt_inside_poly(obj1->_com, obj2->_footprint)) {
				obj2->_car_contact= true;
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


void Track::repair(time_point t) {
	for (auto obj1 : _floating_objects) {
		if (obj1->_model->_type!= CAR) {
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
				// réparation active
				obj2->_car_contact= true;
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
}


void Track::boost(time_point t) {
	for (auto obj1 : _floating_objects) {
		if (obj1->_model->_type!= CAR) {
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
				// boost actif
				obj2->_car_contact= true;
				auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- car->_last_boost_t).count();
				if (dt> BOOST_DELTA_T_MS) {
					car->_last_boost_t= t;
					car->_velocity+= rot(pt_type(0.0, BOOST_AMPLITUDE), obj2->_alpha);
				}
				break;
			}
		}
	}
}


void Track::checkpoints(time_point t) {
	for (auto obj : _floating_objects) {
		if (obj->_model->_type!= CAR) {
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
		
		// intersection de car avec son checkpoint
		if (is_inter) {
			// fin d'un tour ou passage du start du début de piste
			if (car->_next_checkpoint== _start) {
				car->_n_laps++;
				// fin de la piste
				if (car->_n_laps== _info->_n_laps+ 1) {
					// _just_finished et _overlap_finish permettent de départager 2 voitures qui ont passé la ligne d'arrivée
					// lors de la même boucle d'animation
					car->_just_finished= true;
					car->_overlap_finish= overlap;
				}
				// on fige le temps du tour
				else {
					car->_last_start_t= t;
					// si car a déjà fait au - un tour on ajoute un nouveau temps
					if (car->_n_laps> 1) {
						car->_lap_times.push_back(0.0);
					}
				}
			}

			// animation de la zone de checkpoint active pour le joueur
			if (car== _hero) {
				if (car->_next_checkpoint!= _start) {
					car->_next_checkpoint->set_current_sequence(MAIN_SEQUENCE_NAME, t);
				}
				car->_next_checkpoint->_next->set_current_sequence("active", t);
			}
			
			// maj du prochain checkpoint
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
	number dist= number(glm::length(direction));
	number scalprod= scal(car->_forward, direction/ dist);
	number sign= 1.0;
	CarModel * model= car->get_model();

	// si distance suffisament grande on accélère
	if (dist> IA_THRUST_DIST_THRESHOLD) {
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

	// sinon on lache la pédale d'accélération
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

	// si on est globalement dans la bonne direction ou qu'on tourne déjà beaucoup
	if (scalprod< IA_SCALPROD_THRESHOLD && abs(car->_wheel)> IA_WHEEL_THRESHOLD) {

	}

	// sinon on tourne
	else {
		// wheel est >0 à gauche, <0 à droite
		if (is_left(car->_com, car->_forward, checkpoint->_com)) {
			sign= 1.0;
		}
		else {
			sign= -1.0;
		}
		car->_wheel= IA_WHEEL_AMPLITUDE* sign* (1.0- scalprod);
		if (car->_wheel> model->_max_wheel) {
			car->_wheel= model->_max_wheel;
		}
		if (car->_wheel< -1.0* model->_max_wheel) {
			car->_wheel= -1.0* model->_max_wheel;
		}
	}
}


void Track::set_car_contact_action(time_point t) {
	for (auto obj : _floating_objects) {
		if (obj->_car_contact && obj->_model->_sequences.count(CAR_CONTACT_SEQUENCE_NAME)> 0 && obj->_current_sequence_name!= CAR_CONTACT_SEQUENCE_NAME) {
			obj->set_current_sequence(CAR_CONTACT_SEQUENCE_NAME, t);
		}
		else if (!obj->_car_contact && obj->_current_sequence_name== CAR_CONTACT_SEQUENCE_NAME) {
			obj->set_current_sequence(MAIN_SEQUENCE_NAME, t);
		}
	}
}


void Track::lap_time(time_point t) {
	for (auto car  : _sorted_cars) {
		if (car->_finished) {
			continue;
		}

		// n'a pas encore passé le 1er start
		if (car->_n_laps== 0) {
			// init à 0.0 du 1er lap
			if (car->_lap_times.size()== 0) {
				car->_lap_times.push_back(0.0);
			}
			continue;
		}
		
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- car->_last_start_t).count();
		car->_lap_times[car->_lap_times.size()- 1]= number(dt)/ 1000.0;
	}
}


void Track::total_time() {
	// passage de just_finished (créé lors de collisions() et utilisé lors de sort_cars()) à finished
	for (auto car : _sorted_cars) {
		if (car->_just_finished) {
			car->_finished= true;
			// maj du temps total
			car->_total_time= 0.0;
			for (auto lap_time : car->_lap_times) {
				car->_total_time+= lap_time;
			}
		}
	}
}


void Track::anim_drivers(time_point t) {
	for (auto driver : _drivers) {
		if (driver->_tired) {
			driver->set_current_expression("tired", t);
			continue;
		}
		if (driver->_angry) {
			driver->set_current_expression("angry", t);
			continue;
		}
		if (driver->_happy) {
			driver->set_current_expression("happy", t);
			continue;
		}
		if (driver->_sad) {
			driver->set_current_expression("sad", t);
			continue;
		}
		driver->set_current_expression("normal", t);
	}

	for (auto driver : _drivers) {
		driver->anim(t);
	}
}


void Track::direction_help() {
	StaticObject * direction_obj= NULL;
	for (auto obj : _floating_objects) {
		if (obj->_model->_type== DIRECTION_HELP) {
			direction_obj= obj;
			break;
		}
	}
	number dist_hero_chkpt= norm(_hero->_next_checkpoint->_com- _hero->_com);
	pt_type position(0.0);
	number alpha= 0.0;

	if (dist_hero_chkpt< DIRECTION_HELP_DRAWN_THRESHOLD) {
		position.x= 10000.0;
	} else {
		position= _hero->_com+ DIRECTION_HELP_DIST_HERO* (_hero->_next_checkpoint->_com- _hero->_com)/ dist_hero_chkpt;
		alpha= angle(pt_type(0.0, 1.0), _hero->_next_checkpoint->_com- _hero->_com);
	}

	direction_obj->reinit(position, alpha, pt_type(DIRECTION_HELP_SCALE));

	//std::cout << position.x << " ; " << position.y << " ; " << alpha << "\n";
}


void Track::anim(time_point t, InputState * input_state, bool joystick_is_input) {
	// décompte avant course
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

	// captage input
	if (_mode== TRACK_LIVE) {
		if (input_state->_is_joystick && joystick_is_input) {
			_hero->preanim_joystick(input_state->_joystick_a, input_state->_joystick_b, input_state->_joystick);
		}
		else {
			_hero->preanim_keys(input_state->_keys[SDLK_LEFT], input_state->_keys[SDLK_RIGHT], input_state->_keys[SDLK_DOWN], input_state->_keys[SDLK_UP]);
		}
	}

	// on force input à 0
	else if (_mode== TRACK_FINISHED) {
		_hero->_thrust= _hero->_wheel= 0.0;
	}

	if (_mode== TRACK_LIVE || _mode== TRACK_FINISHED) {
		anim_objects(t);
		
		reinit_car_contact();

		// reinit des expressions des drivers
		reinit_drivers(t, false);

		// résolution des collisions
		collisions(t);

		// sur quelles surfaces roulent les voitures
		surfaces(t);

		// se font-elles réparer
		repair(t);

		// sont-elles sur une zone de boost
		boost(t);

		// maj pour chaque Car des checkpoints et des temps réalisés si passage start
		checkpoints(t);

		set_car_contact_action(t);

		// maj du temps du tour en cours
		lap_time(t);

		// tri des voitures selon leur positions
		sort_cars();

		// maj du temps total
		total_time();

		// anim des expressions des drivers
		anim_drivers(t);

		direction_help();
	}

	// fin de la piste
	if (_mode== TRACK_LIVE && _hero->_finished) {
		end();
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

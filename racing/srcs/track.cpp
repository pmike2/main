#include <fstream>
#include <sstream>

#include "json.hpp"

#include "track.h"
#include "utile.h"


using json = nlohmann::json;


// TrackTile ---------------------------------------------
TrackTile::TrackTile() : _json_path("UNKNOWN") {

}


TrackTile::TrackTile(std::string json_path) : _json_path(json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	for (auto polygon : js["polygons"]) {
		std::vector<pt_type> pts;
		for (auto coord : polygon) {
			pt_type pt(coord[0], coord[1]);
			pts.push_back(pt);
		}
		Polygon2D * obstacle= new Polygon2D();
		obstacle->set_points(pts);
		obstacle->triangulate();
		_obstacles.push_back(obstacle);
	}
}


TrackTile::~TrackTile() {
	clear();
}


void TrackTile::clear() {
	for (auto obstacle : _obstacles) {
		delete obstacle;
	}
	_obstacles.clear();
}


std::ostream & operator << (std::ostream & os, const TrackTile & tile) {
	os << "json_path=" << tile._json_path;
	return os;
}


// Track ---------------------------------------------
Track::Track() {
	load_models();
}


Track::~Track() {
	clear();
}


void Track::load_models() {
	for (auto model : _models) {
		delete model.second;
	}
	_models.clear();

	std::vector<std::string> jsons_static= list_files("../data/static_objects", "json");
	for (auto json_path : jsons_static) {
		_models[basename(json_path)]= new StaticObjectModel(json_path);
	}

	std::vector<std::string> jsons_cars= list_files("../data/cars", "json");
	for (auto json_path : jsons_cars) {
		_models[basename(json_path)]= new CarModel(json_path);
	}

	std::vector<std::string> jsons_tiles= list_files("../data/tiles", "json");
	for (auto json_path : jsons_tiles) {
		_model_tiles[basename(json_path)]= new TrackTile(json_path);
	}

	for (auto model : _models) {
		std::cout << model.first << "\n";
	}
	for (auto model_tile : _model_tiles) {
		std::cout << model_tile.first << "\n";
	}
}


void Track::clear() {
	for (auto obj : _objects) {
		delete obj;
	}
	_objects.clear();

	for (auto model : _models) {
		delete model.second;
	}
	_models.clear();

	for (auto tile : _tiles) {
		delete tile;
	}
	_tiles.clear();

	for (auto model : _model_tiles) {
		delete model.second;
	}
	_model_tiles.clear();
}


void Track::load_json(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	set_size(js["width"], js["height"], js["cell_size"]);
	unsigned int compt= 0;
	for (auto tilename : js["tiles"]) {
		set_tile(_model_tiles[tilename], compt);
		compt++;
	}

	for (auto tile : _tiles) {
		for (auto obstacle : tile->_obstacles) {
			StaticObject * obj= new StaticObject(_models["fixed_object"]);
			obj->_footprint= obstacle;
			obj->_bbox->set_aabb(*obstacle->_aabb);
			_objects.push_back(obj);
		}
	}
}


Car * Track::get_hero() {
	for (auto obj : _objects) {
		if (obj->_model->_type== HERO_CAR) {
			return (Car *)(obj);
		}
	}
	return NULL;
}


void Track::collision() {
	for (unsigned int idx_obj_1=0; idx_obj_1<_objects.size()- 1; ++idx_obj_1) {
		for (unsigned int idx_obj_2=idx_obj_1+ 1; idx_obj_2<_objects.size(); ++idx_obj_2) {
			StaticObject * obj1= _objects[idx_obj_1];
			StaticObject * obj2= _objects[idx_obj_2];

			if (obj1->_model->_fixed && obj2->_model->_fixed) {
				continue;
			}

			if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
				continue;
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
	}
}


void Track::anim(number dt) {
	for (auto obj : _objects) {
		obj->anim(dt);
	}

	collision();
}


unsigned int Track::coord2idx(unsigned int col_idx, unsigned int row_idx) {
	return col_idx+ row_idx* _width;
}


std::pair<unsigned int, unsigned int> Track::idx2coord(unsigned int idx) {
	return std::make_pair(idx % _width, idx / _width);
}


TrackTile * Track::get_tile(unsigned int col_idx, unsigned int row_idx) {
	if (row_idx> _height- 1) {
		std::cerr << "Track::get_tile : row_idx=" << row_idx << " >= _height=" << _height << "\n";
		return NULL;
	}
	if (col_idx> _width- 1) {
		std::cerr << "Track::get_tile : col_idx=" << col_idx << " >= _width=" << _width << "\n";
		return NULL;
	}
	return _tiles[coord2idx(col_idx, row_idx)];
}


void Track::set_tile(TrackTile * model_tile, unsigned int col_idx, unsigned int row_idx) {
	TrackTile * tile= get_tile(col_idx, row_idx);

	tile->clear();
	tile->_json_path= model_tile->_json_path;
	for (auto obstacle : model_tile->_obstacles) {
		Polygon2D * translated_obstacle= new Polygon2D(*obstacle);
		translated_obstacle->translate(pt_type(number(col_idx)* _cell_size, number(row_idx)* _cell_size));
		tile->_obstacles.push_back(translated_obstacle);
	}
}


void Track::set_tile(TrackTile * model_tile, unsigned int idx) {
	std::pair<unsigned int, unsigned int> coord= idx2coord(idx);
	set_tile(model_tile, coord.first, coord.second);
}


void Track::set_all(TrackTile * model_tile) {
	for (unsigned int row_idx=0; row_idx<_height; ++row_idx) {
		for (unsigned int col_idx=0; col_idx<_width; ++col_idx) {
			set_tile(model_tile, col_idx, row_idx);
		}
	}
}


void Track::set_size(unsigned int width, unsigned int height, number cell_size) {
	clear();

	_width= width;
	_height= height;
	_cell_size= cell_size;

	for (unsigned int i=0; i<_width* _height; ++i) {
		_tiles.push_back(new TrackTile());
	}
}


std::ostream & operator << (std::ostream & os, const Track & track) {
	os << "width=" << track._width << " ; height=" << track._height << " ; cell_size=" << track._cell_size << " ; tiles=[";
	for (auto tile : track._tiles) {
		os << *tile << " | ";
	}
	os << "]";
	return os;
}

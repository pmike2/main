#include "elements.h"



// Elements ---------------------------------------------------------------------------------------------------------
Elements::Elements() {

}


Elements::Elements(std::string dir_tree_jsons, std::string dir_stone_jsons, Elevation * elevation) : _elevation(elevation) {
	std::vector<std::string> tree_jsons = list_files(dir_tree_jsons, "json");
	for (auto json_path : tree_jsons) {
		_tree_species[basename(json_path)] = new TreeSpecies(json_path);
	}
	std::vector<std::string> stone_jsons = list_files(dir_stone_jsons, "json");
	for (auto json_path : stone_jsons) {
		_stone_species[basename(json_path)] = new StoneSpecies(json_path);
	}
}


Elements::~Elements() {
	clear();

	for (auto species : _tree_species) {
		delete species.second;
	}
	_tree_species.clear();
}


Tree * Elements::add_tree(std::string species_name, pt_2d position) {
	if (_tree_species.count(species_name) == 0) {
		std::cerr << species_name << " espece inconnue\n";
		return NULL;
	}

	Tree * tree = new Tree(_tree_species[species_name], _elevation, position);
	_elements.push_back(tree);
	return tree;
}


Stone * Elements::add_stone(std::string species_name, pt_2d position) {
	if (_stone_species.count(species_name) == 0) {
		std::cerr << species_name << " espece inconnue\n";
		return NULL;
	}

	Stone * stone = new Stone(_stone_species[species_name], _elevation, position);
	_elements.push_back(stone);
	return stone;
}


Lake * Elements::add_lake(pt_2d position) {
	Lake * lake = new Lake(_elevation, position);
	if (!lake->_valid) {
		delete lake;
		return NULL;
	}

	if (lake->_polygon == NULL) {
		delete lake;
		std::cerr << "Map::add_lake polygon NULL\n";
		return NULL;
	}

	_elements.push_back(lake);
	return lake;
}


River * Elements::add_river(pt_2d position) {
	River * river = new River(_elevation, position);
	if (!river->_valid) {
		delete river;
		return NULL;
	}

	if (river->_polygon == NULL) {
		delete river;
		std::cerr << "Map::add_river polygon NULL\n";
		return NULL;
	}

	_elements.push_back(river);
	return river;
}


void Elements::clear() {
	for (auto element : _elements) {
		delete element;
	}
	_elements.clear();
}


void Elements::remove_element(Element * element) {
	_elements.erase(std::remove_if(_elements.begin(), _elements.end(), [element](Element * e) {
		return e == element;
	}), _elements.end());
	delete element;
}


void Elements::remove_elements_in_aabb(AABB_2D * aabb) {
	std::vector<Element *> elements2remove;
	for (auto & element : _elements) {
		if (aabb2d_intersects_aabb2d(aabb, element->_bbox->_aabb->aabb2d())) {
			elements2remove.push_back(element);
		}
	}
	for (auto & element : elements2remove) {
		remove_element(element);
	}
}

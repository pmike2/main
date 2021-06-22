#include <glm/gtx/string_cast.hpp>

#include "dungeon.h"


using namespace std;


Dungeon::Dungeon() {
	
}


Dungeon::Dungeon(glm::vec3 vmin, glm::vec3 vmax, glm::vec3 step) : _step(step) {
	_aabb= new AABB(vmin, vmax);
	_n= (_aabb->_vmax- _aabb->_vmin)/ _step;
	_graph= new Graph();
	for (unsigned int x=0; x<_n.x; ++x) {
		for (unsigned int y=0; y<_n.y; ++y) {
			for (unsigned int z=0; z<_n.z; ++z) {
				glm::uvec3 pos= glm::uvec3(x, y, z);
				_graph->add_vertex(pos2idx(pos), pos2posf(pos));
			}
		}
	}
}


Dungeon::~Dungeon() {
	for (auto mesh : _meshes) {
		delete mesh;
	}
	_meshes.clear();
	delete _graph;
	delete _aabb;
}


void Dungeon::randomize() {
	for (auto mesh : _meshes) {
		delete mesh;
	}
	_meshes.clear();

	unsigned int n_meshes= 10;

	Mesh * mesh= new Mesh();
	glm::uvec3 pos_origin= posf2pos(glm::vec3(0.0f));
	glm::uvec3 pos0= pos_origin;
	glm::uvec3 pos1= pos0+ glm::uvec3(1, 0, 0);
	glm::uvec3 pos2= pos0+ glm::uvec3(1, 1, 0);
	glm::uvec3 pos3= pos0+ glm::uvec3(0, 1, 0);

	mesh->_edges.push_back(make_pair(pos2idx(pos0), pos2idx(pos1)));
	mesh->_edges.push_back(make_pair(pos2idx(pos1), pos2idx(pos2)));
	mesh->_edges.push_back(make_pair(pos2idx(pos2), pos2idx(pos3)));
	mesh->_edges.push_back(make_pair(pos2idx(pos3), pos2idx(pos0)));
	_meshes.push_back(mesh);
}


unsigned int Dungeon::pos2idx(glm::uvec3 pos) {
	return pos.x* _n.y* _n.z+ pos.y* _n.z+ pos.z;
}


glm::uvec3 Dungeon::idx2pos(unsigned int idx) {
	return glm::uvec3(idx/ (_n.y* _n.z), (idx/ _n.z)% _n.y, idx% _n.z);
}


glm::vec3 Dungeon::pos2posf(glm::uvec3 pos) {
	return _aabb->_vmin+ glm::vec3((float)(pos.x), (float)(pos.y), (float)(pos.z))* _step;
}


glm::uvec3 Dungeon::posf2pos(glm::vec3 posf) {
	return glm::uvec3((unsigned int)((posf.x- _aabb->_vmin.x)/ _step.x), (unsigned int)((posf.y- _aabb->_vmin.y)/ _step.y), (unsigned int)((posf.z- _aabb->_vmin.z)/ _step.z));
}


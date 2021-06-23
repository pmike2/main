#include <glm/gtx/string_cast.hpp>

#include "dungeon.h"


using namespace std;


Dungeon::Dungeon() {
	
}


Dungeon::Dungeon(glm::vec3 vmin, glm::vec3 vmax, glm::vec3 step, GLuint prog_draw) : _step(step), _prog_draw(prog_draw), _n_pts(0) {
	_aabb= new AABB(vmin, vmax);
	_n= (_aabb->_vmax- _aabb->_vmin)/ _step;
	//cout << "_n=" << glm::to_string(_n) << "\n";
	_graph= new Graph();
	for (unsigned int x=0; x<_n.x; ++x) {
		for (unsigned int y=0; y<_n.y; ++y) {
			for (unsigned int z=0; z<_n.z; ++z) {
				glm::uvec3 pos= glm::uvec3(x, y, z);
				_graph->add_vertex(pos2idx(pos), pos2posf(pos));
			}
		}
	}

	glGenBuffers(1, &_buffer);
		
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


Dungeon::~Dungeon() {
	for (auto mesh : _meshes) {
		delete mesh;
	}
	_meshes.clear();
	delete _graph;
	delete _aabb;
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


void Dungeon::randomize() {
	for (auto mesh : _meshes) {
		delete mesh;
	}
	_meshes.clear();

	float z= 10.0f;
	vector<AABB *> aabbs;
	for (unsigned int i=0; i<100; ++i) {
		glm::uvec3 pos0= glm::uvec3(rand_int(0, _n.x- 2), rand_int(0, _n.y- 2), rand_int(0, _n.z- 2));
		unsigned int size_x= rand_int(1, _n.x- 1- pos0.x);
		unsigned int size_y= rand_int(1, _n.y- 1- pos0.y);
		glm::uvec3 pos1= pos0+ glm::uvec3(size_x, 0, 0);
		glm::uvec3 pos2= pos0+ glm::uvec3(size_x, size_y, 0);
		glm::uvec3 pos3= pos0+ glm::uvec3(0, size_y, 0);

		bool is_inter= false;
		AABB * new_aabb= new AABB(pos2posf(pos0), pos2posf(pos2)+ glm::vec3(0.0f, 0.0f, z));
		for (auto aabb : aabbs) {
			if (aabb_intersects_aabb(aabb, new_aabb)) {
				is_inter= true;
				break;
			}
		}
		if (is_inter) {
			continue;
		}
		aabbs.push_back(new_aabb);
		
		Mesh * mesh= new Mesh();
		mesh->_edges.push_back(make_pair(pos2idx(pos0), pos2idx(pos1)));
		mesh->_edges.push_back(make_pair(pos2idx(pos1), pos2idx(pos2)));
		mesh->_edges.push_back(make_pair(pos2idx(pos2), pos2idx(pos3)));
		mesh->_edges.push_back(make_pair(pos2idx(pos3), pos2idx(pos0)));
		_meshes.push_back(mesh);
	}

	for (auto aabb : aabbs) {
		delete aabb;
	}
	aabbs.clear();

	/*for (auto mesh : _meshes) {
		for (auto edge : mesh->_edges) {
			cout << edge.first << " ; " << edge.second << "\n";
		}
	}*/

	update();
}


void Dungeon::draw(const glm::mat4 & world2clip) {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_LINES, 0, _n_pts);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Dungeon::update() {
	_n_pts= 0;
	for (auto mesh : _meshes) {
		_n_pts+= mesh->_edges.size()* 2;
	}
	//cout << _n_pts << "\n";

	float data[6* _n_pts];
	unsigned int compt= 0;
	for (auto mesh : _meshes) {
		for (auto edge : mesh->_edges) {
			data[6* compt+ 0]= _graph->_vertices[edge.first]._pos.x;
			data[6* compt+ 1]= _graph->_vertices[edge.first]._pos.y;
			data[6* compt+ 2]= _graph->_vertices[edge.first]._pos.z;
			data[6* compt+ 3]= 1.0f;
			data[6* compt+ 4]= 0.0f;
			data[6* compt+ 5]= 0.0f;
			compt++;

			data[6* compt+ 0]= _graph->_vertices[edge.second]._pos.x;
			data[6* compt+ 1]= _graph->_vertices[edge.second]._pos.y;
			data[6* compt+ 2]= _graph->_vertices[edge.second]._pos.z;
			data[6* compt+ 3]= 1.0f;
			data[6* compt+ 4]= 0.0f;
			data[6* compt+ 5]= 0.0f;
			compt++;
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/*for (int i=0; i<_n_pts; ++i) {
		for (int j=0; j< 6; ++j) {
			cout << data[6* i+ j] << " ; ";
		}
		cout << "\n";
	}*/
}

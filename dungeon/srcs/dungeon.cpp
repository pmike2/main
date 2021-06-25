#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "dungeon.h"


using namespace std;


Dungeon::Dungeon() {
	
}


Dungeon::Dungeon(glm::vec3 vmin, glm::vec3 vmax, glm::vec3 step, GLuint prog_draw_border, GLuint prog_draw_fill) : _step(step), _prog_draw_border(prog_draw_border), _prog_draw_fill(prog_draw_fill), _n_pts(0) {
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

	glGenBuffers(2, _buffers);
		
	_position_loc= glGetAttribLocation(_prog_draw_border, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw_border, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw_border, "world2clip_matrix");
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

	float EPS= 0.01f;
	float z= 10.0f;
	std::vector<Mesh *> rooms;
	std::vector<Mesh *> hallways;
	vector<AABB *> aabbs;
	for (unsigned int i=0; i<50; ++i) {
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
		rooms.push_back(mesh);
	}

	for (unsigned int i=0; i<30; ++i) {
		unsigned int idx_mesh= rand_int(0, aabbs.size()- 1);
		//unsigned int j= rand_int(0, 3);
		unsigned int j= 0;

		// a droite
		if (j== 0) {
			AABB * aabb= new AABB(glm::vec3(aabbs[idx_mesh]->_vmin.x+ EPS, aabbs[idx_mesh]->_vmax.y, _aabb->_vmin.z),
				glm::vec3(aabbs[idx_mesh]->_vmax.x- EPS, _aabb->_vmax.y, _aabb->_vmax.z));
			float dist_min= 1e7;
			unsigned int idx_mesh_min= 0;
			for (unsigned idx_mesh_2=0; idx_mesh_2<aabbs.size(); ++idx_mesh_2) {
				if (idx_mesh_2== idx_mesh) {
					continue;
				}
				if (segment_intersects_aabb(glm::vec3(aabbs[idx_mesh_2]->_vmin.x, aabbs[idx_mesh_2]->_vmin.y, aabbs[idx_mesh_2]->_vmin.z),
					glm::vec3(aabbs[idx_mesh_2]->_vmax.x, aabbs[idx_mesh_2]->_vmin.y, aabbs[idx_mesh_2]->_vmin.z), aabb)) {
					float dist= aabbs[idx_mesh_2]->_vmin.y- aabbs[idx_mesh]->_vmax.y;
					if (dist< dist_min) {
						dist_min= dist;
						idx_mesh_min= idx_mesh_2;
					}
				}
			}
			if (dist_min> 1e5) {
				continue;
			}
			glm::uvec3 pos0= idx2pos(rooms[idx_mesh]->_edges[2].second);
			glm::uvec3 pos1= idx2pos(rooms[idx_mesh]->_edges[2].first);
			glm::uvec3 pos2= idx2pos(rooms[idx_mesh_min]->_edges[0].second);
			glm::uvec3 pos3= idx2pos(rooms[idx_mesh_min]->_edges[0].first);
			unsigned int xmin= max(pos0.x, pos3.x);
			unsigned int xmax= min(pos1.x, pos2.x);
			unsigned int xmin_rand= rand_int(xmin, xmax- 1);
			unsigned int xmax_rand= rand_int(xmin_rand+ 1, xmax);
			pos0.x= xmin_rand;
			pos3.x= xmin_rand;
			pos1.x= xmax_rand;
			pos2.x= xmax_rand;

			bool is_inter= false;
			for (unsigned idx_mesh_2=0; idx_mesh_2<aabbs.size(); ++idx_mesh_2) {
				if ((idx_mesh_2== idx_mesh) || (idx_mesh_2== idx_mesh_min)) {
					continue;
				}
				if ((segment_intersects_aabb(pos2posf(pos0), pos2posf(pos3), aabbs[idx_mesh_2])) ||
					(segment_intersects_aabb(pos2posf(pos1), pos2posf(pos2), aabbs[idx_mesh_2]))) {
					is_inter= true;
					break;
				}
			}
			if (is_inter) {
				continue;
			}

			Mesh * mesh= new Mesh();
			mesh->_edges.push_back(make_pair(pos2idx(pos0), pos2idx(pos1)));
			mesh->_edges.push_back(make_pair(pos2idx(pos1), pos2idx(pos2)));
			mesh->_edges.push_back(make_pair(pos2idx(pos2), pos2idx(pos3)));
			mesh->_edges.push_back(make_pair(pos2idx(pos3), pos2idx(pos0)));
			hallways.push_back(mesh);
		}
	}
	_meshes.insert(_meshes.end(), rooms.begin(), rooms.end());
	_meshes.insert(_meshes.end(), hallways.begin(), hallways.end());

	for (auto aabb : aabbs) {
		delete aabb;
	}
	aabbs.clear();

	update();
}


void Dungeon::draw(const glm::mat4 & world2clip) {
	glUseProgram(_prog_draw_border);
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	
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

	// -------------
	glUseProgram(_prog_draw_fill);
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _meshes.size()* 6);

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

	float data_border[6* _n_pts];
	unsigned int compt= 0;
	for (auto mesh : _meshes) {
		for (auto edge : mesh->_edges) {
			data_border[6* compt+ 0]= _graph->_vertices[edge.first]._pos.x;
			data_border[6* compt+ 1]= _graph->_vertices[edge.first]._pos.y;
			data_border[6* compt+ 2]= _graph->_vertices[edge.first]._pos.z;
			data_border[6* compt+ 3]= 1.0f;
			data_border[6* compt+ 4]= 0.0f;
			data_border[6* compt+ 5]= 0.0f;
			compt++;

			data_border[6* compt+ 0]= _graph->_vertices[edge.second]._pos.x;
			data_border[6* compt+ 1]= _graph->_vertices[edge.second]._pos.y;
			data_border[6* compt+ 2]= _graph->_vertices[edge.second]._pos.z;
			data_border[6* compt+ 3]= 1.0f;
			data_border[6* compt+ 4]= 0.0f;
			data_border[6* compt+ 5]= 0.0f;
			compt++;
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_border), data_border, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// ----------------------
	float data_fill[_meshes.size()* 42];
	compt= 0;
	for (auto mesh : _meshes) {
		data_fill[42* compt+ 0]= _graph->_vertices[mesh->_edges[0].first]._pos.x;
		data_fill[42* compt+ 1]= _graph->_vertices[mesh->_edges[0].first]._pos.y;
		data_fill[42* compt+ 2]= _graph->_vertices[mesh->_edges[0].first]._pos.z;

		data_fill[42* compt+ 7]= _graph->_vertices[mesh->_edges[1].first]._pos.x;
		data_fill[42* compt+ 8]= _graph->_vertices[mesh->_edges[1].first]._pos.y;
		data_fill[42* compt+ 9]= _graph->_vertices[mesh->_edges[1].first]._pos.z;

		data_fill[42* compt+ 14]= _graph->_vertices[mesh->_edges[2].first]._pos.x;
		data_fill[42* compt+ 15]= _graph->_vertices[mesh->_edges[2].first]._pos.y;
		data_fill[42* compt+ 16]= _graph->_vertices[mesh->_edges[2].first]._pos.z;


		data_fill[42* compt+ 21]= _graph->_vertices[mesh->_edges[0].first]._pos.x;
		data_fill[42* compt+ 22]= _graph->_vertices[mesh->_edges[0].first]._pos.y;
		data_fill[42* compt+ 23]= _graph->_vertices[mesh->_edges[0].first]._pos.z;

		data_fill[42* compt+ 28]= _graph->_vertices[mesh->_edges[2].first]._pos.x;
		data_fill[42* compt+ 29]= _graph->_vertices[mesh->_edges[2].first]._pos.y;
		data_fill[42* compt+ 30]= _graph->_vertices[mesh->_edges[2].first]._pos.z;

		data_fill[42* compt+ 35]= _graph->_vertices[mesh->_edges[3].first]._pos.x;
		data_fill[42* compt+ 36]= _graph->_vertices[mesh->_edges[3].first]._pos.y;
		data_fill[42* compt+ 37]= _graph->_vertices[mesh->_edges[3].first]._pos.z;

		for (unsigned int i=0; i<6; ++i) {
			data_fill[42* compt+ i* 7+ 3]= 0.5;
			data_fill[42* compt+ i* 7+ 4]= 0.2;
			data_fill[42* compt+ i* 7+ 5]= 0.7;
			data_fill[42* compt+ i* 7+ 6]= 0.5;
		}

		compt++;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_fill), data_fill, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

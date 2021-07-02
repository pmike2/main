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

	glGenBuffers(4, _buffers);
		
	_position_loc= glGetAttribLocation(_prog_draw_border, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw_border, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw_border, "world2clip_matrix");

	_draw[0]= true;
	_draw[1]= true;
	_draw[2]= false;
	_draw[3]= false;
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
	for (auto aabb_room : _aabbs_rooms) {
		delete aabb_room;
	}
	_aabbs_rooms.clear();
	for (auto bbox_hallway : _bboxs_hallways) {
		delete bbox_hallway;
	}
	_bboxs_hallways.clear();

	float EPS= 0.01f;
	float Z_ROOM_MIN= 10.0f;
	float Z_ROOM_MAX= 20.0f;
	float HALLWAY_THICKNESS= 4.0f;
	float MIN_DIST_BETWEEN_ROOMS= 10.0f;
	unsigned int MIN_ROOM_SIZE= 5;
	unsigned int N_ROOMS= 200;
	unsigned int N_HALLWAYS= 400;
	
	std::vector<Mesh *> rooms;
	std::vector<Mesh *> hallways;
	//vector<bool> connected;

	for (unsigned int i=0; i<N_ROOMS; ++i) {
		glm::uvec3 pos0= glm::uvec3(rand_int(0, _n.x- 1- MIN_ROOM_SIZE), rand_int(0, _n.y- 1- MIN_ROOM_SIZE), rand_int(0, _n.z- 1));
		unsigned int size_x= rand_int(MIN_ROOM_SIZE, _n.x- 1- pos0.x);
		unsigned int size_y= rand_int(MIN_ROOM_SIZE, _n.y- 1- pos0.y);
		glm::uvec3 pos1= pos0+ glm::uvec3(size_x, 0, 0);
		glm::uvec3 pos2= pos0+ glm::uvec3(size_x, size_y, 0);
		glm::uvec3 pos3= pos0+ glm::uvec3(0, size_y, 0);

		bool is_inter= false;
		AABB * new_aabb= new AABB(pos2posf(pos0), pos2posf(pos2)+ glm::vec3(0.0f, 0.0f, rand_float(Z_ROOM_MIN, Z_ROOM_MAX)));
		for (auto aabb : _aabbs_rooms) {
			if (aabb_intersects_aabb(aabb, new_aabb)) {
				is_inter= true;
				break;
			}
		}
		if (is_inter) {
			continue;
		}
		
		_aabbs_rooms.push_back(new_aabb);
		//connected.push_back(false);
		
		Mesh * mesh= new Mesh();
		mesh->_edges.push_back(make_pair(pos2idx(pos0), pos2idx(pos1)));
		mesh->_edges.push_back(make_pair(pos2idx(pos1), pos2idx(pos2)));
		mesh->_edges.push_back(make_pair(pos2idx(pos2), pos2idx(pos3)));
		mesh->_edges.push_back(make_pair(pos2idx(pos3), pos2idx(pos0)));
		rooms.push_back(mesh);
	}

	for (unsigned int i=0; i<N_HALLWAYS; ++i) {
		unsigned int idx_mesh= rand_int(0, _aabbs_rooms.size()- 1);
		unsigned int idx_mesh_min= 0;
		unsigned int j= rand_int(0, 1);
		//unsigned int j= 1;
		BBox * bbox;
		glm::uvec3 pos0, pos1, pos2, pos3;

		// y+
		if (j== 0) {
			AABB * aabb= new AABB(
				glm::vec3(_aabbs_rooms[idx_mesh]->_vmin.x+ EPS, _aabbs_rooms[idx_mesh]->_vmax.y+ MIN_DIST_BETWEEN_ROOMS, _aabb->_vmin.z),
				glm::vec3(_aabbs_rooms[idx_mesh]->_vmax.x- EPS, _aabb->_vmax.y, _aabb->_vmax.z));
			float dist_min= 1e7;
			idx_mesh_min= 0;
			for (unsigned idx_mesh_2=0; idx_mesh_2<_aabbs_rooms.size(); ++idx_mesh_2) {
				if (idx_mesh_2== idx_mesh) {
					continue;
				}
				if (segment_intersects_aabb(
					glm::vec3(_aabbs_rooms[idx_mesh_2]->_vmin.x, _aabbs_rooms[idx_mesh_2]->_vmin.y, _aabbs_rooms[idx_mesh_2]->_vmin.z),
					glm::vec3(_aabbs_rooms[idx_mesh_2]->_vmax.x, _aabbs_rooms[idx_mesh_2]->_vmin.y, _aabbs_rooms[idx_mesh_2]->_vmin.z), aabb)) {
					float dist= _aabbs_rooms[idx_mesh_2]->_vmin.y- _aabbs_rooms[idx_mesh]->_vmax.y;
					if (dist< dist_min) {
						dist_min= dist;
						idx_mesh_min= idx_mesh_2;
					}
				}
			}
			if (dist_min> 1e5) {
				continue;
			}

			pos0= idx2pos(rooms[idx_mesh]->_edges[2].second);
			pos1= idx2pos(rooms[idx_mesh]->_edges[2].first);
			pos2= idx2pos(rooms[idx_mesh_min]->_edges[0].second);
			pos3= idx2pos(rooms[idx_mesh_min]->_edges[0].first);
			unsigned int xmin= max(pos0.x, pos3.x);
			unsigned int xmax= min(pos1.x, pos2.x);
			unsigned int xmin_rand= rand_int(xmin, xmax- 1);
			unsigned int xmax_rand= rand_int(xmin_rand+ 1, xmax);
			pos0.x= xmin_rand;
			pos3.x= xmin_rand;
			pos1.x= xmax_rand;
			pos2.x= xmax_rand;
			glm::vec3 posf0= pos2posf(pos0);
			glm::vec3 posf1= pos2posf(pos1);
			glm::vec3 posf2= pos2posf(pos2);
			glm::vec3 posf3= pos2posf(pos3);

			bool is_inter_room= false;
			for (unsigned idx_mesh_2=0; idx_mesh_2<_aabbs_rooms.size(); ++idx_mesh_2) {
				if ((idx_mesh_2== idx_mesh) || (idx_mesh_2== idx_mesh_min)) {
					continue;
				}
				if ((segment_intersects_aabb(posf0, posf3, _aabbs_rooms[idx_mesh_2])) ||
					(segment_intersects_aabb(posf1, posf2, _aabbs_rooms[idx_mesh_2]))) {
					is_inter_room= true;
					break;
				}
			}
			if (is_inter_room) {
				continue;
			}

			bbox= new BBox(
				glm::vec3(-0.5f* HALLWAY_THICKNESS, -0.5f* HALLWAY_THICKNESS, -0.5f* HALLWAY_THICKNESS),
				glm::vec3(posf1.x- posf0.x, glm::length(glm::vec3(0.0f, posf3.y- posf0.y, posf3.z- posf0.z)), 0.0f)+ glm::vec3(0.5f* HALLWAY_THICKNESS, 0.5f* HALLWAY_THICKNESS, 0.5f* HALLWAY_THICKNESS),
				glm::rotate(glm::translate(glm::mat4(1.0f), posf0), atan((posf3.z- posf0.z)/ (posf3.y- posf0.y)), glm::vec3(1.0f, 0.0f, 0.0f))
			);
		}

		// x+
		else if (j== 1) {
			AABB * aabb= new AABB(
				glm::vec3(_aabbs_rooms[idx_mesh]->_vmax.x+ MIN_DIST_BETWEEN_ROOMS, _aabbs_rooms[idx_mesh]->_vmin.y+ EPS, _aabb->_vmin.z),
				glm::vec3(_aabb->_vmax.x, _aabbs_rooms[idx_mesh]->_vmax.y- EPS, _aabb->_vmax.z));
			float dist_min= 1e7;
			idx_mesh_min= 0;
			for (unsigned idx_mesh_2=0; idx_mesh_2<_aabbs_rooms.size(); ++idx_mesh_2) {
				if (idx_mesh_2== idx_mesh) {
					continue;
				}
				if (segment_intersects_aabb(
					glm::vec3(_aabbs_rooms[idx_mesh_2]->_vmin.x, _aabbs_rooms[idx_mesh_2]->_vmin.y, _aabbs_rooms[idx_mesh_2]->_vmin.z),
					glm::vec3(_aabbs_rooms[idx_mesh_2]->_vmin.x, _aabbs_rooms[idx_mesh_2]->_vmax.y, _aabbs_rooms[idx_mesh_2]->_vmin.z), aabb)) {
					float dist= _aabbs_rooms[idx_mesh_2]->_vmin.x- _aabbs_rooms[idx_mesh]->_vmax.x;
					if (dist< dist_min) {
						dist_min= dist;
						idx_mesh_min= idx_mesh_2;
					}
				}
			}
			if (dist_min> 1e5) {
				continue;
			}

			pos0= idx2pos(rooms[idx_mesh]->_edges[1].first);
			pos1= idx2pos(rooms[idx_mesh_min]->_edges[3].second);
			pos2= idx2pos(rooms[idx_mesh_min]->_edges[3].first);
			pos3= idx2pos(rooms[idx_mesh]->_edges[1].second);
			unsigned int ymin= max(pos0.y, pos1.y);
			unsigned int ymax= min(pos2.y, pos3.y);
			unsigned int ymin_rand= rand_int(ymin, ymax- 1);
			unsigned int ymax_rand= rand_int(ymin_rand+ 1, ymax);
			pos0.y= ymin_rand;
			pos1.y= ymin_rand;
			pos2.y= ymax_rand;
			pos3.y= ymax_rand;
			glm::vec3 posf0= pos2posf(pos0);
			glm::vec3 posf1= pos2posf(pos1);
			glm::vec3 posf2= pos2posf(pos2);
			glm::vec3 posf3= pos2posf(pos3);

			bool is_inter_room= false;
			for (unsigned idx_mesh_2=0; idx_mesh_2<_aabbs_rooms.size(); ++idx_mesh_2) {
				if ((idx_mesh_2== idx_mesh) || (idx_mesh_2== idx_mesh_min)) {
					continue;
				}
				if ((segment_intersects_aabb(posf0, posf1, _aabbs_rooms[idx_mesh_2])) ||
					(segment_intersects_aabb(posf2, posf3, _aabbs_rooms[idx_mesh_2]))) {
					is_inter_room= true;
					break;
				}
			}
			if (is_inter_room) {
				continue;
			}

			bbox= new BBox(
				glm::vec3(-0.5f* HALLWAY_THICKNESS, -0.5f* HALLWAY_THICKNESS, -0.5f* HALLWAY_THICKNESS),
				glm::vec3(glm::length(glm::vec3(0.0f, posf1.x- posf0.x, posf1.z- posf0.z)), posf3.y- posf0.y, 0.0f)+ glm::vec3(0.5f* HALLWAY_THICKNESS, 0.5f* HALLWAY_THICKNESS, 0.5f* HALLWAY_THICKNESS),
				glm::rotate(glm::translate(glm::mat4(1.0f), posf0), atan((posf1.z- posf0.z)/ (posf1.x- posf0.x)), glm::vec3(0.0f, -1.0f, 0.0f)) // pas compris pquoi mais il faut -1 ici
			);
		}		

		// ===================================================
		bool is_inter_hallway= false;
		for (unsigned int idx_hallway=0; idx_hallway<hallways.size(); ++idx_hallway) {
			if (bbox_intersects_bbox(_bboxs_hallways[idx_hallway], bbox)) {
				is_inter_hallway= true;
				break;
			}
		}
		if (is_inter_hallway) {
			continue;
		}
		
		_bboxs_hallways.push_back(bbox);
		
		//connected[idx_mesh]= true;
		//connected[idx_mesh_min]= true;
		//rooms[idx_mesh]->_debug= true;
		//rooms[idx_mesh_min]->_debug= true;

		Mesh * mesh= new Mesh();
		mesh->_edges.push_back(make_pair(pos2idx(pos0), pos2idx(pos1)));
		mesh->_edges.push_back(make_pair(pos2idx(pos1), pos2idx(pos2)));
		mesh->_edges.push_back(make_pair(pos2idx(pos2), pos2idx(pos3)));
		mesh->_edges.push_back(make_pair(pos2idx(pos3), pos2idx(pos0)));
		mesh->_debug= false;
		hallways.push_back(mesh);
	}
	
	_meshes.insert(_meshes.end(), rooms.begin(), rooms.end());
	/*for (unsigned int i=0; i<rooms.size(); ++i) {
		if (connected[i]) {
			_meshes.push_back(rooms[i]);
		}
	}*/
	_meshes.insert(_meshes.end(), hallways.begin(), hallways.end());

	update();
}


void Dungeon::draw(const glm::mat4 & world2clip) {
	if (_draw[0]) {
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
	}

	// -------------
	if (_draw[1]) {
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

	// -------------
	if (_draw[2]) {
		glUseProgram(_prog_draw_fill);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[3]);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_TRIANGLES, 0, _bboxs_hallways.size()* 36);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	// -------------
	if (_draw[3]) {
		glUseProgram(_prog_draw_fill);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_TRIANGLES, 0, _aabbs_rooms.size()* 36);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
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
			if (mesh->_debug) {
				data_fill[42* compt+ i* 7+ 3]= 0.5;
				data_fill[42* compt+ i* 7+ 4]= 0.2;
				data_fill[42* compt+ i* 7+ 5]= 0.7;
			}
			else {
				data_fill[42* compt+ i* 7+ 3]= 0.0;
				data_fill[42* compt+ i* 7+ 4]= 0.9;
				data_fill[42* compt+ i* 7+ 5]= 0.9;
			}
			data_fill[42* compt+ i* 7+ 6]= 0.5;
		}

		compt++;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_fill), data_fill, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// ----------------------
	float data_aabb_room[_aabbs_rooms.size()* 252];
	vector<vector<unsigned int> > aabb_tri_idx= AABB::triangles_idxs();
	
	for (unsigned int idx_room=0; idx_room<_aabbs_rooms.size(); ++idx_room) {
		for (unsigned int i=0; i<12; ++i) {
			for (unsigned int j=0; j<3; ++j) {
				data_aabb_room[idx_room* 12* 3* 7+ i* 3* 7+ j* 7+ 0]= _aabbs_rooms[idx_room]->_pts[aabb_tri_idx[i][j]].x;
				data_aabb_room[idx_room* 12* 3* 7+ i* 3* 7+ j* 7+ 1]= _aabbs_rooms[idx_room]->_pts[aabb_tri_idx[i][j]].y;
				data_aabb_room[idx_room* 12* 3* 7+ i* 3* 7+ j* 7+ 2]= _aabbs_rooms[idx_room]->_pts[aabb_tri_idx[i][j]].z;
				data_aabb_room[idx_room* 12* 3* 7+ i* 3* 7+ j* 7+ 3]= 0.2f;
				data_aabb_room[idx_room* 12* 3* 7+ i* 3* 7+ j* 7+ 4]= 0.7f;
				data_aabb_room[idx_room* 12* 3* 7+ i* 3* 7+ j* 7+ 5]= 0.5f;
				data_aabb_room[idx_room* 12* 3* 7+ i* 3* 7+ j* 7+ 6]= 0.7f;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_aabb_room), data_aabb_room, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// ----------------------
	float data_bbox_hallway[_bboxs_hallways.size()* 252];
	vector<vector<unsigned int> > bbox_tri_idx= BBox::triangles_idxs();
	for (unsigned int idx_hallway=0; idx_hallway<_bboxs_hallways.size(); ++ idx_hallway) {
		for (unsigned int i=0; i<12; ++i) {
			for (unsigned int j=0; j<3; ++j) {
				data_bbox_hallway[idx_hallway* 12* 3* 7+ i* 3* 7+ j* 7+ 0]= _bboxs_hallways[idx_hallway]->_pts[bbox_tri_idx[i][j]].x;
				data_bbox_hallway[idx_hallway* 12* 3* 7+ i* 3* 7+ j* 7+ 1]= _bboxs_hallways[idx_hallway]->_pts[bbox_tri_idx[i][j]].y;
				data_bbox_hallway[idx_hallway* 12* 3* 7+ i* 3* 7+ j* 7+ 2]= _bboxs_hallways[idx_hallway]->_pts[bbox_tri_idx[i][j]].z;
				data_bbox_hallway[idx_hallway* 12* 3* 7+ i* 3* 7+ j* 7+ 3]= 0.7f;
				data_bbox_hallway[idx_hallway* 12* 3* 7+ i* 3* 7+ j* 7+ 4]= 0.2f;
				data_bbox_hallway[idx_hallway* 12* 3* 7+ i* 3* 7+ j* 7+ 5]= 0.5f;
				data_bbox_hallway[idx_hallway* 12* 3* 7+ i* 3* 7+ j* 7+ 6]= 0.7f;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_bbox_hallway), data_bbox_hallway, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool Dungeon::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_KP_0) {
		_draw[0]= !_draw[0];
		return true;
	}
	else if (key== SDLK_KP_1) {
		_draw[1]= !_draw[1];
		return true;
	}
	else if (key== SDLK_KP_2) {
		_draw[2]= !_draw[2];
		return true;
	}
	else if (key== SDLK_KP_3) {
		_draw[3]= !_draw[3];
		return true;
	}

	return false;
}


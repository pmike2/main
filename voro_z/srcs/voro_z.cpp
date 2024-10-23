#include <glm/gtc/type_ptr.hpp>

#include "utile.h"
#include "voronoi.h"
#include "dcel.h"

#include "voro_z.h"


DCEL_FaceData::DCEL_FaceData() : _z(0.0), _color(glm::vec4(0.0)) {

}


DCEL_FaceData::DCEL_FaceData(number z) :
	_z(z), _color(glm::vec4(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f), 0.5f)) 
{

}


DCEL_FaceData::~DCEL_FaceData() {

}


// --------------------------------------------------------
VoroZ::VoroZ() {

}


VoroZ::VoroZ(GLuint prog_draw): _prog_draw(prog_draw) {
	glGenBuffers(4, _buffers);
		
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");

	int n_pts= 200;
	number xmin= -10.0;
	number xmax= 10.0;
	number ymin= -10.0;
	number ymax= 10.0;
	number zmin= 0.0;
	number zmax= 10.0;

	std::vector<pt_type> pts;
	for (unsigned int i=0; i<n_pts; ++i) {
		pt_type pt(rand_number(xmin, xmax), rand_number(ymin, ymax));
		pts.push_back(pt);
	}
	Voronoi * voro= new Voronoi(pts);
	_dcel= voro->_diagram;
	delete voro;

	//std::cout << *dcel;

	// alti aléatoire
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		face->_data= new DCEL_FaceData(rand_number(zmin, zmax));
	}

	// filtre passe-bas
	number k= 0.3;
	unsigned int iter= 5;
	for (unsigned int i=0; i<iter; ++i) {
		for (auto face : _dcel->_faces) {
			if (face->_outer_edge== NULL) {
				continue;
			}
			DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);

			number z= 0.0;
			std::vector<DCEL_Face *> faces= face->get_adjacent_faces();
			for (auto face_adj : faces) {
				if (face_adj->_outer_edge== NULL) {
					continue;
				}
				DCEL_FaceData * data_adj= (DCEL_FaceData *)(face_adj->_data);
				z+= data_adj->_z;
			}
			z/= faces.size();
			data->_z= z+ k*(data->_z- z);
		}
	}

	// suppression du biais
	number z_min= 1e8;
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		if (data->_z< z_min) {
			z_min= data->_z;
		}
	}
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		data->_z-= z_min;
	}

	// threshold des cellules à 0
	number threshold= 2.0;
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		if (data->_z< threshold) {
			data->_z= 0.0;
		}
	}

	// amplification
	number amp= 1.0;
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		data->_z*= amp;
	}

	update();
}


VoroZ::~VoroZ() {

}


void VoroZ::draw(const glm::mat4 & world2clip) {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::update() {
	_n_pts= 0;

	std::vector<float> vdata;

	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		std::vector<DCEL_Vertex *> vertices= face->get_vertices();
		pt_type g= face->get_gravity_center();
		DCEL_FaceData * face_data= (DCEL_FaceData *)(face->_data);
		for (unsigned int idx_vertex=0; idx_vertex<vertices.size(); ++idx_vertex) {
			_n_pts+= 3;
			
			unsigned int idx_vertex2= idx_vertex+ 1;
			if (idx_vertex2== vertices.size()) {
				idx_vertex2= 0;
			}

			vdata.push_back(float(vertices[idx_vertex]->_coords.x));
			vdata.push_back(float(vertices[idx_vertex]->_coords.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(face_data->_color.x);
			vdata.push_back(face_data->_color.y);
			vdata.push_back(face_data->_color.z);
			vdata.push_back(face_data->_color.w);

			vdata.push_back(float(vertices[idx_vertex2]->_coords.x));
			vdata.push_back(float(vertices[idx_vertex2]->_coords.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(face_data->_color.x);
			vdata.push_back(face_data->_color.y);
			vdata.push_back(face_data->_color.z);
			vdata.push_back(face_data->_color.w);

			vdata.push_back(float(g.x));
			vdata.push_back(float(g.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(face_data->_color.x);
			vdata.push_back(face_data->_color.y);
			vdata.push_back(face_data->_color.z);
			vdata.push_back(face_data->_color.w);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* 7, vdata.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool VoroZ::key_down(InputState * input_state, SDL_Keycode key) {
	return false;
}


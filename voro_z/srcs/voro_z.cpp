#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL_image.h>

#include "utile.h"
#include "voronoi.h"
#include "dcel.h"

#include "voro_z.h"


// --------------------------------------------------------
Biome::Biome() {

}


Biome::Biome(BiomeType type, number zmin, number zmax, glm::vec4 color, std::string texture_path) :
	_type(type), _zmin(zmin), _zmax(zmax), _color(color), _texture_path(texture_path)
{
	/*glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D, _texture_id);
	
	SDL_Surface * surface= IMG_Load(texture_path.c_str());
	if (!surface) {
		std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
		return;
	}
	// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_FreeSurface(surface);

	glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glActiveTexture(0);
	
	glBindTexture(GL_TEXTURE_2D, 0);*/
}


Biome::~Biome() {

}


// --------------------------------------------------------
DCEL_FaceData::DCEL_FaceData() : _z(0.0), _biome(NULL) {

}


DCEL_FaceData::DCEL_FaceData(number z) : _z(z), _biome(NULL)
{

}


DCEL_FaceData::~DCEL_FaceData() {

}


void DCEL_FaceData::set_biome(Biome * biome) {
	_biome= biome;
}


// --------------------------------------------------------
VoroZ::VoroZ() {

}


VoroZ::VoroZ(GLuint prog_draw_simple, GLuint prog_draw_texture) {
	_biomes[WATER]= new Biome(WATER, -10.0, 0.01, glm::vec4(0.1, 0.4, 0.8, 0.5), "../data/water.png");
	_biomes[COAST]= new Biome(COAST, 0.01, 10.0, glm::vec4(0.7, 0.7, 0.4, 0.8), "../data/sand.png");
	_biomes[FOREST]= new Biome(FOREST, 10.0, 100.0, glm::vec4(0.3, 0.8, 0.5, 0.9), "../data/grass.png");
	_biomes[MOUNTAIN]= new Biome(MOUNTAIN, 100.0, 200.0, glm::vec4(0.8, 0.8, 0.9, 1.0), "../data/snow.png");
	
	// ------------------------------------------------
	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 1024, 1024, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	unsigned int compt= 0;
	for (auto biome : _biomes) {
		biome.second->_idx_texture= compt;
		
		SDL_Surface * surface= IMG_Load(biome.second->_texture_path.c_str());
		if (!surface) {
			std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
			return;
		}

		// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
						0,                             // mipmap number
						0, 0, biome.second->_idx_texture,   // xoffset, yoffset, zoffset
						1024, 1024, 1, // width, height, depth
						GL_BGRA,                       // format
						GL_UNSIGNED_BYTE,              // type
						surface->pixels);              // pointer to data

		SDL_FreeSurface(surface);

		compt++;
	}

	glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_REPEAT);
	glActiveTexture(0);
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	// ------------------------------------------------
	GLuint buffers[4];
	glGenBuffers(4, buffers);

	_context_top_simple._buffer= buffers[0];
	_context_top_simple._prog= prog_draw_simple;
	_context_top_simple._locs["position"]= glGetAttribLocation(_context_top_simple._prog, "position_in");
	_context_top_simple._locs["color"]= glGetAttribLocation(_context_top_simple._prog, "color_in");
	_context_top_simple._locs["world2clip"]= glGetUniformLocation(_context_top_simple._prog, "world2clip_matrix");

	_context_top_texture._buffer= buffers[1];
	_context_top_texture._prog= prog_draw_texture;
	_context_top_texture._locs["position"]= glGetAttribLocation(_context_top_texture._prog, "position_in");
	_context_top_texture._locs["tex_coord"]= glGetAttribLocation(_context_top_texture._prog, "tex_coord_in");
	_context_top_texture._locs["current_layer"]= glGetAttribLocation(_context_top_texture._prog, "current_layer_in");
	_context_top_texture._locs["world2clip"]= glGetUniformLocation(_context_top_texture._prog, "world2clip_matrix");
	_context_top_texture._locs["texture_array"]= glGetUniformLocation(_context_top_texture._prog, "texture_array");

	_context_side_simple._buffer= buffers[2];
	_context_side_simple._prog= prog_draw_simple;
	_context_side_simple._locs["position"]= glGetAttribLocation(_context_side_simple._prog, "position_in");
	_context_side_simple._locs["color"]= glGetAttribLocation(_context_side_simple._prog, "color_in");
	_context_side_simple._locs["world2clip"]= glGetUniformLocation(_context_side_simple._prog, "world2clip_matrix");

	_context_side_texture._buffer= buffers[3];
	_context_side_texture._prog= prog_draw_texture;
	_context_side_texture._locs["position"]= glGetAttribLocation(_context_side_texture._prog, "position_in");
	_context_side_texture._locs["tex_coord"]= glGetAttribLocation(_context_side_texture._prog, "tex_coord_in");
	_context_side_texture._locs["current_layer"]= glGetAttribLocation(_context_side_texture._prog, "current_layer_in");
	_context_side_texture._locs["world2clip"]= glGetUniformLocation(_context_side_texture._prog, "world2clip_matrix");
	_context_side_texture._locs["texture_array"]= glGetUniformLocation(_context_side_texture._prog, "texture_array");

	int n_pts= 1000;
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
	unsigned int iter= 3;
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
	number threshold= 1.0;
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

	// biomes
	number z_max= -1e5;
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		if (data->_z> z_max) {
			z_max= data->_z;
		}
	}
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		if (data->_z< 1e-5) {
			data->set_biome(_biomes[WATER]);
		}
		else if (data->_z< z_max/ 3.0) {
			data->set_biome(_biomes[COAST]);
		}
		else if (data->_z< 2.0* z_max/ 3.0) {
			data->set_biome(_biomes[FOREST]);
		}
		else {
			data->set_biome(_biomes[MOUNTAIN]);
		}
	}

	update();
}


VoroZ::~VoroZ() {

}


void VoroZ::draw_top_simple(const glm::mat4 & world2clip) {
	glUseProgram(_context_top_simple._prog);
	glBindBuffer(GL_ARRAY_BUFFER, _context_top_simple._buffer);
	
	glEnableVertexAttribArray(_context_top_simple._locs["position"]);
	glEnableVertexAttribArray(_context_top_simple._locs["color"]);

	glUniformMatrix4fv(_context_top_simple._locs["world2clip"], 1, GL_FALSE, glm::value_ptr(world2clip));
	glVertexAttribPointer(_context_top_simple._locs["position"], 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
	glVertexAttribPointer(_context_top_simple._locs["color"], 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts_tops);

	glDisableVertexAttribArray(_context_top_simple._locs["position"]);
	glDisableVertexAttribArray(_context_top_simple._locs["color"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::draw_top_texture(const glm::mat4 & world2clip) {
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(_context_top_texture._prog);
	glBindBuffer(GL_ARRAY_BUFFER, _context_top_texture._buffer);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);

	glUniform1i(_context_top_texture._locs["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(_context_top_texture._locs["world2clip"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(_context_top_texture._locs["position"]);
	glEnableVertexAttribArray(_context_top_texture._locs["tex_coord"]);
	glEnableVertexAttribArray(_context_top_texture._locs["current_layer"]);

	glVertexAttribPointer(_context_top_texture._locs["position"], 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_context_top_texture._locs["tex_coord"], 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_context_top_texture._locs["current_layer"], 1, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(5* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts_tops);

	glDisableVertexAttribArray(_context_top_texture._locs["position"]);
	glDisableVertexAttribArray(_context_top_texture._locs["tex_coord"]);
	glDisableVertexAttribArray(_context_top_texture._locs["current_layer"]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glActiveTexture(0);
}


void VoroZ::draw_side_simple(const glm::mat4 & world2clip) {
	glUseProgram(_context_side_simple._prog);
	glBindBuffer(GL_ARRAY_BUFFER, _context_side_simple._buffer);
	
	glEnableVertexAttribArray(_context_side_simple._locs["position"]);
	glEnableVertexAttribArray(_context_side_simple._locs["color"]);

	glUniformMatrix4fv(_context_side_simple._locs["world2clip"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glVertexAttribPointer(_context_side_simple._locs["position"], 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
	glVertexAttribPointer(_context_side_simple._locs["color"], 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts_sides);

	glDisableVertexAttribArray(_context_side_simple._locs["position"]);
	glDisableVertexAttribArray(_context_side_simple._locs["color"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::draw_side_texture(const glm::mat4 & world2clip) {
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(_context_side_texture._prog);
	glBindBuffer(GL_ARRAY_BUFFER, _context_side_texture._buffer);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);

	glUniform1i(_context_side_texture._locs["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(_context_side_texture._locs["world2clip"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(_context_side_texture._locs["position"]);
	glEnableVertexAttribArray(_context_side_texture._locs["tex_coord"]);
	glEnableVertexAttribArray(_context_side_texture._locs["current_layer"]);

	glVertexAttribPointer(_context_side_texture._locs["position"], 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_context_side_texture._locs["tex_coord"], 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_context_side_texture._locs["current_layer"], 1, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(5* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts_sides);

	glDisableVertexAttribArray(_context_side_texture._locs["position"]);
	glDisableVertexAttribArray(_context_side_texture._locs["tex_coord"]);
	glDisableVertexAttribArray(_context_side_texture._locs["current_layer"]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glActiveTexture(0);
}


void VoroZ::draw(const glm::mat4 & world2clip) {
	//draw_top_simple(world2clip);
	draw_top_texture(world2clip);
	
	draw_side_simple(world2clip);
	//draw_side_texture(world2clip);
}


void VoroZ::update_top_simple() {
	_n_pts_tops= 0;

	std::vector<float> vdata;

	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		std::vector<DCEL_Vertex *> vertices= face->get_vertices();
		pt_type g= face->get_gravity_center();
		DCEL_FaceData * face_data= (DCEL_FaceData *)(face->_data);
		for (unsigned int idx_vertex=0; idx_vertex<vertices.size(); ++idx_vertex) {
			_n_pts_tops+= 3;
			
			unsigned int idx_vertex2= idx_vertex+ 1;
			if (idx_vertex2== vertices.size()) {
				idx_vertex2= 0;
			}

			vdata.push_back(float(vertices[idx_vertex]->_coords.x));
			vdata.push_back(float(vertices[idx_vertex]->_coords.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(face_data->_biome->_color.x);
			vdata.push_back(face_data->_biome->_color.y);
			vdata.push_back(face_data->_biome->_color.z);
			vdata.push_back(face_data->_biome->_color.w);

			vdata.push_back(float(vertices[idx_vertex2]->_coords.x));
			vdata.push_back(float(vertices[idx_vertex2]->_coords.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(face_data->_biome->_color.x);
			vdata.push_back(face_data->_biome->_color.y);
			vdata.push_back(face_data->_biome->_color.z);
			vdata.push_back(face_data->_biome->_color.w);

			vdata.push_back(float(g.x));
			vdata.push_back(float(g.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(face_data->_biome->_color.x);
			vdata.push_back(face_data->_biome->_color.y);
			vdata.push_back(face_data->_biome->_color.z);
			vdata.push_back(face_data->_biome->_color.w);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _context_top_simple._buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts_tops* 7, vdata.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_top_texture() {
	_n_pts_tops= 0;

	std::vector<float> vdata;

	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		std::vector<DCEL_Vertex *> vertices= face->get_vertices();
		pt_type g= face->get_gravity_center();
		DCEL_FaceData * face_data= (DCEL_FaceData *)(face->_data);
		for (unsigned int idx_vertex=0; idx_vertex<vertices.size(); ++idx_vertex) {
			_n_pts_tops+= 3;
			
			unsigned int idx_vertex2= idx_vertex+ 1;
			if (idx_vertex2== vertices.size()) {
				idx_vertex2= 0;
			}

			vdata.push_back(float(vertices[idx_vertex]->_coords.x));
			vdata.push_back(float(vertices[idx_vertex]->_coords.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(0.0);
			vdata.push_back(0.0);
			vdata.push_back(float(face_data->_biome->_idx_texture));

			vdata.push_back(float(vertices[idx_vertex2]->_coords.x));
			vdata.push_back(float(vertices[idx_vertex2]->_coords.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(0.0);
			vdata.push_back(1.0);
			vdata.push_back(float(face_data->_biome->_idx_texture));

			vdata.push_back(float(g.x));
			vdata.push_back(float(g.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(1.0);
			vdata.push_back(1.0);
			vdata.push_back(float(face_data->_biome->_idx_texture));
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _context_top_texture._buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts_tops* 6, vdata.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_side_simple() {
	_n_pts_sides= 0;
	std::vector<float> vdata;
	glm::vec4 color= {0.5, 0.3, 0.2, 1.0};

	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		number face_z= data->_z;

		std::vector<DCEL_Face *> faces= face->get_adjacent_faces();
		for (auto face_adj : faces) {
			if (face_adj->_outer_edge== NULL) {
				continue;
			}
			DCEL_FaceData * data_adj= (DCEL_FaceData *)(face_adj->_data);
			number face_adj_z= data_adj->_z;
			if (face_adj_z> face_z) {
				_n_pts_sides+= 6;

				DCEL_HalfEdge * edge= _dcel->get_dividing_edge(face_adj, face);
				pt_type p1= edge->_origin->_coords;
				pt_type p2= edge->destination()->_coords;
				
				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);

				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);

				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _context_side_simple._buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts_sides* 7, vdata.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_side_texture() {
	_n_pts_sides= 0;
	std::vector<float> vdata;
	glm::vec4 color= {0.5, 0.3, 0.2, 1.0};

	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * face_data= (DCEL_FaceData *)(face->_data);
		number face_z= face_data->_z;

		std::vector<DCEL_Face *> faces= face->get_adjacent_faces();
		for (auto face_adj : faces) {
			if (face_adj->_outer_edge== NULL) {
				continue;
			}
			DCEL_FaceData * face_data_adj= (DCEL_FaceData *)(face_adj->_data);
			number face_adj_z= face_data_adj->_z;
			if (face_adj_z> face_z) {
				_n_pts_sides+= 6;

				DCEL_HalfEdge * edge= _dcel->get_dividing_edge(face, face_adj);
				pt_type p1= edge->destination()->_coords;
				pt_type p2= edge->_origin->_coords;
				
				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_z));
				vdata.push_back(0.0);
				vdata.push_back(0.0);
				vdata.push_back(float(face_data->_biome->_idx_texture));

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_z));
				vdata.push_back(1.0);
				vdata.push_back(0.0);
				vdata.push_back(float(face_data->_biome->_idx_texture));

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(1.0);
				vdata.push_back(1.0);
				vdata.push_back(float(face_data->_biome->_idx_texture));

				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_z));
				vdata.push_back(0.0);
				vdata.push_back(0.0);
				vdata.push_back(float(face_data->_biome->_idx_texture));

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(1.0);
				vdata.push_back(1.0);
				vdata.push_back(float(face_data->_biome->_idx_texture));

				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(0.0);
				vdata.push_back(1.0);
				vdata.push_back(float(face_data->_biome->_idx_texture));
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _context_side_texture._buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts_sides* 6, vdata.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update() {
	update_top_simple();
	update_side_simple();
	update_top_texture();
	update_side_texture();
}


bool VoroZ::key_down(InputState * input_state, SDL_Keycode key) {
	return false;
}


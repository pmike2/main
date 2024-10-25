#include <algorithm>
#include <random>

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
DrawContext::DrawContext() {

}


DrawContext::DrawContext(GLuint prog, GLuint buffer, std::vector<std::string> locs_attrib, std::vector<std::string> locs_uniform) :
	_prog(prog), _buffer(buffer) 
{
	for (auto loc : locs_attrib) {
		_locs[loc]= glGetAttribLocation(_prog, loc.c_str());
	}
	for (auto loc : locs_uniform) {
		_locs[loc]= glGetUniformLocation(_prog, loc.c_str());
	}
}


DrawContext::~DrawContext() {

}


// --------------------------------------------------------
Light::Light() : _position_ini(glm::vec3(0.0)), _position(glm::vec3(0.0)), _color(glm::vec3(1.0)), _animated(true) {

}


Light::Light(glm::vec3 position_ini, glm::vec3 color) : _position_ini(position_ini), _position(position_ini), _color(color), _animated(true) {

}


Light::~Light() {

}


void Light::anim() {
	const number delta= 0.1;

	if (!_animated) {
		return;
	}
	
	_position.x+= delta;
	
	if (_position.x< 0.0) {
		_position.z+= delta;
	}
	else {
		_position.z-= delta;
	}

	if (_position.z< 0.0) {
		_position= glm::vec3(_position_ini);
	}
}


// --------------------------------------------------------
VoroZ::VoroZ() {

}


VoroZ::VoroZ(GLuint prog_draw_simple, GLuint prog_draw_texture, GLuint prog_draw_light) {
	init_biome();
	init_context(prog_draw_simple, prog_draw_texture, prog_draw_light);
	init_texture();
	init_light();
	init_dcel();
	update();
}


VoroZ::~VoroZ() {
	delete _context_simple;
	delete _context_texture;
	delete _context_light;
	delete _dcel;
	delete _light;
}


void VoroZ::init_biome() {
	_biomes[WATER]= new Biome(WATER, -10.0, 0.01, glm::vec4(0.1, 0.4, 0.8, 0.5), "../data/water.png");
	_biomes[COAST]= new Biome(COAST, 0.01, 10.0, glm::vec4(0.7, 0.7, 0.4, 0.8), "../data/sand.png");
	_biomes[FOREST]= new Biome(FOREST, 10.0, 100.0, glm::vec4(0.3, 0.8, 0.5, 0.9), "../data/grass.png");
	_biomes[MOUNTAIN]= new Biome(MOUNTAIN, 100.0, 200.0, glm::vec4(0.8, 0.8, 0.9, 1.0), "../data/snow.png");
}


void VoroZ::init_context(GLuint prog_draw_simple, GLuint prog_draw_texture, GLuint prog_draw_light) {
	GLuint buffers[3];
	glGenBuffers(3, buffers);

	_context_simple= new DrawContext(prog_draw_simple, buffers[0], 
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});
	
	_context_texture= new DrawContext(prog_draw_texture, buffers[1], 
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in"}, 
		std::vector<std::string>{"world2clip_matrix", "texture_array"});
	
	_context_light= new DrawContext(prog_draw_light, buffers[2], 
		std::vector<std::string>{"position_in", "color_in", "normal_in"}, 
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"});
}


void VoroZ::init_texture() {
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

		// sais pas pourquoi mais format == GL_BGRA fonctionne mieux que GL_RGBA
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
			0,                                // mipmap number
			0, 0, biome.second->_idx_texture, // xoffset, yoffset, zoffset
			1024, 1024, 1,                    // width, height, depth
			GL_BGRA,                          // format
			GL_UNSIGNED_BYTE,                 // type
			surface->pixels);                 // pointer to data

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
}


void VoroZ::init_light() {
	_light= new Light(glm::vec3(0.0, 0.0, 100.0), glm::vec3(1.0, 1.0, 1.0));
}


void VoroZ::init_dcel() {
	const unsigned int n_pts= 1000;
	const number xmin= -100.0;
	const number xmax= 100.0;
	const number ymin= -100.0;
	const number ymax= 100.0;
	const number deviation= 20.0;
	const number zmin= 0.0;
	const number zmax= 50.0;
	const unsigned int n_holes= 4;
	const number hole_radius= 30.0;
	const number lowfilter_cut= 0.7;
	const unsigned int lowfilter_iter= 7;
	const number water_threshold= 5.0;
	const number amplification= 1.0;

	// pts aléatoires
	std::vector<pt_type> pts;

	std::random_device rd{};
	std::mt19937 gen{rd()};
	std::normal_distribution<number> d{0.0, deviation};

	for (unsigned int i=0; i<n_pts; ++i) {
		//pt_type pt(rand_number(xmin, xmax), rand_number(ymin, ymax));
		pt_type pt(d(gen), d(gen));
		pts.push_back(pt);
	}
	
	// trous
	for (unsigned int i=0; i<n_holes; ++i) {
		//pt_type center(rand_number(xmin, xmax), rand_number(ymin, ymax));
		pt_type center(d(gen), d(gen));
		pts.erase(std::remove_if(pts.begin(), pts.end(), [center, hole_radius](pt_type pt){
			return point_in_circle(center, hole_radius, pt);
		}), pts.end());
	}

	// calcul diagramme de Voronoi
	Voronoi * voro= new Voronoi(pts);
	_dcel= voro->_diagram;
	delete voro;

	// alti aléatoire
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		face->_data= new DCEL_FaceData(rand_number(zmin, zmax));
	}

	// filtre passe-bas
	for (unsigned int i=0; i<lowfilter_iter; ++i) {
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
			data->_z= z+ lowfilter_cut* (data->_z- z);
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
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		if (data->_z< water_threshold) {
			data->_z= 0.0;
		}
	}

	// amplification
	for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		data->_z*= amplification;
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
}


void VoroZ::draw_simple(const glm::mat4 & world2clip) {
	glUseProgram(_context_simple->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _context_simple->_buffer);
	
	glUniformMatrix4fv(_context_simple->_locs["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(_context_simple->_locs["position_in"]);
	glEnableVertexAttribArray(_context_simple->_locs["color_in"]);

	glVertexAttribPointer(_context_simple->_locs["position_in"], 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
	glVertexAttribPointer(_context_simple->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_context_simple->_locs["position_in"]);
	glDisableVertexAttribArray(_context_simple->_locs["color_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::draw_texture(const glm::mat4 & world2clip) {
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(_context_texture->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _context_texture->_buffer);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);

	glUniform1i(_context_texture->_locs["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(_context_texture->_locs["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(_context_texture->_locs["position_in"]);
	glEnableVertexAttribArray(_context_texture->_locs["tex_coord_in"]);
	glEnableVertexAttribArray(_context_texture->_locs["current_layer_in"]);

	glVertexAttribPointer(_context_texture->_locs["position_in"], 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_context_texture->_locs["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_context_texture->_locs["current_layer_in"], 1, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(5* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_context_texture->_locs["position_in"]);
	glDisableVertexAttribArray(_context_texture->_locs["tex_coord_in"]);
	glDisableVertexAttribArray(_context_texture->_locs["current_layer_in"]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glActiveTexture(0);
}


void VoroZ::draw_light(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	glUseProgram(_context_light->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _context_light->_buffer);
	
	glUniformMatrix4fv(_context_light->_locs["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	glUniform3fv(_context_light->_locs["light_position"], 1, glm::value_ptr(_light->_position));
	glUniform3fv(_context_light->_locs["light_color"], 1, glm::value_ptr(_light->_color));
	glUniform3fv(_context_light->_locs["view_position"], 1, glm::value_ptr(camera_position));

	glEnableVertexAttribArray(_context_light->_locs["position_in"]);
	glEnableVertexAttribArray(_context_light->_locs["color_in"]);
	glEnableVertexAttribArray(_context_light->_locs["normal_in"]);

	glVertexAttribPointer(_context_light->_locs["position_in"], 3, GL_FLOAT, GL_FALSE, 10* sizeof(float), (void*)0);
	glVertexAttribPointer(_context_light->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, 10* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_context_light->_locs["normal_in"], 3, GL_FLOAT, GL_FALSE, 10* sizeof(float), (void*)(7* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_context_light->_locs["position_in"]);
	glDisableVertexAttribArray(_context_light->_locs["color_in"]);
	glDisableVertexAttribArray(_context_light->_locs["normal_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::draw(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	//draw_simple(world2clip);
	//draw_texture(world2clip);
	draw_light(world2clip, camera_position);
}


void VoroZ::update_simple() {
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
				_n_pts+= 6;

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

	glBindBuffer(GL_ARRAY_BUFFER, _context_simple->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* 7, vdata.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_texture() {
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
				_n_pts+= 6;

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

	glBindBuffer(GL_ARRAY_BUFFER, _context_texture->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* 6, vdata.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_light() {
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
			vdata.push_back(face_data->_biome->_color.x);
			vdata.push_back(face_data->_biome->_color.y);
			vdata.push_back(face_data->_biome->_color.z);
			vdata.push_back(face_data->_biome->_color.w);
			vdata.push_back(0.0);
			vdata.push_back(0.0);
			vdata.push_back(1.0);

			vdata.push_back(float(vertices[idx_vertex2]->_coords.x));
			vdata.push_back(float(vertices[idx_vertex2]->_coords.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(face_data->_biome->_color.x);
			vdata.push_back(face_data->_biome->_color.y);
			vdata.push_back(face_data->_biome->_color.z);
			vdata.push_back(face_data->_biome->_color.w);
			vdata.push_back(0.0);
			vdata.push_back(0.0);
			vdata.push_back(1.0);

			vdata.push_back(float(g.x));
			vdata.push_back(float(g.y));
			vdata.push_back(float(face_data->_z));
			vdata.push_back(face_data->_biome->_color.x);
			vdata.push_back(face_data->_biome->_color.y);
			vdata.push_back(face_data->_biome->_color.z);
			vdata.push_back(face_data->_biome->_color.w);
			vdata.push_back(0.0);
			vdata.push_back(0.0);
			vdata.push_back(1.0);
		}
	}

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
				_n_pts+= 6;

				DCEL_HalfEdge * edge= _dcel->get_dividing_edge(face_adj, face);
				pt_type p1= edge->_origin->_coords;
				pt_type p2= edge->destination()->_coords;
				number dx= p2.x- p1.x;
				number dy= p2.y- p1.y;
				
				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);
				vdata.push_back(float(dy));
				vdata.push_back(-float(dx));
				vdata.push_back(0.0);

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);
				vdata.push_back(float(dy));
				vdata.push_back(-float(dx));
				vdata.push_back(0.0);

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);
				vdata.push_back(float(dy));
				vdata.push_back(-float(dx));
				vdata.push_back(0.0);

				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);
				vdata.push_back(float(dy));
				vdata.push_back(-float(dx));
				vdata.push_back(0.0);

				vdata.push_back(float(p2.x));
				vdata.push_back(float(p2.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);
				vdata.push_back(float(dy));
				vdata.push_back(-float(dx));
				vdata.push_back(0.0);

				vdata.push_back(float(p1.x));
				vdata.push_back(float(p1.y));
				vdata.push_back(float(face_adj_z));
				vdata.push_back(color.x);
				vdata.push_back(color.y);
				vdata.push_back(color.z);
				vdata.push_back(color.w);
				vdata.push_back(float(dy));
				vdata.push_back(-float(dx));
				vdata.push_back(0.0);
			}
		}
	}

	DCEL_Face * unbounded_face= _dcel->get_unbounded_face();
	std::vector<std::vector<DCEL_HalfEdge *> > inner_edges= unbounded_face->get_inner_edges();
	for (const auto & unbounded_edge : inner_edges[0]) {
		DCEL_HalfEdge * edge= unbounded_edge->_twin;
		DCEL_Face * face= edge->_incident_face;
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		number face_z= data->_z;
		if (face_z> 0.0) {
			_n_pts+= 6;

			pt_type p1= edge->_origin->_coords;
			pt_type p2= edge->destination()->_coords;
			number dx= p2.x- p1.x;
			number dy= p2.y- p1.y;
			
			vdata.push_back(float(p1.x));
			vdata.push_back(float(p1.y));
			vdata.push_back(float(0.0));
			vdata.push_back(color.x);
			vdata.push_back(color.y);
			vdata.push_back(color.z);
			vdata.push_back(color.w);
			vdata.push_back(float(dy));
			vdata.push_back(-float(dx));
			vdata.push_back(0.0);

			vdata.push_back(float(p2.x));
			vdata.push_back(float(p2.y));
			vdata.push_back(float(0.0));
			vdata.push_back(color.x);
			vdata.push_back(color.y);
			vdata.push_back(color.z);
			vdata.push_back(color.w);
			vdata.push_back(float(dy));
			vdata.push_back(-float(dx));
			vdata.push_back(0.0);

			vdata.push_back(float(p2.x));
			vdata.push_back(float(p2.y));
			vdata.push_back(float(face_z));
			vdata.push_back(color.x);
			vdata.push_back(color.y);
			vdata.push_back(color.z);
			vdata.push_back(color.w);
			vdata.push_back(float(dy));
			vdata.push_back(-float(dx));
			vdata.push_back(0.0);

			vdata.push_back(float(p1.x));
			vdata.push_back(float(p1.y));
			vdata.push_back(float(0.0));
			vdata.push_back(color.x);
			vdata.push_back(color.y);
			vdata.push_back(color.z);
			vdata.push_back(color.w);
			vdata.push_back(float(dy));
			vdata.push_back(-float(dx));
			vdata.push_back(0.0);

			vdata.push_back(float(p2.x));
			vdata.push_back(float(p2.y));
			vdata.push_back(float(face_z));
			vdata.push_back(color.x);
			vdata.push_back(color.y);
			vdata.push_back(color.z);
			vdata.push_back(color.w);
			vdata.push_back(float(dy));
			vdata.push_back(-float(dx));
			vdata.push_back(0.0);

			vdata.push_back(float(p1.x));
			vdata.push_back(float(p1.y));
			vdata.push_back(float(face_z));
			vdata.push_back(color.x);
			vdata.push_back(color.y);
			vdata.push_back(color.z);
			vdata.push_back(color.w);
			vdata.push_back(float(dy));
			vdata.push_back(-float(dx));
			vdata.push_back(0.0);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _context_light->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* 10, vdata.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update() {
	update_simple();
	update_texture();
	update_light();
}


void VoroZ::anim() {
	_light->anim();
}


bool VoroZ::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_a) {
		init_dcel();
		update();
		return true;
	}

	if (key== SDLK_b) {
		_light->_animated= !_light->_animated;
	}

	return false;
}


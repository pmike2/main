#include <algorithm>
#include <random>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <SDL2/SDL_image.h>

#include "utile.h"
#include "voronoi.h"
#include "dcel.h"

#include "voro_z.h"


// test anisotropic filter
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE


glm::vec3 normal(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3) {
	glm::vec3 v12(p2- p1);
	glm::vec3 v13(p3- p1);

	glm::vec3 result(
		v12.y* v13.z- v12.z* v13.y,
		v12.z* v13.x- v12.x* v13.z,
		v12.x* v13.y- v12.y* v13.x
	);

	//return result/ sqrt(result.x* result.x+ result.y* result.y+ result.z* result.z);
	return result;
}


// cf https://learnopengl.com/Advanced-Lighting/Normal-Mapping
glm::vec3 tangent(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3, const glm::vec2 & uv1, const glm::vec2 & uv2, const glm::vec2 & uv3) {
	glm::vec3 result;
	glm::vec3 e1= p2- p1;
	glm::vec3 e2= p3- p1;
	glm::vec2 delta_uv1= uv2- uv1;
	glm::vec2 delta_uv2= uv3- uv1;
	number f= 1.0 / (delta_uv1.x * delta_uv2.y- delta_uv2.x * delta_uv1.y);
	result.x = f * (delta_uv2.y * e1.x - delta_uv1.y * e2.x);
	result.y = f * (delta_uv2.y * e1.y - delta_uv1.y * e2.y);
	result.z = f * (delta_uv2.y * e1.z - delta_uv1.y * e2.z);

	return result;
}


// tentative de transfo d'un triangle vers un triangle plat pour application texture parallaxe
/*glm::vec3 triangle2euler(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3) {
	const number EPS= 1e-7;
	number a= sqrt(p3.x* p3.x+ p3.y* p3.y);
	number precession, nutation, rotation;
	if (a> EPS) {
		precession= atan2(p2.x* p3.y- p2.y* p3.x, p1.x* p3.y- p1.y* p3.x);
		nutation= atan2(a, p3.z);
		rotation= -atan2(-p3.x, p3.y);
	}
	else {
		precession= 0.0;
		nutation= (p3.z> 0.0) ? 0.0 : M_PI;
		rotation= -atan2(p1.y, p1.x);
	}
	return glm::vec3(precession, nutation, rotation);
}


glm::mat3 triangle2mat(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3) {
	glm::vec3 angles= triangle2euler(p1, p2, p3);
	float cos_psi= cos(angles[0]); float sin_psi= sin(angles[0]);
	float cos_theta= cos(angles[1]); float sin_theta= sin(angles[1]);
	float cos_phi= cos(angles[2]); float sin_phi= sin(angles[2]);

	return glm::mat3(
		cos_psi* cos_phi- sin_psi* cos_theta* sin_phi, sin_psi* cos_phi+ cos_psi* cos_theta* sin_phi, sin_theta* sin_phi,
		-cos_psi* sin_phi- sin_psi* cos_theta* cos_phi, -sin_psi* sin_phi+ cos_psi* cos_theta* cos_phi, -cos_psi* sin_theta,
		sin_theta* sin_phi, sin_theta* cos_phi, cos_theta
	);
}*/


// --------------------------------------------------------
Biome::Biome() {

}


Biome::Biome(BiomeType type, number zmin, number zmax, glm::vec4 color, float uv_factor, std::string diffuse_texture_path, std::string normal_texture_path, std::string parallax_texture_path, float anim_speed) :
	_type(type), _zmin(zmin), _zmax(zmax), _color(color), _uv_factor(uv_factor), _diffuse_texture_path(diffuse_texture_path), 
	_normal_texture_path(normal_texture_path), _parallax_texture_path(parallax_texture_path), _anim_speed(anim_speed)
{
	if (_diffuse_texture_path.find(".png")!= std::string::npos) {
		_diffuse_pngs.push_back(_diffuse_texture_path);
	}
	else {
		_diffuse_pngs= list_files(_diffuse_texture_path, "png");
	}

	if (_normal_texture_path.find(".png")!= std::string::npos) {
		_normal_pngs.push_back(_normal_texture_path);
	}
	else {
		_normal_pngs= list_files(_normal_texture_path, "png");
	}

	/*for (auto png : _diffuse_pngs) {
		std::cout << "diffuse : " << png << "\n";
	}
	for (auto png : _normal_pngs) {
		std::cout << "normal : " << png << "\n";
	}*/
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
TriangleData::TriangleData() {

}


TriangleData::TriangleData(
	const glm::vec3 & pt1, const glm::vec3 & pt2, const glm::vec3 & pt3, 
	const glm::vec2 & uv1_diffuse, const glm::vec2 & uv2_diffuse, const glm::vec2 & uv3_diffuse,
	const glm::vec2 & uv1_normal, const glm::vec2 & uv2_normal, const glm::vec2 & uv3_normal,
	const glm::vec2 & uv1_parallax, const glm::vec2 & uv2_parallax, const glm::vec2 & uv3_parallax,
	Biome * biome) :
	_biome(biome)
{
	_pts[0]= glm::vec3(pt1);
	_pts[1]= glm::vec3(pt2);
	_pts[2]= glm::vec3(pt3);
	_uvs_diffuse[0]= glm::vec2(uv1_diffuse);
	_uvs_diffuse[1]= glm::vec2(uv2_diffuse);
	_uvs_diffuse[2]= glm::vec2(uv3_diffuse);
	_uvs_normal[0]= glm::vec2(uv1_normal);
	_uvs_normal[1]= glm::vec2(uv2_normal);
	_uvs_normal[2]= glm::vec2(uv3_normal);
	_uvs_parallax[0]= glm::vec2(uv1_parallax);
	_uvs_parallax[1]= glm::vec2(uv2_parallax);
	_uvs_parallax[2]= glm::vec2(uv3_parallax);
	_normal= normal(_pts[0], _pts[1], _pts[2]);
	_tangent= tangent(_pts[0], _pts[1], _pts[2], _uvs_normal[0], _uvs_normal[1], _uvs_normal[2]);
}


TriangleData::~TriangleData() {

}


// --------------------------------------------------------
VoroZ::VoroZ() {

}


VoroZ::VoroZ(std::map<std::string, GLuint> progs) :
	_draw_mode(NORMAL)
{
	init_biome();
	init_context(progs);
	init_texture_diffuse();
	init_texture_normal();
	init_texture_parallax();
	init_light();
	init_dcel();
	update();
}


VoroZ::~VoroZ() {
	for (auto context  : _contexts) {
		delete context.second;
	}
	_contexts.clear();
	delete _dcel;
	delete _light;
}


void VoroZ::init_biome() {
	_biomes[WATER]= new Biome(WATER, -10.0, 0.01, glm::vec4(0.1, 0.4, 0.8, 0.5), 0.007, "../data/water/diffuse", "../data/water/normal", "../data/brick_parallax.png", 0.3);
	_biomes[COAST]= new Biome(COAST, 0.01, 10.0, glm::vec4(0.7, 0.7, 0.4, 0.8), 0.02, "../data/sand.png", "../data/sand_normal.png", "../data/brick_parallax.png", 0.3);
	_biomes[FOREST]= new Biome(FOREST, 10.0, 100.0, glm::vec4(0.3, 0.8, 0.5, 0.9), 0.02, "../data/grass/diffuse", "../data/grass/normal", "../data/brick_parallax.png", 0.1);
	_biomes[MOUNTAIN]= new Biome(MOUNTAIN, 100.0, 200.0, glm::vec4(0.8, 0.8, 0.9, 1.0), 0.02, "../data/snow.png", "../data/snow_normal.png", "../data/brick_parallax.png", 0.3);
	_biomes[DIRT]= new Biome(DIRT, 0.0, 0.0, glm::vec4(0.5, 0.3, 0.2, 1.0), 0.04, "../data/dirt.png", "../data/dirt_normal.png", "../data/brick_parallax.png", 0.3);
}


void VoroZ::init_context(std::map<std::string, GLuint> progs) {
	GLuint buffers[5];
	glGenBuffers(5, buffers);

	_contexts["simple"]= new DrawContext(progs["repere"], buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});
	
	_contexts["texture"]= new DrawContext(progs["texture"], buffers[1],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in"},
		std::vector<std::string>{"world2clip_matrix", "diffuse_texture_array"});
	
	_contexts["light"]= new DrawContext(progs["light"], buffers[2],
		std::vector<std::string>{"position_in", "color_in", "normal_in"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"});

	_contexts["normal"]= new DrawContext(progs["normal"], buffers[3],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_diffuse_in", "current_layer_normal_in", "normal_in", "tangent_in"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position", "diffuse_texture_array", "normal_texture_array"});

	_contexts["parallax"]= new DrawContext(progs["parallax"], buffers[4],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_diffuse_in", "current_layer_normal_in", "current_layer_parallax_in", "normal_in", "tangent_in"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position", "diffuse_texture_array", "normal_texture_array", "parallax_texture_array", "height_scale"});
}


void VoroZ::init_texture_diffuse() {
	glGenTextures(1, &_texture_id_diffuse);

	glActiveTexture(GL_TEXTURE0+ 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_diffuse);

	unsigned int n_textures= 0;
	for (auto biome : _biomes) {
		n_textures+= biome.second->_diffuse_pngs.size();
	}
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, n_textures, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	unsigned int compt= 0;
	for (auto biome : _biomes) {
		biome.second->_diffuse_texture_idx_start= compt;
		biome.second->_diffuse_texture_idx_current= float(compt);
		for (auto png : biome.second->_diffuse_pngs) {
			SDL_Surface * surface= IMG_Load(png.c_str());
			if (!surface) {
				std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
				return;
			}

			// sais pas pourquoi mais format == GL_BGRA fonctionne mieux que GL_RGBA
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
				0,                                // mipmap number
				0, 0, compt,                      // xoffset, yoffset, zoffset
				1024, 1024, 1,                    // width, height, depth
				GL_BGRA,                          // format
				GL_UNSIGNED_BYTE,                 // type
				surface->pixels);                 // pointer to data

			SDL_FreeSurface(surface);
			compt++;
		}
		biome.second->_diffuse_texture_idx_end= compt- 1;
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_REPEAT);
	glActiveTexture(0);
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}


void VoroZ::init_texture_normal() {
	glGenTextures(1, &_texture_id_normal);

	glActiveTexture(GL_TEXTURE0+ 1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_normal);

	unsigned int n_textures= 0;
	for (auto biome : _biomes) {
		n_textures+= biome.second->_normal_pngs.size();
	}
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, n_textures, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY); // allocation mipmaps

	unsigned int compt= 0;
	for (auto biome : _biomes) {
		biome.second->_normal_texture_idx_start= compt;
		biome.second->_normal_texture_idx_current= float(compt);
		for (auto png : biome.second->_normal_pngs) {
			SDL_Surface * surface= IMG_Load(png.c_str());
			if (!surface) {
				std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
				return;
			}

			// sais pas pourquoi mais format == GL_BGRA fonctionne mieux que GL_RGBA
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
				0,                                // mipmap number
				0, 0, compt,                      // xoffset, yoffset, zoffset
				1024, 1024, 1,                    // width, height, depth
				GL_BGRA,                          // format
				GL_UNSIGNED_BYTE,                 // type
				surface->pixels);                 // pointer to data

			SDL_FreeSurface(surface);
			compt++;
		}
		biome.second->_normal_texture_idx_end= compt- 1;
	}

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY); // generation mipmaps
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
	glActiveTexture(0);
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}


void VoroZ::init_texture_parallax() {
	glGenTextures(1, &_texture_id_parallax);

	glActiveTexture(GL_TEXTURE0+ 2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_parallax);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, _biomes.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY); // allocation mipmaps

	unsigned int compt= 0;
	for (auto biome : _biomes) {
		biome.second->_parallax_texture_idx= compt;
		SDL_Surface * surface= IMG_Load(biome.second->_parallax_texture_path.c_str());
		if (!surface) {
			std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
			return;
		}

		// sais pas pourquoi mais format == GL_BGRA fonctionne mieux que GL_RGBA
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
			0,                                // mipmap number
			0, 0, biome.second->_parallax_texture_idx, // xoffset, yoffset, zoffset
			1024, 1024, 1,                    // width, height, depth
			GL_BGRA,                          // format
			GL_UNSIGNED_BYTE,                 // type
			surface->pixels);                 // pointer to data

		SDL_FreeSurface(surface);

		compt++;
	}

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY); // generation mipmaps
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
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
	glUseProgram(_contexts["simple"]->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _contexts["simple"]->_buffer);
	
	glUniformMatrix4fv(_contexts["simple"]->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(_contexts["simple"]->_locs_attrib["position_in"]);
	glEnableVertexAttribArray(_contexts["simple"]->_locs_attrib["color_in"]);

	glVertexAttribPointer(_contexts["simple"]->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
	glVertexAttribPointer(_contexts["simple"]->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_contexts["simple"]->_locs_attrib["position_in"]);
	glDisableVertexAttribArray(_contexts["simple"]->_locs_attrib["color_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::draw_texture(const glm::mat4 & world2clip) {
	glActiveTexture(GL_TEXTURE0+ 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_diffuse);
	glActiveTexture(0);

	glUseProgram(_contexts["texture"]->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _contexts["texture"]->_buffer);

	glUniform1i(_contexts["texture"]->_locs_uniform["diffuse_texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(_contexts["texture"]->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(_contexts["texture"]->_locs_attrib["position_in"]);
	glEnableVertexAttribArray(_contexts["texture"]->_locs_attrib["tex_coord_in"]);
	glEnableVertexAttribArray(_contexts["texture"]->_locs_attrib["current_layer_in"]);

	glVertexAttribPointer(_contexts["texture"]->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_contexts["texture"]->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_contexts["texture"]->_locs_attrib["current_layer_in"], 1, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(5* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_contexts["texture"]->_locs_attrib["position_in"]);
	glDisableVertexAttribArray(_contexts["texture"]->_locs_attrib["tex_coord_in"]);
	glDisableVertexAttribArray(_contexts["texture"]->_locs_attrib["current_layer_in"]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::draw_light(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	glUseProgram(_contexts["light"]->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _contexts["light"]->_buffer);
	
	glUniformMatrix4fv(_contexts["light"]->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	glUniform3fv(_contexts["light"]->_locs_uniform["light_position"], 1, glm::value_ptr(_light->_position));
	glUniform3fv(_contexts["light"]->_locs_uniform["light_color"], 1, glm::value_ptr(_light->_color));
	glUniform3fv(_contexts["light"]->_locs_uniform["view_position"], 1, glm::value_ptr(camera_position));

	glEnableVertexAttribArray(_contexts["light"]->_locs_attrib["position_in"]);
	glEnableVertexAttribArray(_contexts["light"]->_locs_attrib["color_in"]);
	glEnableVertexAttribArray(_contexts["light"]->_locs_attrib["normal_in"]);

	glVertexAttribPointer(_contexts["light"]->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, 10* sizeof(float), (void*)0);
	glVertexAttribPointer(_contexts["light"]->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, 10* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_contexts["light"]->_locs_attrib["normal_in"], 3, GL_FLOAT, GL_FALSE, 10* sizeof(float), (void*)(7* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_contexts["light"]->_locs_attrib["position_in"]);
	glDisableVertexAttribArray(_contexts["light"]->_locs_attrib["color_in"]);
	glDisableVertexAttribArray(_contexts["light"]->_locs_attrib["normal_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::draw_normal(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	glActiveTexture(GL_TEXTURE0+ 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_diffuse);
	glActiveTexture(GL_TEXTURE0+ 1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_normal);
	glActiveTexture(0);

	glUseProgram(_contexts["normal"]->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _contexts["normal"]->_buffer);

	glUniformMatrix4fv(_contexts["normal"]->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	glUniform3fv(_contexts["normal"]->_locs_uniform["light_position"], 1, glm::value_ptr(_light->_position));
	glUniform3fv(_contexts["normal"]->_locs_uniform["light_color"], 1, glm::value_ptr(_light->_color));
	glUniform3fv(_contexts["normal"]->_locs_uniform["view_position"], 1, glm::value_ptr(camera_position));
	glUniform1i(_contexts["normal"]->_locs_uniform["diffuse_texture_array"], 0); //Sampler refers to texture unit 0
	glUniform1i(_contexts["normal"]->_locs_uniform["normal_texture_array"], 1); //Sampler refers to texture unit 1

	glEnableVertexAttribArray(_contexts["normal"]->_locs_attrib["position_in"]);
	glEnableVertexAttribArray(_contexts["normal"]->_locs_attrib["tex_coord_in"]);
	glEnableVertexAttribArray(_contexts["normal"]->_locs_attrib["current_layer_diffuse_in"]);
	glEnableVertexAttribArray(_contexts["normal"]->_locs_attrib["current_layer_normal_in"]);
	glEnableVertexAttribArray(_contexts["normal"]->_locs_attrib["normal_in"]);
	glEnableVertexAttribArray(_contexts["normal"]->_locs_attrib["tangent_in"]);

	glVertexAttribPointer(_contexts["normal"]->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, 13* sizeof(float), (void*)0);
	glVertexAttribPointer(_contexts["normal"]->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, 13* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_contexts["normal"]->_locs_attrib["current_layer_diffuse_in"], 1, GL_FLOAT, GL_FALSE, 13* sizeof(float), (void*)(5* sizeof(float)));
	glVertexAttribPointer(_contexts["normal"]->_locs_attrib["current_layer_normal_in"], 1, GL_FLOAT, GL_FALSE, 13* sizeof(float), (void*)(6* sizeof(float)));
	glVertexAttribPointer(_contexts["normal"]->_locs_attrib["normal_in"], 3, GL_FLOAT, GL_FALSE, 13* sizeof(float), (void*)(7* sizeof(float)));
	glVertexAttribPointer(_contexts["normal"]->_locs_attrib["tangent_in"], 3, GL_FLOAT, GL_FALSE, 13* sizeof(float), (void*)(10* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_contexts["normal"]->_locs_attrib["position_in"]);
	glDisableVertexAttribArray(_contexts["normal"]->_locs_attrib["tex_coord_in"]);
	glDisableVertexAttribArray(_contexts["normal"]->_locs_attrib["current_layer_diffuse_in"]);
	glDisableVertexAttribArray(_contexts["normal"]->_locs_attrib["current_layer_normal_in"]);
	glDisableVertexAttribArray(_contexts["normal"]->_locs_attrib["normal_in"]);
	glDisableVertexAttribArray(_contexts["normal"]->_locs_attrib["tangent_in"]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VoroZ::draw_parallax(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	glActiveTexture(GL_TEXTURE0+ 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_diffuse);
	glActiveTexture(GL_TEXTURE0+ 1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_normal);
	glActiveTexture(GL_TEXTURE0+ 2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id_parallax);

	glUseProgram(_contexts["parallax"]->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _contexts["parallax"]->_buffer);

	glUniformMatrix4fv(_contexts["parallax"]->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	glUniform3fv(_contexts["parallax"]->_locs_uniform["light_position"], 1, glm::value_ptr(_light->_position));
	glUniform3fv(_contexts["parallax"]->_locs_uniform["light_color"], 1, glm::value_ptr(_light->_color));
	glUniform3fv(_contexts["parallax"]->_locs_uniform["view_position"], 1, glm::value_ptr(camera_position));
	glUniform1i(_contexts["parallax"]->_locs_uniform["diffuse_texture_array"], 0); //Sampler refers to texture unit 0
	glUniform1i(_contexts["parallax"]->_locs_uniform["normal_texture_array"], 1); //Sampler refers to texture unit 1
	glUniform1i(_contexts["parallax"]->_locs_uniform["parallax_texture_array"], 2); //Sampler refers to texture unit 2
	glUniform1f(_contexts["parallax"]->_locs_uniform["height_scale"], 0.1);

	glEnableVertexAttribArray(_contexts["parallax"]->_locs_attrib["position_in"]);
	glEnableVertexAttribArray(_contexts["parallax"]->_locs_attrib["tex_coord_in"]);
	glEnableVertexAttribArray(_contexts["parallax"]->_locs_attrib["current_layer_diffuse_in"]);
	glEnableVertexAttribArray(_contexts["parallax"]->_locs_attrib["current_layer_normal_in"]);
	glEnableVertexAttribArray(_contexts["parallax"]->_locs_attrib["current_layer_parallax_in"]);
	glEnableVertexAttribArray(_contexts["parallax"]->_locs_attrib["normal_in"]);
	glEnableVertexAttribArray(_contexts["parallax"]->_locs_attrib["tangent_in"]);

	glVertexAttribPointer(_contexts["parallax"]->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, 14* sizeof(float), (void*)0);
	glVertexAttribPointer(_contexts["parallax"]->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, 14* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_contexts["parallax"]->_locs_attrib["current_layer_diffuse_in"], 1, GL_FLOAT, GL_FALSE, 14* sizeof(float), (void*)(5* sizeof(float)));
	glVertexAttribPointer(_contexts["parallax"]->_locs_attrib["current_layer_normal_in"], 1, GL_FLOAT, GL_FALSE, 14* sizeof(float), (void*)(6* sizeof(float)));
	glVertexAttribPointer(_contexts["parallax"]->_locs_attrib["current_layer_parallax_in"], 1, GL_FLOAT, GL_FALSE, 14* sizeof(float), (void*)(7* sizeof(float)));
	glVertexAttribPointer(_contexts["parallax"]->_locs_attrib["normal_in"], 3, GL_FLOAT, GL_FALSE, 14* sizeof(float), (void*)(8* sizeof(float)));
	glVertexAttribPointer(_contexts["parallax"]->_locs_attrib["tangent_in"], 3, GL_FLOAT, GL_FALSE, 14* sizeof(float), (void*)(11* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_pts);

	glDisableVertexAttribArray(_contexts["parallax"]->_locs_attrib["position_in"]);
	glDisableVertexAttribArray(_contexts["parallax"]->_locs_attrib["tex_coord_in"]);
	glDisableVertexAttribArray(_contexts["parallax"]->_locs_attrib["current_layer_diffuse_in"]);
	glDisableVertexAttribArray(_contexts["parallax"]->_locs_attrib["current_layer_normal_in"]);
	glDisableVertexAttribArray(_contexts["parallax"]->_locs_attrib["current_layer_parallax_in"]);
	glDisableVertexAttribArray(_contexts["parallax"]->_locs_attrib["normal_in"]);
	glDisableVertexAttribArray(_contexts["parallax"]->_locs_attrib["tangent_in"]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glActiveTexture(0);
}


void VoroZ::draw(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	if (_draw_mode== SIMPLE) {
		draw_simple(world2clip);
	}
	else if (_draw_mode== TEXTURE) {
		draw_texture(world2clip);
	}
	else if (_draw_mode== LIGHT) {
		draw_light(world2clip, camera_position);
	}
	else if (_draw_mode== NORMAL) {
		draw_normal(world2clip, camera_position);
	}
	else if (_draw_mode== PARALLAX) {
		draw_parallax(world2clip, camera_position);
	}
}


void VoroZ::update_triangle_data() {
	_n_pts= 0;
	_triangle_data.clear();

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
			glm::vec3 pt1(float(vertices[idx_vertex]->_coords.x), float(vertices[idx_vertex]->_coords.y), float(face_data->_z));
			glm::vec3 pt2(float(vertices[idx_vertex2]->_coords.x), float(vertices[idx_vertex2]->_coords.y), float(face_data->_z));
			glm::vec3 pt3(float(g.x), float(g.y), float(face_data->_z));
			glm::vec2 uv1_diffuse= glm::vec2(pt1.x, pt1.y)* face_data->_biome->_uv_factor;
			glm::vec2 uv2_diffuse= glm::vec2(pt2.x, pt2.y)* face_data->_biome->_uv_factor;
			glm::vec2 uv3_diffuse= glm::vec2(pt3.x, pt3.y)* face_data->_biome->_uv_factor;
			glm::vec2 uv1_normal= glm::vec2(pt1.x, pt1.y)* face_data->_biome->_uv_factor;
			glm::vec2 uv2_normal= glm::vec2(pt2.x, pt2.y)* face_data->_biome->_uv_factor;
			glm::vec2 uv3_normal= glm::vec2(pt3.x, pt3.y)* face_data->_biome->_uv_factor;
			glm::vec2 uv1_parallax(0.0, 0.0);
			glm::vec2 uv2_parallax(1.0, 0.0);
			glm::vec2 uv3_parallax(0.0, 1.0);
			_triangle_data.push_back(new TriangleData(
				pt1, pt2, pt3,
				uv1_diffuse, uv2_diffuse, uv3_diffuse,
				uv1_normal, uv2_normal, uv3_normal,
				uv1_parallax, uv2_parallax, uv3_parallax,
				face_data->_biome)
			);
		}
	}

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
			DCEL_FaceData * data_adj= (DCEL_FaceData *)(face_adj->_data);
			number face_adj_z= data_adj->_z;
			if (face_adj_z> face_z) {
				_n_pts+= 6;

				DCEL_HalfEdge * edge= _dcel->get_dividing_edge(face_adj, face);
				pt_type p1= edge->_origin->_coords;
				pt_type p2= edge->destination()->_coords;

				glm::vec3 pt1(float(p1.x), float(p1.y), float(face_z));
				glm::vec3 pt2(float(p2.x), float(p2.y), float(face_z));
				glm::vec3 pt3(float(p2.x), float(p2.y), float(face_adj_z));
				glm::vec3 pt4(float(p1.x), float(p1.y), float(face_adj_z));
				float dxy= _biomes[DIRT]->_uv_factor* sqrt((p1.x- p2.x)* (p1.x- p2.x)+ (p1.y- p2.y)* (p1.y- p2.y));
				float dz= _biomes[DIRT]->_uv_factor* (face_adj_z- face_z);
				glm::vec2 uv1_diffuse(0.0, 0.0);
				glm::vec2 uv2_diffuse(dxy, 0.0);
				glm::vec2 uv3_diffuse(dxy, dz);
				glm::vec2 uv4_diffuse(0.0, dz);
				glm::vec2 uv1_normal(0.0, 0.0);
				glm::vec2 uv2_normal(dxy, 0.0);
				glm::vec2 uv3_normal(dxy, dz);
				glm::vec2 uv4_normal(0.0, dz);
				glm::vec2 uv1_parallax(0.0, 0.0);
				glm::vec2 uv2_parallax(dxy, 0.0);
				glm::vec2 uv3_parallax(dxy, dz);
				glm::vec2 uv4_parallax(0.0, dz);

				_triangle_data.push_back(new TriangleData(
					pt1, pt2, pt3,
					uv1_diffuse, uv2_diffuse, uv3_diffuse,
					uv1_normal, uv2_normal, uv3_normal,
					uv1_parallax, uv2_parallax, uv3_parallax,
					_biomes[DIRT])
				);
				_triangle_data.push_back(new TriangleData(
					pt1, pt3, pt4,
					uv1_diffuse, uv3_diffuse, uv4_diffuse,
					uv1_normal, uv3_normal, uv4_normal,
					uv1_parallax, uv3_parallax, uv4_parallax,
					_biomes[DIRT])
				);
			}
		}
	}

	DCEL_Face * unbounded_face= _dcel->get_unbounded_face();
	std::vector<std::vector<DCEL_HalfEdge *> > inner_edges= unbounded_face->get_inner_edges();
	for (const auto & unbounded_edge : inner_edges[0]) {
		DCEL_HalfEdge * edge= unbounded_edge->_twin;
		DCEL_Face * face= edge->_incident_face;
		DCEL_FaceData * face_data= (DCEL_FaceData *)(face->_data);
		number face_z= face_data->_z;
		if (face_z> 0.0) {
			_n_pts+= 6;

			pt_type p1= edge->_origin->_coords;
			pt_type p2= edge->destination()->_coords;
			number dx= p2.x- p1.x;
			number dy= p2.y- p1.y;

			glm::vec3 pt1(float(p1.x), float(p1.y), 0.0);
			glm::vec3 pt2(float(p2.x), float(p2.y), 0.0);
			glm::vec3 pt3(float(p2.x), float(p2.y), float(face_z));
			glm::vec3 pt4(float(p1.x), float(p1.y), float(face_z));
			float dxy= _biomes[DIRT]->_uv_factor* sqrt((p1.x- p2.x)* (p1.x- p2.x)+ (p1.y- p2.y)* (p1.y- p2.y));
			float dz= _biomes[DIRT]->_uv_factor* face_z;
			glm::vec2 uv1_diffuse(0.0, 0.0);
			glm::vec2 uv2_diffuse(dxy, 0.0);
			glm::vec2 uv3_diffuse(dxy, dz);
			glm::vec2 uv4_diffuse(0.0, dz);
			glm::vec2 uv1_normal(0.0, 0.0);
			glm::vec2 uv2_normal(dxy, 0.0);
			glm::vec2 uv3_normal(dxy, dz);
			glm::vec2 uv4_normal(0.0, dz);
			glm::vec2 uv1_parallax(0.0, 0.0);
			glm::vec2 uv2_parallax(dxy, 0.0);
			glm::vec2 uv3_parallax(dxy, dz);
			glm::vec2 uv4_parallax(0.0, dz);
			_triangle_data.push_back(new TriangleData(
				pt1, pt2, pt3,
				uv1_diffuse, uv2_diffuse, uv3_diffuse,
				uv1_normal, uv2_normal, uv3_normal,
				uv1_parallax, uv2_parallax, uv3_parallax,
				_biomes[DIRT])
			);
			_triangle_data.push_back(new TriangleData(
				pt1, pt3, pt4,
				uv1_diffuse, uv3_diffuse, uv4_diffuse,
				uv1_normal, uv3_normal, uv4_normal,
				uv1_parallax, uv3_parallax, uv4_parallax,
				_biomes[DIRT])
			);
		}
	}
}


void VoroZ::update_simple() {
	unsigned int n_attrs= 7;

	float data[_n_pts* n_attrs];

	for (unsigned int idx_triangle=0; idx_triangle<_triangle_data.size(); ++idx_triangle) {
		for (unsigned int idx_pt=0; idx_pt<3; ++idx_pt) {
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 0]= _triangle_data[idx_triangle]->_pts[idx_pt].x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 1]= _triangle_data[idx_triangle]->_pts[idx_pt].y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 2]= _triangle_data[idx_triangle]->_pts[idx_pt].z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 3]= _triangle_data[idx_triangle]->_biome->_color.x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 4]= _triangle_data[idx_triangle]->_biome->_color.y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 5]= _triangle_data[idx_triangle]->_biome->_color.z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 6]= _triangle_data[idx_triangle]->_biome->_color.w;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _contexts["simple"]->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* n_attrs, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_texture() {
	unsigned int n_attrs= 6;

	float data[_n_pts* n_attrs];

	for (unsigned int idx_triangle=0; idx_triangle<_triangle_data.size(); ++idx_triangle) {
		for (unsigned int idx_pt=0; idx_pt<3; ++idx_pt) {
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 0]= _triangle_data[idx_triangle]->_pts[idx_pt].x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 1]= _triangle_data[idx_triangle]->_pts[idx_pt].y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 2]= _triangle_data[idx_triangle]->_pts[idx_pt].z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 3]= _triangle_data[idx_triangle]->_uvs_diffuse[idx_pt].x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 4]= _triangle_data[idx_triangle]->_uvs_diffuse[idx_pt].y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 5]= _triangle_data[idx_triangle]->_biome->_diffuse_texture_idx_current;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _contexts["texture"]->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* n_attrs, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_light() {
	unsigned int n_attrs= 10;

	float data[_n_pts* n_attrs];

	for (unsigned int idx_triangle=0; idx_triangle<_triangle_data.size(); ++idx_triangle) {
		for (unsigned int idx_pt=0; idx_pt<3; ++idx_pt) {
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 0]= _triangle_data[idx_triangle]->_pts[idx_pt].x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 1]= _triangle_data[idx_triangle]->_pts[idx_pt].y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 2]= _triangle_data[idx_triangle]->_pts[idx_pt].z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 3]= _triangle_data[idx_triangle]->_biome->_color.x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 4]= _triangle_data[idx_triangle]->_biome->_color.y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 5]= _triangle_data[idx_triangle]->_biome->_color.z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 6]= _triangle_data[idx_triangle]->_biome->_color.w;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 7]= _triangle_data[idx_triangle]->_normal.x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 8]= _triangle_data[idx_triangle]->_normal.y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 9]= _triangle_data[idx_triangle]->_normal.z;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _contexts["light"]->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* n_attrs, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_normal() {
	unsigned int n_attrs= 13;

	float data[_n_pts* n_attrs];

	for (unsigned int idx_triangle=0; idx_triangle<_triangle_data.size(); ++idx_triangle) {
		for (unsigned int idx_pt=0; idx_pt<3; ++idx_pt) {
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 0]= _triangle_data[idx_triangle]->_pts[idx_pt].x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 1]= _triangle_data[idx_triangle]->_pts[idx_pt].y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 2]= _triangle_data[idx_triangle]->_pts[idx_pt].z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 3]= _triangle_data[idx_triangle]->_uvs_normal[idx_pt].x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 4]= _triangle_data[idx_triangle]->_uvs_normal[idx_pt].y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 5]= _triangle_data[idx_triangle]->_biome->_diffuse_texture_idx_current;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 6]= _triangle_data[idx_triangle]->_biome->_normal_texture_idx_current;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 7]= _triangle_data[idx_triangle]->_normal.x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 8]= _triangle_data[idx_triangle]->_normal.y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 9]= _triangle_data[idx_triangle]->_normal.z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 10]= _triangle_data[idx_triangle]->_tangent.x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 11]= _triangle_data[idx_triangle]->_tangent.y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 12]= _triangle_data[idx_triangle]->_tangent.z;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _contexts["normal"]->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* n_attrs, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update_parallax() {
	unsigned int n_attrs= 14;

	float data[_n_pts* n_attrs];

	for (unsigned int idx_triangle=0; idx_triangle<_triangle_data.size(); ++idx_triangle) {
		for (unsigned int idx_pt=0; idx_pt<3; ++idx_pt) {
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 0]= _triangle_data[idx_triangle]->_pts[idx_pt].x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 1]= _triangle_data[idx_triangle]->_pts[idx_pt].y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 2]= _triangle_data[idx_triangle]->_pts[idx_pt].z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 3]= _triangle_data[idx_triangle]->_uvs_parallax[idx_pt].x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 4]= _triangle_data[idx_triangle]->_uvs_parallax[idx_pt].y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 5]= _triangle_data[idx_triangle]->_biome->_diffuse_texture_idx_current;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 6]= _triangle_data[idx_triangle]->_biome->_normal_texture_idx_current;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 7]= _triangle_data[idx_triangle]->_biome->_parallax_texture_idx;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 8]= _triangle_data[idx_triangle]->_normal.x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 9]= _triangle_data[idx_triangle]->_normal.y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 10]= _triangle_data[idx_triangle]->_normal.z;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 11]= _triangle_data[idx_triangle]->_tangent.x;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 12]= _triangle_data[idx_triangle]->_tangent.y;
			data[idx_triangle* n_attrs* 3+ idx_pt* n_attrs+ 13]= _triangle_data[idx_triangle]->_tangent.z;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _contexts["parallax"]->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts* n_attrs, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void VoroZ::update() {
	update_triangle_data();
	update_simple();
	update_texture();
	update_light();
	update_normal();
	update_parallax();
}


void VoroZ::anim() {
	/*for (auto face : _dcel->_faces) {
		if (face->_outer_edge== NULL) {
			continue;
		}
		DCEL_FaceData * data= (DCEL_FaceData *)(face->_data);
		data->_z+= rand_number(0.1, 0.5);
		if (data->_z> 20.0) {
			data->_z= 0.0;
		}
	}

	update();*/
	_light->anim();

	for (auto biome : _biomes) {
		biome.second->_diffuse_texture_idx_current+= biome.second->_anim_speed;
		if (biome.second->_diffuse_texture_idx_current> float(biome.second->_diffuse_texture_idx_end)) {
			biome.second->_diffuse_texture_idx_current= float(biome.second->_diffuse_texture_idx_start);
		}
		biome.second->_normal_texture_idx_current+= biome.second->_anim_speed;
		if (biome.second->_normal_texture_idx_current> float(biome.second->_normal_texture_idx_end)) {
			biome.second->_normal_texture_idx_current= float(biome.second->_normal_texture_idx_start);
		}
	}
	update_normal();
}


bool VoroZ::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_a) {
		init_dcel();
		update();
		return true;
	}

	if (key== SDLK_b) {
		_light->_animated= !_light->_animated;
		return true;
	}

	if (key== SDLK_o) {
		if (_draw_mode== SIMPLE) {
			_draw_mode= PARALLAX;
		}
		else if (_draw_mode== TEXTURE) {
			_draw_mode= SIMPLE;
		}
		else if (_draw_mode== LIGHT) {
			_draw_mode= TEXTURE;
		}
		else if (_draw_mode== NORMAL) {
			_draw_mode= LIGHT;
		}
		else if (_draw_mode== PARALLAX) {
			_draw_mode= NORMAL;
		}
		return true;
	}

	if (key== SDLK_p) {
		if (_draw_mode== SIMPLE) {
			_draw_mode= TEXTURE;
		}
		else if (_draw_mode== TEXTURE) {
			_draw_mode= LIGHT;
		}
		else if (_draw_mode== LIGHT) {
			_draw_mode= NORMAL;
		}
		else if (_draw_mode== NORMAL) {
			_draw_mode= PARALLAX;
		}
		else if (_draw_mode== PARALLAX) {
			_draw_mode= SIMPLE;
		}
		return true;
	}

	return false;
}


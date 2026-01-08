#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include "particle.h"


// Particle -----------------------------------------------------------------------------
Particle::Particle() {

}


Particle::Particle(glm::vec2 position, glm::vec2 size, glm::vec2 velocity, glm::vec4 color) : 
	_velocity(velocity), _life(1.0), _color(color) {
	_aabb= AABB_2D(position, size);
}


Particle::~Particle() {

}


// ParticleSystem ------------------------------------------------------------------------
ParticleSystem::ParticleSystem() {

}


ParticleSystem::ParticleSystem(GLuint prog, ScreenGL * screengl) {
	const float Z_NEAR= -1.0f;
	const float Z_FAR= 1.0f;
	_camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	
	GLuint buffer;
	glGenBuffers(1, &buffer);
	_context= new DrawContext(prog, buffer,
		std::vector<std::string>{"vertex_in", "color_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix"});

	fill_texture_array();
}


ParticleSystem::~ParticleSystem() {
	for (auto particle : _particles) {
		delete particle;
	}
	_particles.clear();
}


void ParticleSystem::fill_texture_array() {
	uint n_tex= 0;
	for (auto model : _models) {
		for (auto texture : model.second->_textures) {
			n_tex+= texture.second->_pngs.size();
		}
	}

	glGenTextures(1, &_texture_id);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, n_tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	uint compt= 0;
	for (auto model : _models) {
		//std::cout << "\nmodel=" << model.first << "\n";
		for (auto texture : model.second->_textures) {
			//std::cout << "texture=" << texture.first << "\n";
			texture.second->_first_idx= compt;
			for (auto png : texture.second->_pngs) {
				std::string png_abs= splitext(model.second->_json_path).first+ "/"+ png;
				//std::cout << "png=" << png_abs << " ; compt=" << compt << "\n";
				SDL_Surface * surface= IMG_Load(png_abs.c_str());
				if (!surface) {
					std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
					return;
				}

				// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
								0,                             // mipmap number
								0, 0, compt,   // xoffset, yoffset, zoffset
								TEXTURE_SIZE, TEXTURE_SIZE, 1, // width, height, depth
								GL_BGRA,                       // format
								GL_UNSIGNED_BYTE,              // type
								surface->pixels);              // pointer to data

				SDL_FreeSurface(surface);

				compt++;
			}
		}
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);

	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}


void ParticleSystem::draw() {

}


void ParticleSystem::update() {

}


void ParticleSystem::anim() {
	
}

#ifndef OBJFILE_H
#define OBJFILE_H

#include <string>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <sys/time.h>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utile.h"


// evolutions : specular_color, emissive_color -> chercher un fragment shader qui les gèrent
// textures...


class ModelObj {
public:
	ModelObj();
	ModelObj(GLuint prog_draw_);
    void load(std::string ch_obj, std::string ch_mat);
	void release();
	void draw();
	void print(bool verbose);
	void anim(float * world2camera, float * camera2clip);
	
	
	unsigned int n_faces;
	float * vertices;
	unsigned int * faces;
	GLuint buffers[2];
	
	float model2camera[16];
	float model2clip[16];
	float normal[9];
	
	float model2world[16];
	float ambient[3]; // couleur des reflets ambiants, géré par objet
	float diffuse[3]; // couleur des reflets diffus, géré par sommet (à faire évoluer), inutilisé pour l'instant (fichier .mat remplace)
	float shininess; // eclat (entre 0. et 128., 0. étant le + éclatant), géré par objet
	float alpha;

	GLint model2clip_loc, model2camera_loc, normal_mat_loc;
	GLint position_loc, normal_loc;
	GLint ambient_color_loc, shininess_loc, diffuse_color_loc;
	GLint alpha_loc;
	GLuint prog_draw;

	bool is_active;
};


#endif

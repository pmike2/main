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
#include "config.h"


// evolutions : specular_color, emissive_color -> chercher un fragment shader qui les gèrent
// textures...


class ModelObj {
public:
	ModelObj();
	ModelObj(GLuint prog_draw_, struct timeval t_);
    void load(std::string ch_obj, std::string ch_mat);
	void release();
	void draw();
	void print(bool verbose);
	void anim(float * world2camera, float * camera2clip);
	void set_config(ModelObjConfig * config_);
	
	
	unsigned int n_faces;
	float * vertices;
	unsigned int * faces;
	GLuint buffers[2];
	
	float model2camera[16];
	float model2clip[16];
	float normal[9];
	
	GLint model2clip_loc, model2camera_loc, normal_mat_loc;
	GLint position_loc, normal_loc;
	GLint ambient_color_loc, shininess_loc, diffuse_color_loc;
	GLint alpha_loc;

	GLuint prog_draw;

	float xmin, xmax, ymin, ymax, zmin, zmax;
	
	ModelObjMorph current_values;
	ModelObjConfig * config;
	bool is_active;
	struct timeval t;
	float alpha;
};


#endif

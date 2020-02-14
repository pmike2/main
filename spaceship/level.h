#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "utile.h"
#include "constantes.h"
#include "objfile.h"
#include "ship.h"
#include "terrain.h"



// skybox ; désactivée pour l'instant
// https://learnopengl.com/Advanced-OpenGL/Cubemaps
// http://www.custommapmakers.org/skyboxes.php
class SkyBox {
public:
	SkyBox();
	SkyBox(GLuint prog);
	void draw();
	void anim(float * world2camera, float * camera2clip);
	

	float _world2camera[16];
	float _camera2clip[16];

	GLuint _buffer;
	GLuint _prog;
	GLint _position_loc, _world2camera_loc, _camera2clip_loc;
	unsigned int _cubemap_texture;
};


// TODO : faire de l'instanced comme pour Explosion
/*class Cloud {
public:
	Cloud();
	Cloud(GLuint prog_draw, GLuint prog_draw_basic, std::string model_path, std::string material_path, glm::vec3 position, glm::mat3 rotation_matrix, float size_factor, float speed);
	~Cloud();
	void draw();
	void anim(float * world2camera, float * camera2clip);
	
	StaticInstance _model;
	glm::vec3 _position; // position
	glm::mat3 _rotation_matrix; // matrice de rotation
	float _speed; // vitesse de déplacement du nuage
};
*/

// cf https://stackoverflow.com/questions/396084/headers-including-each-other-in-c
class Ship;

// carte
class LevelMap {
public:
	LevelMap();
	LevelMap(GLuint prog_draw, unsigned int n_ships);
	~LevelMap();
	void draw();
	void anim(std::vector<Ship *> & ships);
	
	
	unsigned int _n_ships;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffer;
	float * _data;
	float _camera2clip[16];
};



#endif

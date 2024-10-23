#ifndef VORO_Z_H
#define VORO_Z_H

#include <iostream>

#include <OpenGL/gl3.h>

#include "typedefs.h"
#include "input_state.h"
#include "dcel.h"



class DCEL_FaceData {
public:
	DCEL_FaceData();
	DCEL_FaceData(number z);
	~DCEL_FaceData();


	number _z;
	glm::vec4 _color;
};


class VoroZ {
public:
	VoroZ();
	VoroZ(GLuint prog_draw);
	~VoroZ();
	void draw(const glm::mat4 & world2clip);
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);


	GLuint _prog_draw;
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[4];
	unsigned int _n_pts;
	DCEL * _dcel;
};


#endif

#ifndef RACING_H
#define RACING_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <OpenGL/gl3.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "geom_2d.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "typedefs.h"

#include "static_object.h"
#include "car.h"
#include "track.h"


enum cam_mode {FIXED, TRANSLATE, TRANSLATE_AND_ROTATE};


// plans z de contrainte d'affichage de glm::ortho
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;

const number ANIM_DT= 0.05;

const number CAM_INC= 0.05;
const number CAM_INC_ALPHA= 0.05;

const float CROSS_SIZE= 0.1;
const float ARROW_ANGLE= M_PI* 0.1;
const float ARROW_TIP_SIZE= 0.2;
const float INFO_ALPHA= 0.8;
const glm::vec4 COM_CROSS_COLOR(1.0, 0.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_FWD_CROSS_COLOR(1.0, 1.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_BWD_CROSS_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);
const glm::vec4 BBOX_CROSS_COLOR(0.0, 0.5, 1.0, INFO_ALPHA);
const glm::vec4 FORCE_FWD_ARROW_COLOR(1.0, 1.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_BWD_ARROW_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);
const glm::vec4 ACCELERATION_ARROW_COLOR(1.0, 0.0, 1.0, INFO_ALPHA);
const glm::vec4 VELOCITY_ARROW_COLOR(0.0, 1.0, 1.0, INFO_ALPHA);
const glm::vec4 FORWARD_ARROW_COLOR(1.0, 0.5, 1.0, INFO_ALPHA);
const glm::vec4 RIGHT_ARROW_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);


class Racing {
public:
	Racing();
	Racing(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl, bool is_joystick);
	~Racing();

	void load_track(std::string json_path);
	
	// dessins
	void draw_bbox();
	void draw_force();
	void draw();

	void show_info();

	// maj des buffers
	void update_bbox();
	void update_force();

	// animation
	void anim();
	void camera();

	// input
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	bool joystick_down(unsigned int button_idx);
	bool joystick_up(unsigned int button_idx);
	bool joystick_axis(unsigned int axis_idx, int value);


	Track * _track;

	pt_type _pt_min, _pt_max;
	pt_type _com_camera;
	number _alpha_camera;
	cam_mode _cam_mode;

	bool _draw_bbox, _draw_force, _show_info; // faut-il afficher les BBox
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	glm::mat4 _camera2clip; // glm::ortho
	glm::mat4 _world2camera;
	Font * _font; // font pour écriture textes

	bool _key_left, _key_right, _key_up, _key_down, _key_a, _key_z; // les touches sont-elle enfoncées
	bool _is_joystick;
	glm::vec2 _joystick; // valeurs x, y stick joystick
	bool _joystick_a, _joystick_b; // boutons 

	bool _ia;
};


#endif

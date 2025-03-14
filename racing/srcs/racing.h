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


// type de caméra : fixe, suit le héros, suit et s'oriente comme le héros
enum cam_mode {FIXED, TRANSLATE, TRANSLATE_AND_ROTATE};


// plans z de contrainte d'affichage de glm::ortho
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;

// à quelle vitesse la caméra s'ajuste-t'elle au mouvement du héros
const number CAM_INC= 0.1;
const number CAM_INC_ALPHA= 0.01;

// params de debug ds forces
const float CROSS_SIZE= 0.1;
const float ARROW_ANGLE= M_PI* 0.1;
const float ARROW_TIP_SIZE= 0.2;
const float INFO_ALPHA= 0.8;

// couleurs de debug
const glm::vec4 COM_CROSS_COLOR(1.0, 0.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_FWD_CROSS_COLOR(1.0, 1.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_BWD_CROSS_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);
const glm::vec4 FORCE_FWD_ARROW_COLOR(1.0, 1.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_BWD_ARROW_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);
const glm::vec4 ACCELERATION_ARROW_COLOR(1.0, 0.0, 1.0, INFO_ALPHA);
const glm::vec4 VELOCITY_ARROW_COLOR(0.0, 1.0, 1.0, INFO_ALPHA);
const glm::vec4 FORWARD_ARROW_COLOR(1.0, 0.5, 1.0, INFO_ALPHA);
const glm::vec4 RIGHT_ARROW_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);
const glm::vec4 BBOX_COLOR(1.0, 0.0, 0.0, 0.5);
const glm::vec4 FOOTPRINT_COLOR(0.0, 1.0, 0.0, 0.5);
const glm::vec4 NLAPS_COLOR(0.0, 1.0, 0.0, 1.0);
const glm::vec4 NLAPS_LAST_COLOR(1.0, 0.0, 0.0, 1.0);
const glm::vec4 HERO_COLOR(1.0, 1.0, 0.0, 1.0);
const glm::vec4 ENNEMY_COLOR(0.5, 0.5, 0.6, 1.0);

/*
	Classe principale de course de voiture 2D
*/
class Racing {
public:
	Racing();
	Racing(GLuint prog_simple, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, InputState * input_state);
	~Racing();

	// chargement des textures
	void fill_texture_array();
	
	// dessins
	void draw_bbox();
	void draw_footprint();
	void draw_force();
	void draw_texture();
	void draw();

	void show_info();
	void show_debug_info();

	// maj des buffers
	void update_bbox();
	void update_footprint();
	void update_force();
	void update_texture();

	// animation
	void anim(std::chrono::system_clock::time_point t);
	void camera();

	// input
	bool key_down(SDL_Keycode key);


	Track * _track;

	// params caméra
	pt_type _com_camera;
	number _alpha_camera;
	cam_mode _cam_mode;

	bool _draw_bbox, _draw_force, _draw_texture, _show_debug_info, _show_info; // faut-il afficher les BBox
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	glm::mat4 _camera2clip; // glm::ortho
	glm::mat4 _world2camera; // caméra
	Font * _font; // font pour écriture textes
	ScreenGL * _screengl;
	InputState * _input_state;

	// A REVOIR ; association modèle <-> texture
	std::map<std::string, unsigned int> _model_tex_idxs;
};


#endif

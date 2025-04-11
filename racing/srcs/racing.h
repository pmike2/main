#ifndef RACING_H
#define RACING_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <chrono>

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
#include "smoke.h"
#include "tire_track.h"
#include "spark.h"
#include "driver.h"


// type de caméra : fixe, suit le héros, suit et s'oriente comme le héros
enum cam_mode {FIXED, TRANSLATE, TRANSLATE_AND_ROTATE};
// mode : choix du joueur, choix d'une piste, course en cours
enum racing_mode {CHOOSE_DRIVER, CHOOSE_TRACK, RACING};


// plans z de contrainte d'affichage de glm::ortho
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;
// z des types de dessin
const float Z_BBOX= -30.0f;
const float Z_FOOTPRINT= -20.0f;
const float Z_FORCE= -10.0f;
const float Z_TIRE_TRACK= -50.0f;
const float Z_SPARK= -35.0f;
const float Z_DRIVER_FACE= -5.0f;
const float Z_BARRIER= -60.0f;
const float Z_MAP= -5.0f;
// voir également static_object.h / Z_OBJECTS

// à quelle vitesse la caméra s'ajuste-t'elle au mouvement du héros
const number CAM_INC= 0.1;
const number CAM_INC_ALPHA= 0.01;

// params de debug des forces
const float CROSS_SIZE= 0.1;
const float ARROW_ANGLE= M_PI* 0.1;
const float ARROW_TIP_SIZE= 0.2;
const float INFO_ALPHA= 0.8;

// mélange de gris lors des phases inactives
const float GRAY_BLEND= 0.6f;

// faces
const glm::vec2 RANKING_ORIGIN(-7.5f, 0.0f);
const std::vector<float> RANKING_FACE_SIZE {0.7f, 0.6f, 0.5f, 0.4f};
const std::vector<float> RANKING_FACE_ALPHA {1.0f, 1.0f, 1.0f, 1.0f};
const std::vector<float> RANKING_TEXT_SCALE {0.007f, 0.006f, 0.005f, 0.004f};
const number RANKING_FACE_MARGIN_FACTOR= 1.3f;
const number RANKING_FACE_TEXT_MARGIN= 0.1f;
const glm::vec2 IN_RACE_FACE_OFFSET(0.1f, 0.1f);
const glm::vec2 IN_RACE_TEXT_OFFSET(0.1f, 0.1f);
const std::vector<float> IN_RACE_FACE_SIZE {1.0f, 0.8f, 0.7f, 0.6f};
const std::vector<float> IN_RACE_FACE_ALPHA {1.0f, 1.0f, 1.0f, 1.0f};
const std::vector<float> IN_RACE_TEXT_SCALE {0.006f, 0.005f, 0.004f, 0.003f};

// map
const glm::vec2 MAP_ORIGIN(-7.4f, 1.9f);
const glm::vec2 MAP_SIZE(3.5f, 3.5f);
const std::vector<float> MAP_FACE_SIZE {0.7f, 0.6f, 0.5f, 0.4f};
const float MAP_OPACITY= 0.8;
const float MAP_FACE_OPACITY= 0.9;

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

// couleurs d'info
const glm::vec4 NLAPS_COLOR(0.0, 1.0, 0.5, 1.0);
const glm::vec4 NLAPS_LAST_COLOR(1.0, 0.0, 0.5, 1.0);
const glm::vec4 RANKING_HERO_COLOR(1.0, 1.0, 0.0, 1.0);
const glm::vec4 RANKING_ENNEMY_COLOR(0.9, 0.8, 0.9, 1.0);
const glm::vec4 LOW_SPEED_COLOR(0.3, 1.0, 0.5, 1.0);
const glm::vec4 HIGH_SPEED_COLOR(1.0, 0.3, 0.5, 1.0);
const glm::vec4 PAST_LAP_TIME_COLOR(0.6, 0.6, 0.6, 1.0);
const glm::vec4 CURRENT_LAP_TIME_COLOR(0.8, 0.8, 0.6, 1.0);
const glm::vec4 TOTAL_LAP_TIME_COLOR(0.8, 0.8, 0.6, 1.0);
const glm::vec4 NOT_BEST_LAP_TIME_COLOR(0.8, 0.4, 0.4, 1.0);
const glm::vec4 BEST_LAP_TIME_COLOR(0.4, 0.8, 0.4, 1.0);

/*
	Classe principale de course de voiture 2D
*/
class Racing {
public:
	Racing();
	Racing(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t);
	~Racing();

	//void choose_driver(unsigned int idx_driver);
	// choix d'une piste
	void choose_track(unsigned int idx_track, time_point t);

	// chargement des textures
	void fill_texture_driver();
	void fill_texture_choose_track();
	void fill_texture_array_models();
	void fill_texture_array_smoke();
	void fill_texture_array_tire_track();
	void fill_texture_map();
	
	// dessins
	void draw_choose_driver();
	void draw_choose_track();
	void draw_bbox();
	void draw_footprint();
	void draw_force();
	void draw_texture();
	void draw_smoke();
	void draw_tire_track();
	void draw_spark();
	void draw_driver_face();
	void draw_barrier();
	void draw_map();
	void show_info();
	void draw();

	// maj des buffers
	void update_choose_driver();
	void update_choose_track();
	void update_bbox();
	void update_footprint();
	void update_force();
	void update_texture();
	void update_smoke();
	void update_tire_track();
	void update_spark();
	void update_driver_face();
	void update_barrier();
	void update_map();
	
	// animation
	void anim(time_point t);
	void camera();

	// input
	bool key_down(SDL_Keycode key, time_point t);


	Track * _track; // piste courante
	TrackInfo * _track_info;
	std::vector<SmokeSystem *> _smoke_systems; // système de fumée : 1 par Car
	TireTrackSystem * _tire_track_system; // traces de pneu
	SparkSystem * _spark_system; // étincelles des collisions

	// params caméra
	pt_type _com_camera;
	number _alpha_camera;
	cam_mode _cam_mode;

	racing_mode _mode; // mode
	int _idx_chosen_driver;
	int _idx_chosen_track; // indice piste choisie
	unsigned int _n_available_tracks; // nombre total de pistes
	number _track_lap_record, _track_overall_record;

	bool _draw_bbox, _draw_force, _draw_texture, _show_debug_info; // booléens d'affichage
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	// indices des textures
	unsigned int _texture_idx_model, _texture_idx_bump, _texture_idx_smoke, _texture_idx_choose_track,
		_texture_idx_tire_track, _texture_idx_driver_face, _texture_idx_map;
	glm::mat4 _camera2clip; // glm::ortho
	glm::mat4 _world2camera; // caméra
	Font * _font; // font pour écriture textes
	ScreenGL * _screengl;
	InputState * _input_state;
	bool _joystick_is_input;
};


#endif

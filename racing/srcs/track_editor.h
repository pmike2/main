#ifndef TRACK_EDITOR
#define TRACK_EDITOR

#include <map>

#include <OpenGL/gl3.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "geom_2d.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "typedefs.h"

#include "track.h"


// plans z de contrainte d'affichage de glm::ortho
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;

const glm::vec4 GRID_COLOR(0.0, 1.0, 0.0, 0.7);
const glm::vec4 OBSTACLE_SETTING_COLOR(1.0, 0.0, 0.0, 0.5);
const glm::vec4 OBSTACLE_FLOATING_FOOTPRINT_COLOR(1.0, 0.0, 1.0, 0.5);
const glm::vec4 OBSTACLE_FLOATING_BBOX_COLOR(0.5, 0.2, 1.0, 0.5);
const glm::vec4 SELECTION_COLOR(1.0, 1.0, 0.0, 0.3);

const pt_type TRACK_ORIGIN(-9.0, -7.5);
const pt_type TILES_ORIGIN(3.5, -7.5);
const pt_type FLOATING_OBJECTS_ORIGIN(-7.0, 3.0);


class GridEditor {
public:
	GridEditor();
	GridEditor(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl, glm::vec4 tile_color, number cell_size, GridType type);
	~GridEditor();
	void draw_grid();
	void draw_selection();
	void draw_tiles();
	void draw();
	void show_info();
	void update_grid();
	void update_selection();
	void update_tiles();
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	pt_type screen2pt(int x, int y);
	bool mouse_button_down(InputState * input_state);


	StaticObjectGrid * _grid;
	int _row_idx_select, _col_idx_select;
	glm::vec4 _tile_color;
	pt_type _translation;
	number _scale;

	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	glm::mat4 _camera2clip, _world2camera; // glm::ortho
	ScreenGL * _screengl;
	Font * _font;
};


class TrackEditor {
public:
	TrackEditor();
	TrackEditor(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl, number cell_size);
	~TrackEditor();
	void reinit();
	void load_json(std::string json_path);
	void save_json(std::string json_path);
	void draw_grid();
	void draw_selection();
	void draw_tiles();
	void draw_floating_objects_footprint();
	void draw_floating_objects_bbox();
	void draw();
	void show_info();
	void update_grid();
	void update_selection();
	void update_tiles();
	void update_floating_objects_footprint();
	void update_floating_objects_bbox();
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	pt_type screen2pt(int x, int y);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool mouse_motion(InputState * input_state);
	bool mouse_wheel(InputState * input_state);


	Track * _track;
	int _row_idx_select, _col_idx_select;
	StaticObject * _selected_floating_object;
	pt_type _translation;
	number _scale;
	CheckPoint * _last_checkpoint;

	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	glm::mat4 _camera2clip, _world2camera; // glm::ortho
	ScreenGL * _screengl;
	Font * _font;
};


class Editor {
public:
	Editor();
	Editor(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl);
	~Editor();
	void draw();
	void show_info();
	void sync_track_with_tile();
	void add_floating_object(pt_type pos);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool mouse_motion(InputState * input_state);
	bool mouse_wheel(InputState * input_state);


	TrackEditor * _track_editor;
	GridEditor * _tile_grid_editor;
	GridEditor * _floating_grid_editor;
	//Font * _font; // font pour écriture textes
	ScreenGL * _screengl;
};

#endif


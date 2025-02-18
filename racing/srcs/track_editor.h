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
const glm::vec4 OBSTACLE_COLOR(1.0, 0.0, 0.0, 0.5);
const glm::vec4 SELECTION_COLOR(1.0, 1.0, 0.0, 0.3);
const pt_type GRID_OFFSET(-9.0, -7.5);
const pt_type POOL_OFFSET(3.5, -7.5);


class TilesPool {
public:
	TilesPool();
	~TilesPool();
	void set_tiles(std::map<std::string, TrackTile * > tiles);
	unsigned int coord2idx(unsigned int col_idx, unsigned int row_idx);
	std::pair<unsigned int, unsigned int> idx2coord(unsigned int idx);
	TrackTile * get_tile_by_idx(unsigned int col_idx, unsigned int row_idx);


	std::map<std::string, TrackTile * > _tiles;
	std::vector<std::string> _tile_names;
	unsigned int _width;
	unsigned int _height;
	number _cell_size;
};


class TrackEditor {
public:
	TrackEditor();
	TrackEditor(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl);
	~TrackEditor();
	void reinit();
	void load_json(std::string json_path);
	void save_json(std::string json_path);
	void draw_grid();
	void draw_selection();
	void draw_obstacle();
	void draw_pool();
	void draw();
	void update_grid();
	void update_selection();
	void update_obstacle();
	void update_pool();
	void randomize();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	bool mouse_button_down(InputState * input_state);


	Track * _track;
	TilesPool * _pool;
	int _row_idx_select, _col_idx_select;
	int _row_idx_pool, _col_idx_pool;

	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	glm::mat4 _camera2clip;//, _world2camera; // glm::ortho
	Font * _font; // font pour écriture textes
	ScreenGL * _screengl;
};

#endif


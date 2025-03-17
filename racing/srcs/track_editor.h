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

// couleurs
const glm::vec4 GRID_COLOR(0.0, 1.0, 0.0, 0.7);
const glm::vec4 OBSTACLE_SETTING_COLOR(1.0, 0.0, 0.0, 0.5);
const glm::vec4 OBSTACLE_FLOATING_FOOTPRINT_COLOR(1.0, 0.0, 1.0, 0.5);
const glm::vec4 OBSTACLE_FLOATING_BBOX_COLOR(0.5, 0.2, 1.0, 0.5);
const glm::vec4 SELECTION_COLOR(1.0, 1.0, 0.0, 0.3);

// positions des grilles
const pt_type TRACK_ORIGIN(-9.0, -7.5);
const pt_type TILES_ORIGIN(1.5, -7.5);
const pt_type FLOATING_OBJECTS_ORIGIN(-7.0, 3.0);
// scale par défaut de la grille track
const number DEFAULT_TRACK_EDITOR_SCALE= 0.3;
// dimensions par défaut des grilles de tile et d'objets flottants
const unsigned int TILE_GRID_WIDTH= 8;
const unsigned int FLOATING_GRID_HEIGHT= 4;
// dimensions défaut en tuiles d'une piste
const unsigned int TRACK_DEFAULT_SIZE= 16;
// nombre de tours par défaut
const unsigned int DEFAULT_N_LAPS= 3;


// Editeur de grille d'objets
class GridEditor {
public:
	GridEditor();
	GridEditor(GLuint prog_simple, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, glm::vec4 tile_color, number cell_size, GridType type);
	~GridEditor();
	void draw_grid();
	void draw_selection();
	void draw_tiles();
	void draw_texture(GLuint texture);
	void draw(GLuint texture);
	void show_info();
	void update_grid();
	void update_selection();
	void update_tiles();
	void update_texture();
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	pt_type screen2pt(int x, int y);
	bool mouse_button_down(InputState * input_state);


	bool _draw_bbox, _draw_texture;
	StaticObjectGrid * _grid; // grille à éditer
	int _row_idx_select, _col_idx_select; // tuile courante sélectionnée
	glm::vec4 _tile_color;
	pt_type _translation; // position et agrandissement
	number _scale;

	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	glm::mat4 _camera2clip, _world2camera; // glm::ortho
	ScreenGL * _screengl;
	Font * _font;

	//std::map<std::string, unsigned int> _model_tex_idxs;
};


// Editeur de piste
class TrackEditor {
public:
	TrackEditor();
	TrackEditor(GLuint prog_simple, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, number cell_size);
	~TrackEditor();
	void reinit();
	void load_json(std::string json_path);
	void save_json(std::string json_path);
	void draw_grid();
	void draw_selection();
	void draw_tiles();
	void draw_floating_objects_footprint();
	void draw_floating_objects_bbox();
	void draw_texture(GLuint texture);
	void draw(GLuint texture);
	void show_info();
	void update_grid();
	void update_selection();
	void update_tiles();
	void update_floating_objects_footprint();
	void update_floating_objects_bbox();
	void update_texture();
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	pt_type screen2pt(int x, int y);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool mouse_motion(InputState * input_state);
	bool mouse_wheel(InputState * input_state);


	bool _draw_bbox, _draw_texture;
	Track * _track; // piste à éditer
	int _row_idx_select, _col_idx_select; // tuile courante sélectionnée
	StaticObject * _selected_floating_object; // objet flottant courant sélectionné
	pt_type _translation; // position et agrandissement
	number _scale;
	CheckPoint * _last_checkpoint; // dernier chkpt mis

	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	glm::mat4 _camera2clip, _world2camera; // glm::ortho
	ScreenGL * _screengl;
	Font * _font;	

	//std::map<std::string, unsigned int> _model_tex_idxs;
};


// Editeur, classe principale
class Editor {
public:
	Editor();
	Editor(GLuint prog_simple, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl);
	~Editor();
	void fill_texture_array();
	void draw();
	void sync_track_with_tile();
	void add_floating_object(pt_type pos);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool mouse_motion(InputState * input_state);
	bool mouse_wheel(InputState * input_state);


	TrackEditor * _track_editor; // éditeur de track
	GridEditor * _tile_grid_editor; // éditeur des tuiles disponibles
	GridEditor * _floating_grid_editor; // éditeur des objets flottants disponibles
	//Font * _font; // font pour écriture textes
	ScreenGL * _screengl;
	GLuint * _textures; // texture arrays pour tous les PNGs
};

#endif


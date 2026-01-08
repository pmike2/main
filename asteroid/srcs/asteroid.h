#ifndef ASTEROID_H
#define ASTEROID_H

#include <vector>
#include <map>
#include <chrono>

#include <OpenGL/gl3.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"

#include "ship_model.h"
#include "ship.h"
#include "level.h"
#include "star.h"
#include "constantes.h"


// Classe principale du jeu
class Asteroid {
public:
	Asteroid();
	Asteroid(GLuint prog_aabb, GLuint prog_texture, GLuint prog_star, GLuint prog_font, ScreenGL * screengl, bool is_joystick, std::chrono::system_clock::time_point t);
	~Asteroid();

	// chargements
	void load_models();
	void load_levels(std::chrono::system_clock::time_point t);
	void fill_texture_array_ship();
	void fill_texture_array_star();
	
	// dessins
	void draw_border_aabb();
	void draw_ship_aabb();
	void draw_ship_footprint();
	void draw_ship_texture();
	void draw_star();
	void draw();

	// affichages textes
	void show_playing_info();
	void show_inactive_info();
	void show_set_score_name_info();

	// maj des buffers
	void update_border_aabb();
	void update_ship_aabb();
	void update_ship_footprint();
	void update_ship_texture();
	void update_star();
	
	// animation
	void anim_playing(std::chrono::system_clock::time_point t);
	void anim_inactive();
	void anim_set_score_name();
	void anim(std::chrono::system_clock::time_point t);

	// input
	bool key_down(InputState * input_state, SDL_Keycode key, std::chrono::system_clock::time_point t);
	bool key_up(InputState * input_state, SDL_Keycode key, std::chrono::system_clock::time_point t);
	bool joystick_down(uint button_idx, std::chrono::system_clock::time_point t);
	bool joystick_up(uint button_idx, std::chrono::system_clock::time_point t);
	bool joystick_axis(uint axis_idx, int value, std::chrono::system_clock::time_point t);
	
	// événements
	void add_level_events(std::chrono::system_clock::time_point t);
	void set_level(uint level_idx, std::chrono::system_clock::time_point t);
	void add_rand_enemy(std::chrono::system_clock::time_point t);
	void reinit(std::chrono::system_clock::time_point t);
	void gameover();
	
	// scores
	void read_highest_scores();
	void write_highest_scores();

	// music; les static sont nécessaires à cause du callback géré par Mix_HookMusicFinished
	static void set_music(std::string music_path, uint music_fade_in_ms=MUSIC_FADE_IN_MS);
	void set_music_with_fadeout(std::string music_path, uint music_fade_out_ms=MUSIC_FADE_OUT_MS, uint music_fade_in_ms=MUSIC_FADE_IN_MS);
	static void music_finished_callback();


	std::map<std::string, ShipModel *> _models; // modèles
	std::vector<Ship *> _ships; // vaisseaux
	std::vector<Level *> _levels; // niveaux
	uint _current_level_idx; // indice niveau courant
	glm::vec2 _pt_min, _pt_max; // emprise niveaux
	GameMode _mode; // mode de jeu

	std::vector<std::pair<std::string, uint> > _highest_scores; // meilleurs scores
	uint _score; // score du joueur
	int _new_highest_idx; // indice dans la liste des meilleurs scores du nouveau score
	int _new_highest_char_idx; // indice du char sur les 3 dispos du nouveau meilleur score

	bool _draw_aabb, _draw_footprint, _draw_texture, _draw_star; // faut-il afficher les AABB, footprints, textures, étoiles
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	glm::mat4 _camera2clip; // glm::ortho
	Font * _font; // font pour écriture textes

	bool _key_left, _key_right, _key_up, _key_down, _key_a, _key_z; // les touches sont-elle enfoncées
	bool _is_joystick;
	glm::vec2 _joystick; // valeurs x, y stick joystick
	bool _joystick_a, _joystick_b; // boutons 

	// les static sont nécessaires à cause du callback géré par Mix_HookMusicFinished
	static Mix_Music * _music; // musique courante
	static std::string _next_music_path; // prochain chemin musique à jouer
	bool _play_music, _play_sounds; // faut-il jouer la musique, les sons

	StarSystem * _star_system;
};


#endif

#ifndef ASTEROID_H
#define ASTEROID_H

#include <vector>
#include <map>
#include <chrono>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "json.hpp"

#include "bbox_2d.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"


// couleurs
const glm::dvec4 BORDER_COLOR(0.3, 0.3, 0.2, 1.0);
const glm::dvec4 AABB_FRIENDLY_COLOR(0.0, 1.0, 0.0, 1.0);
const glm::dvec4 AABB_UNFRIENDLY_COLOR(1.0, 0.0, 0.0, 1.0);
const glm::dvec4 FOOTPRINT_FRIENDLY_COLOR(0.0, 1.0, 1.0, 1.0);
const glm::dvec4 FOOTPRINT_UNFRIENDLY_COLOR(1.0, 0.0, 1.0, 1.0);
// vitesse héro
const float HERO_VELOCITY= 0.1;
// plans z de contrainte d'affichage de glm::ortho
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;
// taille textures
const unsigned int TEXTURE_SIZE= 1024;
// temps d'invulnérabilité après avoir été touché
const unsigned int HIT_UNTOUCHABLE_MS= 130;
// temps de rotation dans un sens lors d'un hit
const unsigned int HIT_ROTATION_MS= 60;
// incrément de rotation lors d'un hit
const float HIT_ROTATION_INC= 0.1;
// temps d'animation de la mort
const unsigned int DEATH_MS= 200;
// incrément de scale lors d'une mort
const float DEATH_SCALE_INC= 0.1;
// nom de l'action principale de chaque ship (voir jsons)
const std::string MAIN_ACTION_NAME= "main";
// temps de fade in / out de la musique
const unsigned int MUSIC_FADE_IN_MS= 1500;
const unsigned int MUSIC_FADE_OUT_MS= 1500;


// type de ship
enum ShipType {HERO, ENEMY, BULLET};
// mode jeu : PLAYING = jeu en cours ; INACTIVE = affichage meilleurs scores ; SET_SCORE_NAME = saisie nom meilleur score
enum GameMode {PLAYING, INACTIVE, SET_SCORE_NAME};
// type d'événement dans un niveau
enum EventType {NEW_ENEMY, LEVEL_END};


class ShipModel;


// action d'un vaisseau
class Action {
public:
	Action();
	Action(glm::vec2 direction, int t, std::string bullet_name, unsigned int t_shooting, std::string texture_name, Mix_Chunk * shoot_sound);
	~Action();
	friend std::ostream & operator << (std::ostream & os, const Action & action);


	glm::vec2 _direction; // direction du vaisseau
	int _t; // durée de l'action en ms
	unsigned int _t_shooting; // fréquence de tir en ms si l'action implique du tir
	std::string _bullet_name; // nom du type de munition
	ShipModel * _bullet_model; // modèle munition
	std::string _texture_name; // nom de l'ActionTexture associée
	Mix_Chunk * _shoot_sound; // son tir
};


// Ensemble d'images à animer associées à une action d'un vaisseau
class ActionTexture {
public:
	ActionTexture();
	ActionTexture(std::vector<std::string> & pngs, std::vector<unsigned int> & t_anims, AABB_2D & footprint);
	~ActionTexture();
	friend std::ostream & operator << (std::ostream & os, const ActionTexture & at);


	std::vector<std::string> _pngs; // liste des images PNG
	std::vector<unsigned int> _t_anims; // durées d'affichage des textures
	unsigned int _first_idx; // indice de la 1ere image liée a cette action dans la liste d'actions stockées dans un GL_TEXTURE_2D_ARRAY
	AABB_2D _footprint; // un footprint pour une action en prenant le + petit footprint des pngs de l'action
};


// Modèle vaisseau
class ShipModel {
public:
	ShipModel();
	ShipModel(std::string json_path);
	~ShipModel();
	friend std::ostream & operator << (std::ostream & os, const ShipModel & model);


	std::string _json_path; // chemin du json de paramètres
	ShipType _type; // type de vaisseau
	pt_type _size; // taille
	unsigned int _score_hit; // valeur remportée par le joueur lors d'un hit
	unsigned int _score_death; // valeur remportée par le joueur lors d'une mort
	unsigned int _lives; // nombre de vies
	std::map<std::string, std::vector<Action *> > _actions; // une action correspond en fait à une liste de Action *
	std::map<std::string, ActionTexture *> _textures; // dico des ActionTexture
	Mix_Chunk * _hit_sound, * _death_sound; // sons
};


// Vaisseau
class Ship {
public:
	Ship();
	Ship(ShipModel * model, pt_type pos, bool friendly, std::chrono::system_clock::time_point t);
	~Ship();
	void anim(std::chrono::system_clock::time_point t, bool play_sounds);
	Action * get_current_action();
	ShipModel * get_current_bullet_model();
	ActionTexture * get_current_texture();
	void set_current_action(std::string action_name, std::chrono::system_clock::time_point t);
	bool hit(std::chrono::system_clock::time_point t, bool play_sounds);
	friend std::ostream & operator << (std::ostream & os, const Ship & ship);


	ShipModel * _model; // modèle vaisseau
	AABB_2D _aabb; // AABB liéé à l'emprise totale du PNG
	AABB_2D _footprint; // AABB réduit à l'emprise du vaisseau
	float _rotation, _scale; // angle de rotation, facteur de mise à l'échelle
	bool _friendly; // est-ce le héro ou un ami
	glm::vec2 _velocity; // vecteur vitesse
	bool _shooting; // si vrai alors il faut créer une nouvelle munition
	std::string _current_action_name; // nom action courante
	unsigned int _idx_action; // indice dans la liste d'action
	unsigned int _lives; // nombre de vies restantes
	unsigned int _idx_anim; // indice dans la liste des PNG
	bool _hit; // vient t'il d'être touché
	float _hit_value; // valeur flottante pour le fragment shader montrant l'aspect hit
	bool _dead; // est t'il mort
	bool _delete; // doit il etre supprimé ; entre _dead et _delete il y a un temps d'anim de la mort
	float _alpha; // transparence
	std::chrono::system_clock::time_point _t_anim_start; // temps de début de l'affichage du PNG
	std::chrono::system_clock::time_point _t_action_start; // temps de début de l'action
	std::chrono::system_clock::time_point _t_die; // temps de mort
	std::chrono::system_clock::time_point _t_last_bullet; // temps de dernier tir
	std::chrono::system_clock::time_point _t_last_hit; // temps de dernier hit
};


// Evénement dans le niveau
class Event {
public:
	Event();
	Event(EventType type, unsigned int t, glm::vec2 position, std::string enemy);
	~Event();
	friend std::ostream & operator << (std::ostream & os, const Event & event);


	EventType _type; // type
	unsigned int _t; // temps en ms en absolu à partir du début du niveau auquel déclencher cet événement
	glm::vec2 _position; // position de l'ennemi (dans le cas _type == NEW_ENEMY)
	std::string _enemy; // nom de l'ennemi (dans le cas _type == NEW_ENEMY)
};


// Niveau
class Level {
public:
	Level();
	Level(std::string json_path, std::chrono::system_clock::time_point t);
	~Level();
	void reinit(std::chrono::system_clock::time_point t);
	friend std::ostream & operator << (std::ostream & os, const Level & level);


	std::string _json_path; // json du niveau
	std::chrono::system_clock::time_point _t_start; // temps de début du niveau
	std::vector<Event *> _events; // liste d'événements du niveau
	unsigned int _current_event_idx; // indice de l'événement courant
	std::string _music_path; // chemin musique niveau
};


// Classe principale du jeu
class Asteroid {
public:
	Asteroid();
	Asteroid(GLuint prog_aabb, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, bool is_joystick, std::chrono::system_clock::time_point t);
	~Asteroid();

	// chargements
	void load_models();
	void load_levels(std::chrono::system_clock::time_point t);
	void fill_texture_array();
	
	// dessins
	void draw_border_aabb();
	void draw_ship_aabb();
	void draw_ship_footprint();
	void draw_ship_texture();
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
	
	// animation
	void anim_playing(std::chrono::system_clock::time_point t);
	void anim_inactive();
	void anim_set_score_name();
	void anim(std::chrono::system_clock::time_point t);

	// input
	bool key_down(InputState * input_state, SDL_Keycode key, std::chrono::system_clock::time_point t);
	bool key_up(InputState * input_state, SDL_Keycode key, std::chrono::system_clock::time_point t);
	bool joystick_down(unsigned int button_idx, std::chrono::system_clock::time_point t);
	bool joystick_up(unsigned int button_idx, std::chrono::system_clock::time_point t);
	bool joystick_axis(unsigned int axis_idx, int value, std::chrono::system_clock::time_point t);
	
	// événements
	void add_level_events(std::chrono::system_clock::time_point t);
	void set_level(unsigned int level_idx, std::chrono::system_clock::time_point t);
	void add_rand_enemy(std::chrono::system_clock::time_point t);
	void reinit(std::chrono::system_clock::time_point t);
	void gameover();
	
	// scores
	void read_highest_scores();
	void write_highest_scores();

	// music; les static sont nécessaires à cause du callback géré par Mix_HookMusicFinished
	static void set_music(std::string music_path, unsigned int music_fade_in_ms=MUSIC_FADE_IN_MS);
	void set_music_with_fadeout(std::string music_path, unsigned int music_fade_out_ms=MUSIC_FADE_OUT_MS, unsigned int music_fade_in_ms=MUSIC_FADE_IN_MS);
	static void music_finished_callback();


	std::map<std::string, ShipModel *> _models; // modèles
	std::vector<Ship *> _ships; // vaisseaux
	std::vector<Level *> _levels; // niveaux
	unsigned int _current_level_idx; // indice niveau courant
	glm::vec2 _pt_min, _pt_max; // emprise niveaux
	GameMode _mode; // mode de jeu

	std::vector<std::pair<std::string, unsigned int> > _highest_scores; // meilleurs scores
	unsigned int _score; // score du joueur
	int _new_highest_idx; // indice dans la liste des meilleurs scores du nouveau score
	int _new_highest_char_idx; // indice du char sur les 3 dispos du nouveau meilleur score

	bool _draw_aabb, _draw_footprint, _draw_texture; // faut-il afficher les AABB, footprints, textures
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint _texture_id; // texture array pour tous les PNGs
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
};


#endif

#ifndef ANIM_2D_H
#define ANIM_2D_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "gl_utils.h"
#include "bbox_2d.h"
#include "input_state.h"
#include "typedefs.h"


// constantes ------------------------------------------------------------------
// _z doit etre entre ces bornes
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;

// si vitesse en dessous de ce seuil, changement d'action
const number UPDATE_ACTION_TRESH= 0.1f;

// si hero sort du rectangle _viewpoint +- MOVE_VIEWPOINT, o, déplace _viewpoint
const pt_2d MOVE_VIEWPOINT(5.0f, 5.0f);

// incrément gravité et max
const number GRAVITY_INC= 1.0f;
const number GRAVITY_MAX= 20.0f;

// distance seuil pour changer de checkpoint
const number CHECKPOINT_TRESH= 0.1f;

// on multiplie la correction à apporter par ce facteur pour éviter l'intersection. Foireux...
const number CORRECT_FACTOR= 1.1f;


// enums -----------------------------------------------------------------------
enum ObjectPhysics {STATIC_DESTRUCTIBLE, STATIC_INDESTRUCTIBLE, STATIC_UNSOLID, FALLING, CHECKPOINT_SOLID, CHECKPOINT_UNSOLID, CHECKPOINT_SOLID_TOP};
enum CharacterType {CHARACTER_2D, ANIM_CHARACTER_2D, PERSON_2D};


// fonctions ------------------------------------------------------------------
// enum ObjectPhysics vers string
ObjectPhysics str2physics(std::string s);

// enum CharacterType vers string
CharacterType str2character_type(std::string s);


// classes ---------------------------------------------------------------------
class CheckPoint {
public:
	CheckPoint();
	CheckPoint(pt_2d pos, number velocity);
	~CheckPoint();


	pt_2d _pos;
	number _velocity;
};


// (aabb = emprise texture) + (footprint = emprise physique) + vitesse ; éventuellement des checkpoints pour les objets mouvants
// dans le constructeur footprint doit etre compris entre 0 et 1 en position et en taille
class Object2D {
public:
	Object2D();
	Object2D(AABB_2D * aabb, AABB_2D * footprint, ObjectPhysics physics, std::vector<CheckPoint> checkpoints = std::vector<CheckPoint>());
	Object2D(const Object2D & obj);
	~Object2D();
	void update_pos(number elapsed_time);
	void update_velocity();
	void update_footprint_pos();
	void set_aabb_pos(pt_2d pos);
	void set_footprint(AABB_2D * footprint);
	friend std::ostream & operator << (std::ostream & os, const Object2D & obj);


	AABB_2D * _aabb; // emprise texture
	pt_2d _footprint_offset; // pt bas gauche du footprint dans le repere _aabb
	AABB_2D * _footprint; // emprise physique
	pt_2d _velocity; // vitesse
	ObjectPhysics _physics; // type physique objet
	std::vector<CheckPoint *> _checkpoints; // chekpoints éventuels
	unsigned int _idx_checkpoint; // indice chekpoint courant
	std::vector<Object2D *> _bottom; // liste des objs en contact dessous
	std::vector<Object2D *> _top; // liste des objs en contact dessus
	Object2D * _referential; // obj ref, nullptr si sol, sinon objet mouvant sur lequel se trouve l'objet
};


// test d'intersection entre un obj en mouvement et un obj statique
//bool obj_intersect(const Object2D * anim_obj, const Object2D * static_obj, const number time_step, pt_2d & contact_pt, pt_2d & contact_normal, number & contact_time);


// action d'un character animé -----------------------------------------------------------------------------------------------------
class Action {
public:
	Action();
	~Action();
	friend std::ostream & operator << (std::ostream & os, const Action & action);


	std::string _name; // nom
	std::vector<std::string> _pngs; // liste chemins images
	unsigned int _first_idx; // indice de la 1ere image liée a cette action dans la liste d'actions stockées dans un GL_TEXTURE_2D_ARRAY
	unsigned int _n_idx; // nombre d'images liées a cette action
	number _anim_time; // temps à attendre pour passer d'une image à la suivante
	AABB_2D * _footprint; // emprise physique ; pos et size entre 0 et 1
};


class Character2D;

// classe virtuelle des textures dont héritent StaticTexture et AnimTexture --------------------------------------------------------
class Texture2D {
public:
	Texture2D();
	Texture2D(GLuint prog_draw, std::string path, ScreenGL * screengl, ObjectPhysics physics, CharacterType character_type);
	virtual ~Texture2D();
	virtual void draw() = 0;
	virtual void update() = 0;
	void set_model2world(glm::mat4 model2world);
	Action * get_action(std::string action_name);


	GLuint _texture_id;
	GLuint _vbo;
	GLuint _prog_draw;
	glm::mat4 _camera2clip;
	glm::mat4 _model2world;
	ScreenGL * _screengl;
	unsigned int _n_aabbs; // nombre de characters liés à cette texture
	std::vector<Character2D *> _characters; // liste des characters
	std::vector<Action *> _actions; // actions possibles ; pour une StaticTexture 1 action
	std::string _name; // nom
	ObjectPhysics _physics; // type physique dont vont hériter tous les Object2D liées à cette texture
	CharacterType _character_type; // type character
};


class StaticTexture : public Texture2D {
public:
	StaticTexture();
	StaticTexture(GLuint prog_draw, std::string path, ScreenGL * screengl, ObjectPhysics physics, CharacterType character_type);
	~StaticTexture();
	void draw();
	void update();


	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _tex_loc, _alpha_loc;
	number _alpha;
};


class AnimTexture : public Texture2D {
public:
	AnimTexture();
	AnimTexture(GLuint prog_draw, std::string path, ScreenGL * screengl, ObjectPhysics physics, CharacterType character_type);
	~AnimTexture();
	void draw();
	void update();


	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _texture_array_loc, _current_layer_loc;
	std::map<std::string, number> _velocities; // vitesses liées à un nom
};


// classes de characters ------------------------------------------------------------------------------------------------------
// Character2D = liaison entre objet et texture statique
class Character2D {
public:
	Character2D();
	Character2D(Object2D * obj, Texture2D * texture, number z);
	virtual ~Character2D(); // virtual pour pouvoir faire du polymorphisme


	Object2D * _obj;
	Texture2D * _texture;
	number _z;
};


// AnimatedCharacter2D = char animé
class AnimatedCharacter2D : public Character2D {
public:
	AnimatedCharacter2D();
	AnimatedCharacter2D(Object2D * obj, Texture2D * texture, number z);
	~AnimatedCharacter2D();
	void anim(number elapsed_time);
	void set_action(unsigned int idx_action);
	void set_action(std::string action_name);
	std::string current_action();


	Action * _current_action; // action courante
	unsigned int _current_anim; // indice d'animation au sein de l'action courante
	number _accumulated_time; // temps à comparer avec Action._anim_time
};


// char animé et se déplacant ; héros, ennemis
class Person2D : public AnimatedCharacter2D {
public:
	Person2D();
	Person2D(Object2D * obj, Texture2D * texture, number z);
	~Person2D();
	void update_velocity();
	void update_action();
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	void ia();
	std::string current_direction();
	std::string current_type();


	bool _left_pressed, _right_pressed, _down_pressed, _up_pressed, _lshift_pressed;
	bool _jump;
};


// parcours SVG pour chargement niveau --------------------------------------------------------------------------------------
class SVGParser {
public:
	SVGParser();
	SVGParser(std::string svg_path, ScreenGL * screengl);
	~SVGParser();
	AABB_2D svg2screen(AABB_2D aabb);


	ScreenGL * _screengl;
	AABB_2D * _view;
	std::vector<std::map<std::string, std::string> > _models;
	std::vector<std::map<std::string, std::string> > _objs;
};


// level
class Level {
public:
	Level();
	// méthodes appelés par le constructeur----
	void gen_textures(GLuint prog_draw_anim, GLuint prog_draw_static, ScreenGL * screengl, SVGParser * svg_parser, bool verbose);
	void update_static_textures(SVGParser * svg_parser);
	void update_anim_textures(SVGParser * svg_parser);
	void add_characters(SVGParser * svg_parser, bool verbose);
	// ----------------------------------------
	Level(GLuint prog_draw_anim, GLuint prog_draw_static, GLuint prog_draw_aabb, std::string path, ScreenGL * screengl, bool verbose = false);
	~Level();
	Texture2D * get_texture(std::string texture_name, bool verbose = true);
	void add_character(std::string texture_name, AABB_2D * aabb, number z, std::vector<CheckPoint> checkpoints = std::vector<CheckPoint>(), bool update_texture = true);
	void delete_character(Character2D * character);
	// méthodes appelées par anim() -----------
	void update_velocities();
	void intersections(number elapsed_time);
	void deletes();
	void update_positions(number elapsed_time);
	void update_actions();
	void anim_characters(number elapsed_time);
	void update_textures();
	void follow_hero();
	void anim(number elapsed_time);
	// ----------------------------------------
	void draw();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);


	std::vector<Texture2D *> _textures;
	std::vector<Character2D *> _characters;
	
	unsigned int _w, _h;
	number _block_w, _block_h;
	ScreenGL * _screengl;
	Person2D * _hero;
	pt_2d _viewpoint;
	bool _draw;
};


// classe debug pour visualiser aabbs et footprints
class LevelDebug {
public:
	LevelDebug();
	LevelDebug(GLuint prog_draw_aabb, Level * level, ScreenGL * screengl);
	~LevelDebug();
	void draw();
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);


	GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _color_loc, _z_loc;
	glm::mat4 _camera2clip;
	glm::mat4 _model2world;
	ScreenGL * _screengl;
	Level * _level;
	unsigned int _n_aabbs;
	bool _draw_aabb, _draw_footprint;
};


#endif

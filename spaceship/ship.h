
#ifndef SHIP_H
#define SHIP_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "objfile.h"
#include "constantes.h"
#include "level.h"
#include "font.h"


// force appliquée en un pt
class AppliedForce {
public:
	AppliedForce();
	AppliedForce(const glm::vec3 & f, const glm::vec3 & pt, std::string name);
	
	glm::vec3 _f;
	glm::vec3 _pt;
	std::string _name;
	bool _is_active;
};


// dessin des forces
class ForcesDraw {
public:
	ForcesDraw();
	ForcesDraw(GLuint prog_draw_basic);
	void draw();
	void anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip);
	void sync2appliedforces(std::vector<AppliedForce> & applied_forces);
	
	unsigned int _n_forces;
	glm::mat4 _model2world;
	glm::mat4 _model2clip;
	GLuint _buffer;
	GLfloat _data[12* NMAX_FORCE_DRAW];
	GLint _model2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _prog_draw;
};


// corps rigide : équations ...
class RigidBody {
public:
	RigidBody();
	RigidBody(std::string model_path, bool is_paves, float size_factor);
	void as_paves(std::string model_path, float size_factor);
	void from_txt(std::string model_path, float size_factor);
	void anim(std::vector<AppliedForce> & applied_forces);
	
	float _xmin, _xmax, _ymin, _ymax, _zmin, _zmax; // boite englobante
	float _radius; // rayon sphère englobante
	float _mass; // masse
	glm::vec3 _mass_center; // center of mass
	glm::mat3 _local_inertia_matrix; // moment d'inertie dans le repère du ship
	glm::mat3 _local_inertia_matrix_inverse; // inverse du moment d'inertie dans le repère du ship
	glm::mat3 _world_inertia_matrix_inverse; // inverse du moment d'inertie dans le repère monde

	//glm::vec3 _linear_a;
	glm::vec3 _linear_momentum; // moment linéaire
	glm::vec3 _linear_v; // vitesse
	glm::vec3 _position; // position

	//glm::vec3 _angular_a;
	glm::vec3 _angular_momentum; // moment angulaire
	glm::vec3 _angular_v; // vitesse angulaire
	glm::mat3 _rotation_matrix; // matrice de rotation
	glm::quat _quaternion; // quaternion associé à la rotation
};


// un point est-il hors monde
bool out_of_bound(const glm::vec3 & position);


// camera de suivi du ship pour plus de dynamisme
class FollowCamera {
public:
	FollowCamera();
	FollowCamera(GLuint prog_draw_basic);
	void draw();
	void anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip, RigidBody & rigid_body);
	
	glm::mat4 _model2world;
	glm::mat4 _model2clip;
	
	GLuint _buffer;
	GLfloat _data[36];
	GLint _model2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _prog_draw;
	
	glm::vec3 _position; // position
	glm::mat3 _rotation_matrix; // matrice de rotation
	glm::quat _quaternion; // quaternion associé à la rotation
};


// balle tirée par ship
class Bullet {
public:
	Bullet();
	Bullet(GLuint prog_draw_3d, GLuint prog_draw_basic, std::string model_path, std::string material_path, const glm::vec3 & position, const glm::mat3 & rotation_matrix, float size_factor, const glm::vec3 & color);
	~Bullet();
	void draw();
	void anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip);
	
	bool _is_active;
	ModelObj _model;
	glm::vec3 _position; // position
	glm::mat3 _rotation_matrix; // matrice de rotation
};


// transfo d'une particule de l'explosion
struct ExplosionTransfo {
	glm::vec3 _rotation;
	float _angle;
	glm::vec3 _translation;
	glm::vec3 _init_translation;
	float _scale;
	
	void reinit(float translation);
};


// explosion
class Explosion {
public:
	Explosion();
	Explosion(GLuint prog_draw_3d, GLuint prog_draw_basic, std::string model_path, std::string material_path, const glm::vec3 & position, const ExplosionParams & ep);
	void release();
	void draw();
	void anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip);
	void reinit();
	
	ExplosionParams _ep;
	glm::vec3 _position;

	unsigned int _n_faces;
	float * _vertices;
	unsigned int * _faces;
	GLuint _buffers[3];
	
	float _world2camera[16];
	float _world2clip[16];
	//float _normal[9];
	
	float _ambient[3];
	float _shininess;
	float _alpha;

	GLint _world2clip_loc, _world2camera_loc;
	GLint _position_loc, _normal_loc;
	GLint _ambient_color_loc, _shininess_loc, _diffuse_color_loc;
	GLint _alpha_loc;
	GLint _instanced_matrix_loc;
	GLuint _prog_draw;
	
	glm::mat4* _model_matrices;
	ExplosionTransfo* _transfo;
	
	bool _is_active;
};


// générer une explosion à un endroit et avec une couleur
void new_explosion(std::vector<Explosion*> & explosions, const glm::vec3 & position, const glm::vec3 & color);


// cf https://stackoverflow.com/questions/396084/headers-including-each-other-in-c
class Ranking;
class GlobalMsg;


// ship : regroupe ForcesDraw, RigidBody, FollowCamera, Bullet
class Ship {
public:
    Ship();
    Ship(std::string id, GLuint prog_draw_3d, GLuint prog_draw_basic, std::string model_path, std::string material_path, bool is_paves, float size_factor, const glm::vec3 & color);
	~Ship();
	
	void init_applied_forces();
	void draw(bool force_draw, bool draw_follow_camera);
	void anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip);
	void update_keys_pressed();
	void sync_keys2forces();
	void collision(RandTerrain * level, std::vector<Ship *> & ships, std::vector<Explosion *> & little_explosions, std::vector<Explosion *> & big_explosions, Ranking * ranking, GlobalMsg * global_msg);
	void reinit();
	
	std::string _id;
	glm::vec3 _color;
	ModelObj _model;
	std::vector<AppliedForce> _applied_forces;
	ForcesDraw _forces_draw;
	RigidBody _rigid_body;
	FollowCamera _follow_camera;
	std::vector<Bullet*> _bullets;
	bool _is_shooting;
	unsigned int _tik_shooting_1, _tik_shooting_2;
	
	std::map<std::string, bool> _keypresseds;
	std::map<std::string, std::vector<std::string> > _key2forces;
};


// pour les ennemis ; a un Ship* comme attribut
class IA {
public:
	IA();
	IA(std::string id, GLuint prog_draw_3d, GLuint prog_draw_basic, std::string model_path, std::string material_path, bool is_paves, float size_factor, const glm::vec3 & color);
	~IA();
	bool align2target();
	void think(RandTerrain * level, std::vector<Ship *> & ships);
	
	Ship* _ship;
	glm::vec3 _target_direction;
	unsigned int _tik1, _tik2;
};


// points d'un ship
struct ShipPoints {
	std::string _id;
	glm::vec3 _color;
	int _n_loose;
	int _n_win;
};


// classement entre ships
class Ranking {
public:
	Ranking();
	Ranking(Font * font, std::vector<Ship *> & ships);
	void draw();
	bool loose(std::string id);
	bool win(std::string id);
	bool sort_ships();
	void print();
	std::string id_first();
	
	Font * _font;
	std::vector<ShipPoints> _ship_points;
};


class GlobalMsg {
public:
	GlobalMsg();
	GlobalMsg(Font * font);
	void new_msg(std::string msg, const glm::vec3 & color);
	void draw();
	void anim();

	Font * _font;
	bool _is_active;
	std::string _msg;
	glm::vec3 _color;
};


#endif

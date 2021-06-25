#ifndef BBOX_H
#define BBOX_H

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "bbox_2d.h"


// boite englobante dont les axes sont alignes avec le repere (axis aligned bouding box)
class AABB {
public:
	AABB();
	AABB(const glm::vec3 & vmin, const glm::vec3 & vmax);
	~AABB();
	void set_vmin_vmax(const glm::vec3 & vmin, const glm::vec3 & vmax);


	glm::vec3 _vmin, _vmax;
	float _radius;
	glm::vec3 _pts[8]; // sommets du parallelepipede droit
};


// boite englobante
class BBox {
public:
	BBox();
	BBox(const glm::vec3 & vmin, const glm::vec3 & vmax, const glm::mat4 & model2world);
	~BBox();
	void set_model2world(const glm::mat4 & model2world);


	glm::vec3 _vmin, _vmax;
	float _radius;
	glm::mat4 _model2world;
	glm::vec3 _pts[8]; // sommets du parallelepipede droit
	AABB * _aabb;
};


// proprietes communes a un objet statique et a un objet dynamique
class InstancePosRot {
public:
	InstancePosRot();
	InstancePosRot(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale);
	InstancePosRot(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale, AABB * aabb);
	~InstancePosRot();
	void set_pos_rot_scale(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale);
	//void set_pos_rot_scale(const glm::mat4 & mat);
	void update_dist2(glm::vec3 view_eye);


	glm::vec3 _position;
	glm::quat _rotation;
	glm::vec3 _scale;
	glm::mat4 _model2world;
	BBox * _bbox;
	AABB_2D * _emprise;
	bool _active;
	bool _selected;
	float _dist2;
};


// fonctions utilitaires
bool aabb_intersects_aabb(AABB * aabb_1, AABB * aabb_2);
bool aabb_intersects_bbox(AABB * aabb, BBox * bbox);
float aabb_distance_pt_2(AABB * aabb, const glm::vec3 & pt);
float aabb_distance_pt(AABB * aabb, const glm::vec3 & pt);
bool ray_intersects_aabb(glm::vec3 origin, glm::vec3 direction, AABB * aabb, float & t_hit);
bool segment_intersects_aabb(const glm::vec3 & pt1, const glm::vec3 & pt2, AABB * aabb);


// dessin de BBox ; a utiliser avec parcimonie ; lent si trop d'objets
/*class BBoxDraw {
public:
	BBoxDraw();
	BBoxDraw(GLuint prog_draw, const glm::vec3 & vmin, const glm::vec3 & vmax, const glm::vec3 & color);
	BBoxDraw(GLuint prog_draw, AABB * aabb, const glm::vec3 & color);
	BBoxDraw(GLuint prog_draw, BBox * bbox, const glm::vec3 & color);
	~BBoxDraw();
	void init();
	void draw();
	void anim(const glm::mat4 & world2clip);
	void set_model2world(const glm::mat4 & model2world);


	glm::mat4 _model2world;
	glm::mat4 _world2clip;
	glm::vec3 _vmin, _vmax;
	glm::vec4 _color;
	GLuint _prog_draw;
	GLint _model2world_loc, _world2clip_loc, _position_loc, _color_loc;
	GLuint _buffer;
	float _data[72];
};*/


#endif

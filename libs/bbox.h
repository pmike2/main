#ifndef BBOX_H
#define BBOX_H

#include <vector>
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "typedefs.h"
#include "bbox_2d.h"


// boite englobante dont les axes sont alignes avec le repere (axis aligned bouding box)
class AABB {
public:
	AABB();
	AABB(const pt_type_3d & vmin, const pt_type_3d & vmax);
	AABB(AABB_2D * aabb_2d);
	~AABB();
	void set_vmin_vmax(const pt_type_3d & vmin, const pt_type_3d & vmax);
	static std::vector<std::vector<unsigned int> > triangles_idxs();
	std::vector<pt_type_3d> segments();
	void translate(pt_type_3d v);
	void scale(number x);
	pt_type_3d center();
	void set_z(number z);
	friend std::ostream & operator << (std::ostream & os, const AABB & aabb);


	pt_type_3d _vmin, _vmax;
	number _radius;
	pt_type_3d _pts[8]; // sommets du parallelepipede droit
};


// boite englobante
class BBox {
public:
	BBox();
	BBox(const pt_type_3d & vmin, const pt_type_3d & vmax, const mat_4d & model2world);
	BBox(AABB * aabb);
	~BBox();
	void set_model2world(const mat_4d & model2world);
	static std::vector<std::vector<unsigned int> > triangles_idxs();
	friend std::ostream & operator << (std::ostream & os, const BBox & bbox);


	pt_type_3d _vmin, _vmax;
	number _radius;
	mat_4d _model2world;
	pt_type_3d _pts[8]; // sommets du parallelepipede droit
	AABB * _aabb;
};


// proprietes communes a un objet statique et a un objet dynamique
class InstancePosRot {
public:
	InstancePosRot();
	InstancePosRot(const pt_type_3d & position, const quat & rotation, const pt_type_3d & scale);
	InstancePosRot(const pt_type_3d & position, const quat & rotation, const pt_type_3d & scale, AABB * aabb);
	~InstancePosRot();
	void set_pos_rot_scale(const pt_type_3d & position, const quat & rotation, const pt_type_3d & scale);
	// lent, mieux vaut utiliser l'autre
	void set_pos_rot_scale(const mat_4d & mat);
	void update_dist2(pt_type_3d view_eye);


	pt_type_3d _position;
	quat _rotation;
	pt_type_3d _scale;
	mat_4d _model2world;
	BBox * _bbox;
	AABB_2D * _emprise;
	bool _active;
	bool _selected;
	number _dist2;
};


// fonctions utilitaires
bool aabb_intersects_aabb(AABB * aabb_1, AABB * aabb_2);
bool aabb_intersects_bbox(AABB * aabb, BBox * bbox);
bool bbox_intersects_bbox(BBox * bbox_1, BBox * bbox_2);
number aabb_distance_pt_2(AABB * aabb, const pt_type_3d & pt);
number aabb_distance_pt(AABB * aabb, const pt_type_3d & pt);
bool ray_intersects_aabb(pt_type_3d origin, pt_type_3d direction, AABB * aabb, number & t_hit);
bool segment_intersects_aabb(const pt_type_3d & pt1, const pt_type_3d & pt2, AABB * aabb);


#endif

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
	AABB(const pt_3d & vmin, const pt_3d & vmax);
	AABB(AABB_2D * aabb_2d);
	~AABB();
	void set_vmin_vmax(const pt_3d & vmin, const pt_3d & vmax);
	static std::vector<std::vector<uint> > triangles_idxs();
	std::vector<pt_3d> segments();
	void translate(pt_3d v);
	void scale(number x);
	pt_3d center();
	pt_3d bottom_center();
	pt_3d size();
	AABB_2D * aabb2d();
	void set_z(number z);
	friend std::ostream & operator << (std::ostream & os, const AABB & aabb);


	pt_3d _vmin, _vmax;
	number _radius; // rayon sphère englobante
	number _base_radius; // rayon cercle englobant AABB2D
	pt_3d _pts[8]; // sommets du parallelepipede droit
};


// boite englobante
class BBox {
public:
	BBox();
	BBox(const pt_3d & vmin, const pt_3d & vmax, const mat_4d & model2world = mat_4d(1.0));
	BBox(AABB * aabb);
	~BBox();
	void update_radius();
	void set_aabb(AABB * aabb);
	void set_model2world(const mat_4d & model2world);
	static std::vector<std::vector<uint> > triangles_idxs();
	std::vector<pt_3d> segments();
	BBox_2D * bbox2d();
	friend std::ostream & operator << (std::ostream & os, const BBox & bbox);


	pt_3d _vmin, _vmax;
	number _radius;
	mat_4d _model2world;
	pt_3d _pts[8]; // sommets du parallelepipede droit
	AABB * _aabb;
};


// proprietes communes a un objet statique et a un objet dynamique
class InstancePosRot {
public:
	InstancePosRot();
	InstancePosRot(const pt_3d & position, const quat & rotation, const pt_3d & scale);
	InstancePosRot(const pt_3d & position, const quat & rotation, const pt_3d & scale, AABB * aabb);
	~InstancePosRot();
	void set_pos_rot_scale(const pt_3d & position, const quat & rotation, const pt_3d & scale);
	// lent, mieux vaut utiliser l'autre
	void set_pos_rot_scale(const mat_4d & mat);
	void update_dist2(pt_3d view_eye);


	pt_3d _position;
	quat _rotation;
	pt_3d _scale;
	mat_4d _model2world;
	BBox * _bbox;
	//AABB_2D * _emprise;
	bool _active;
	bool _selected;
	number _dist2;
};


#endif

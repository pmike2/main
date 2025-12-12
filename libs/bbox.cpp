#include <cmath>
#include <iostream>
#include <cstdlib>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

#include "bbox.h"

using namespace std;


AABB::AABB() {
	set_vmin_vmax(pt_type_3d(0.0), pt_type_3d(0.0));
}


AABB::AABB(const pt_type_3d & vmin, const pt_type_3d & vmax) {
	set_vmin_vmax(vmin, vmax);
}


AABB::AABB(AABB_2D * aabb_2d) {
	set_vmin_vmax(pt_type_3d(aabb_2d->_pos.x, aabb_2d->_pos.y, -0.01), pt_type_3d(aabb_2d->_pos.x + aabb_2d->_size.x, aabb_2d->_pos.y + aabb_2d->_size.y, 0.01));
}


AABB::~AABB() {

}


void AABB::set_vmin_vmax(const pt_type_3d & vmin, const pt_type_3d & vmax) {
	_vmin= vmin;
	_vmax= vmax;
	
	/*_radius= abs(_vmin.x);
	if (abs(_vmax.x)> _radius) _radius= abs(_vmax.x);
	if (abs(_vmin.y)> _radius) _radius= abs(_vmin.y);
	if (abs(_vmax.y)> _radius) _radius= abs(_vmax.y);
	if (abs(_vmin.z)> _radius) _radius= abs(_vmin.z);
	if (abs(_vmax.z)> _radius) _radius= abs(_vmax.z);*/

	_radius = glm::distance(_vmin, _vmax) * 0.5;

	_pts[0]= pt_type_3d(_vmin.x, _vmin.y, _vmin.z);
	_pts[1]= pt_type_3d(_vmax.x, _vmin.y, _vmin.z);
	_pts[2]= pt_type_3d(_vmin.x, _vmax.y, _vmin.z);
	_pts[3]= pt_type_3d(_vmax.x, _vmax.y, _vmin.z);
	_pts[4]= pt_type_3d(_vmin.x, _vmin.y, _vmax.z);
	_pts[5]= pt_type_3d(_vmax.x, _vmin.y, _vmax.z);
	_pts[6]= pt_type_3d(_vmin.x, _vmax.y, _vmax.z);
	_pts[7]= pt_type_3d(_vmax.x, _vmax.y, _vmax.z);
}


vector<vector<unsigned int> > AABB::triangles_idxs() {
	vector<vector<unsigned int> > idx = {
		{0, 4, 2}, {2, 4, 6}, // x-
		{1, 3, 7}, {1, 7, 5}, // x+
		{0, 1, 5}, {0, 5, 4}, // y-
		{3, 2, 6}, {3, 6, 7}, // y+
		{0, 2, 3}, {0, 3, 1}, // z-
		{5, 7, 6}, {5, 6, 4} // z+
	};
	return idx;
}


std::vector<pt_type_3d> AABB::segments() {
	return std::vector<pt_type_3d> {
		_pts[0], _pts[1], _pts[1], _pts[3], _pts[3], _pts[2], _pts[2], _pts[0], // bottom
		_pts[4], _pts[5], _pts[5], _pts[7], _pts[7], _pts[6], _pts[6], _pts[4], // top
		_pts[0], _pts[1], _pts[1], _pts[5], _pts[5], _pts[4], _pts[4], _pts[0], // left
		_pts[2], _pts[6], _pts[6], _pts[7], _pts[7], _pts[3], _pts[3], _pts[2], // right
		_pts[1], _pts[3], _pts[3], _pts[7], _pts[7], _pts[5], _pts[5], _pts[1], // front
		_pts[0], _pts[2], _pts[2], _pts[6], _pts[6], _pts[4], _pts[4], _pts[0], // back
	};
}


void AABB::translate(pt_type_3d v) {
	set_vmin_vmax(_vmin+ v, _vmax+ v);
}


void AABB::scale(number x) {
	set_vmin_vmax(x* _vmin, x* _vmax);
}


pt_type_3d AABB::center() {
	return 0.5 * (_vmin + _vmax);
}


pt_type_3d AABB::bottom_center() {
	return pt_type_3d(0.5 * (_vmin.x + _vmax.x), 0.5 * (_vmin.y + _vmax.y), _vmin.z);
}


void AABB::set_z(number z) {
	set_vmin_vmax(pt_type_3d(_vmin.x, _vmin.y, z), pt_type_3d(_vmax.x, _vmax.y, _vmax.z - _vmin.z + z));
}


ostream & operator << (ostream & os, const AABB & aabb) {
	os << "vmin=" << glm::to_string(aabb._vmin) << " ; vmax=" << glm::to_string(aabb._vmax);
	return os;
}


// ------------------------------------------------------------------------------------------------------
BBox::BBox() : _vmin(pt_type_3d(0.0)), _vmax(pt_type_3d(0.0)), _radius(0.0) {
	_aabb= new AABB();
}


BBox::BBox(const pt_type_3d & vmin, const pt_type_3d & vmax, const mat_4d & model2world) : _vmin(vmin), _vmax(vmax), _model2world(model2world) {
	_radius= abs(_vmin.x);
	if (abs(_vmax.x)> _radius) _radius= abs(_vmax.x);
	if (abs(_vmin.y)> _radius) _radius= abs(_vmin.y);
	if (abs(_vmax.y)> _radius) _radius= abs(_vmax.y);
	if (abs(_vmin.z)> _radius) _radius= abs(_vmin.z);
	if (abs(_vmax.z)> _radius) _radius= abs(_vmax.z);
	
	_aabb= new AABB();
	set_model2world(_model2world);
}


BBox::BBox(AABB * aabb) : _vmin(aabb->_vmin), _vmax(aabb->_vmax), _model2world(mat_4d(1.0)) {
	_radius= abs(_vmin.x);
	if (abs(_vmax.x)> _radius) _radius= abs(_vmax.x);
	if (abs(_vmin.y)> _radius) _radius= abs(_vmin.y);
	if (abs(_vmax.y)> _radius) _radius= abs(_vmax.y);
	if (abs(_vmin.z)> _radius) _radius= abs(_vmin.z);
	if (abs(_vmax.z)> _radius) _radius= abs(_vmax.z);

	_aabb= new AABB();
	set_model2world(_model2world);
}


BBox::~BBox() {
	delete _aabb;
}


void BBox::set_model2world(const mat_4d & model2world) {
	_model2world= model2world;

	_pts[0]= pt_type_3d(_model2world* pt_type_4d(_vmin.x, _vmin.y, _vmin.z, 1.0));
	_pts[1]= pt_type_3d(_model2world* pt_type_4d(_vmax.x, _vmin.y, _vmin.z, 1.0));
	_pts[2]= pt_type_3d(_model2world* pt_type_4d(_vmin.x, _vmax.y, _vmin.z, 1.0));
	_pts[3]= pt_type_3d(_model2world* pt_type_4d(_vmax.x, _vmax.y, _vmin.z, 1.0));
	_pts[4]= pt_type_3d(_model2world* pt_type_4d(_vmin.x, _vmin.y, _vmax.z, 1.0));
	_pts[5]= pt_type_3d(_model2world* pt_type_4d(_vmax.x, _vmin.y, _vmax.z, 1.0));
	_pts[6]= pt_type_3d(_model2world* pt_type_4d(_vmin.x, _vmax.y, _vmax.z, 1.0));
	_pts[7]= pt_type_3d(_model2world* pt_type_4d(_vmax.x, _vmax.y, _vmax.z, 1.0));

	pt_type_3d vmin(_pts[0]);
	pt_type_3d vmax(_pts[0]);
	for (unsigned int i=1; i<8; ++i) {
		if (_pts[i].x< vmin.x) {
			vmin.x= _pts[i].x;
		}
		if (_pts[i].y< vmin.y) {
			vmin.y= _pts[i].y;
		}
		if (_pts[i].z< vmin.z) {
			vmin.z= _pts[i].z;
		}
		if (_pts[i].x> vmax.x) {
			vmax.x= _pts[i].x;
		}
		if (_pts[i].y> vmax.y) {
			vmax.y= _pts[i].y;
		}
		if (_pts[i].z> vmax.z) {
			vmax.z= _pts[i].z;
		}
	}
	_aabb->set_vmin_vmax(vmin, vmax);
}


vector<vector<unsigned int> > BBox::triangles_idxs() {
	vector<vector<unsigned int> > idx= {
		{0, 4, 2}, {2, 4, 6}, // x-
		{1, 3, 7}, {1, 7, 5}, // x+
		{0, 1, 5}, {0, 5, 4}, // y-
		{3, 2, 6}, {3, 6, 7}, // y+
		{0, 2, 3}, {0, 3, 1}, // z-
		{5, 7, 6}, {5, 6, 4} // z+
	};
	return idx;
}


std::ostream & operator << (std::ostream & os, const BBox & bbox) {
	os << "aabb = " << *bbox._aabb;
	os << " ; vmin=" << glm::to_string(bbox._vmin) << " ; vmax=" << glm::to_string(bbox._vmax);
	return os;
}


// ---------------------------------------------------------------------------------------------------------------------
InstancePosRot::InstancePosRot() {

}


InstancePosRot::InstancePosRot(const pt_type_3d & position, const quat & rotation, const pt_type_3d & scale) :
	_position(position), _rotation(rotation), _scale(scale), _active(false), _dist2(0.0), _selected(false)
{
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox= new BBox();
	_emprise= new AABB_2D();
}


InstancePosRot::InstancePosRot(const pt_type_3d & position, const quat & rotation, const pt_type_3d & scale, AABB * aabb) : 
	_position(position), _rotation(rotation), _scale(scale), _active(false), _dist2(0.0), _selected(false)
{
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox= new BBox(aabb->_vmin, aabb->_vmax, _model2world);
	_emprise= new AABB_2D(pt_type(_bbox->_aabb->_vmin), pt_type(_bbox->_aabb->_vmax- _bbox->_aabb->_vmin));
}


InstancePosRot::~InstancePosRot() {
	delete _bbox;
	delete _emprise;
}


void InstancePosRot::set_pos_rot_scale(const pt_type_3d & position, const quat & rotation, const pt_type_3d & scale) {
	_position= position;
	_rotation= rotation;
	_scale= scale;
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox->set_model2world(_model2world);
	_emprise->_pos= pt_type(_bbox->_aabb->_vmin);
	_emprise->_size= pt_type(_bbox->_aabb->_vmax- _bbox->_aabb->_vmin);
}


// lent, mieux vaut utiliser l'autre
void InstancePosRot::set_pos_rot_scale(const mat_4d & mat) {
	pt_type_3d skew;
	pt_type_4d perspective;
	glm::decompose(mat, _scale, _rotation, _position, skew, perspective);
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox->set_model2world(_model2world);
}


void InstancePosRot::update_dist2(pt_type_3d view_eye) {
	// glm::distance2 est lent !
	//_dist2= glm::distance2(_position, view_eye);
	_dist2= (_position.x- view_eye.x)* (_position.x- view_eye.x)+ (_position.y- view_eye.y)* (_position.y- view_eye.y)+ (_position.z- view_eye.z)* (_position.z- view_eye.z);
}


// -----------------------------------------------------------------------------------------------------
bool aabb_intersects_aabb(AABB * aabb_1, AABB * aabb_2) {
	if ((aabb_1->_vmin.x> aabb_2->_vmax.x) || (aabb_1->_vmax.x< aabb_2->_vmin.x) ||
		(aabb_1->_vmin.y> aabb_2->_vmax.y) || (aabb_1->_vmax.y< aabb_2->_vmin.y) ||
		(aabb_1->_vmin.z> aabb_2->_vmax.z) || (aabb_1->_vmax.z< aabb_2->_vmin.z)) {
		return false;
	}
	return true;
}


bool aabb_intersects_bbox(AABB * aabb, BBox * bbox) {
	for (unsigned int i=0; i<8; ++i) {
		if ((bbox->_pts[i].x> aabb->_vmin.x) && (bbox->_pts[i].x< aabb->_vmax.x) &&
			(bbox->_pts[i].y> aabb->_vmin.y) && (bbox->_pts[i].y< aabb->_vmax.y) &&
			(bbox->_pts[i].z> aabb->_vmin.z) && (bbox->_pts[i].z< aabb->_vmax.z)) {
			return true;
		}
	}
	return false;
}


// https://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20or%20Oriented%20Bounding%20Boxes.pdf
bool bbox_intersects_bbox(BBox * bbox_1, BBox * bbox_2) {
	pt_type_3d center_1= 0.5* (bbox_1->_pts[7]+ bbox_1->_pts[0]);
	pt_type_3d x_1= glm::normalize(bbox_1->_pts[1]- bbox_1->_pts[0]);
	pt_type_3d y_1= glm::normalize(bbox_1->_pts[2]- bbox_1->_pts[0]);
	pt_type_3d z_1= glm::normalize(bbox_1->_pts[4]- bbox_1->_pts[0]);
	number half_width_1= 0.5* (bbox_1->_vmax[0]- bbox_1->_vmin[0]);
	number half_height_1= 0.5* (bbox_1->_vmax[1]- bbox_1->_vmin[1]);
	number half_depth_1= 0.5* (bbox_1->_vmax[2]- bbox_1->_vmin[2]);

	pt_type_3d center_2= 0.5* (bbox_2->_pts[7]+ bbox_2->_pts[0]);
	pt_type_3d x_2= glm::normalize(bbox_2->_pts[1]- bbox_2->_pts[0]);
	pt_type_3d y_2= glm::normalize(bbox_2->_pts[2]- bbox_2->_pts[0]);
	pt_type_3d z_2= glm::normalize(bbox_2->_pts[4]- bbox_2->_pts[0]);
	number half_width_2= 0.5* (bbox_2->_vmax[0]- bbox_2->_vmin[0]);
	number half_height_2= 0.5* (bbox_2->_vmax[1]- bbox_2->_vmin[1]);
	number half_depth_2= 0.5* (bbox_2->_vmax[2]- bbox_2->_vmin[2]);

	pt_type_3d axes[15]= {
		x_1, y_1, z_1, x_2, y_2, z_2, 
		glm::cross(x_1, x_2), glm::cross(x_1, y_2), glm::cross(x_1, z_2), 
		glm::cross(y_1, x_2), glm::cross(y_1, y_2), glm::cross(y_1, z_2), 
		glm::cross(z_1, x_2), glm::cross(z_1, y_2), glm::cross(z_1, z_2)
	};

	for (unsigned i=0; i<15; ++i) {
		number a= abs(glm::dot(axes[i], center_2- center_1));
		number b= abs(half_width_1* glm::dot(axes[i], x_1))+ abs(half_height_1* glm::dot(axes[i], y_1))+ abs(half_depth_1* glm::dot(axes[i], z_1))+
				 abs(half_width_2* glm::dot(axes[i], x_2))+ abs(half_height_2* glm::dot(axes[i], y_2))+ abs(half_depth_2* glm::dot(axes[i], z_2));
		if (a> b) {
			return false;
		}
	}
	return true;
}


number aabb_distance_pt_2(AABB * aabb, const pt_type_3d & pt) {
	number dx, dy, dz;
	
	if (pt.x> aabb->_vmax.x) {
		dx= pt.x- aabb->_vmax.x;
	}
	else if (pt.x< aabb->_vmin.x) {
		dx= aabb->_vmin.x- pt.x;
	}
	else {
		dx= 0.0;
	}

	if (pt.y> aabb->_vmax.y) {
		dy= pt.y- aabb->_vmax.y;
	}
	else if (pt.y< aabb->_vmin.y) {
		dy= aabb->_vmin.y- pt.y;
	}
	else {
		dy= 0.0;
	}

	if (pt.z> aabb->_vmax.z) {
		dz= pt.z- aabb->_vmax.z;
	}
	else if (pt.z< aabb->_vmin.z) {
		dz= aabb->_vmin.z- pt.z;
	}
	else {
		dz= 0.0;
	}

	return dx* dx+ dy* dy+ dz* dz;
}


number aabb_distance_pt(AABB * aabb, const pt_type_3d & pt) {
	return sqrt(aabb_distance_pt_2(aabb, pt));
}


// cf https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
bool ray_intersects_aabb(pt_type_3d origin, pt_type_3d direction, AABB * aabb, number & t_hit) {
	direction= glm::normalize(direction);
	if (direction.x== 0.0) {
		direction.x= 1e-7;
	}
	if (direction.y== 0.0) {
		direction.y= 1e-7;
	}
	if (direction.z== 0.0) {
		direction.z= 1e-7;
	}
	pt_type_3d dirfrac(1.0/ direction.x, 1.0/ direction.y, 1.0/ direction.z);
	number t1= (aabb->_vmin.x- origin.x)* dirfrac.x;
	number t2= (aabb->_vmax.x- origin.x)* dirfrac.x;
	number t3= (aabb->_vmin.y- origin.y)* dirfrac.y;
	number t4= (aabb->_vmax.y- origin.y)* dirfrac.y;
	number t5= (aabb->_vmin.z- origin.z)* dirfrac.z;
	number t6= (aabb->_vmax.z- origin.z)* dirfrac.z;

	number tmin= max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	number tmax= min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0) {
		t_hit= tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax) {
		t_hit= tmax;
		return false;
	}

	t_hit= tmin;
	return true;
}


bool segment_intersects_aabb(const pt_type_3d & pt1, const pt_type_3d & pt2, AABB * aabb) {
	number t_hit;
	bool ray_inter= ray_intersects_aabb(pt1, pt2- pt1, aabb, t_hit);
	//cout << ray_inter << " ; " << t_hit << " ; " << glm::length(pt2- pt1) << "\n";
	if ((!ray_inter) || (t_hit> glm::length(pt2- pt1))) {
		return false;
	}
	return true;
}


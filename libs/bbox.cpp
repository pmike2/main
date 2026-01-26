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
#include "utile.h"



AABB::AABB() {
	set_vmin_vmax(pt_3d(0.0), pt_3d(0.0));
}


AABB::AABB(const pt_3d & vmin, const pt_3d & vmax) {
	set_vmin_vmax(vmin, vmax);
}


AABB::AABB(AABB_2D * aabb_2d) {
	set_vmin_vmax(pt_3d(aabb_2d->_pos.x, aabb_2d->_pos.y, -0.01), pt_3d(aabb_2d->_pos.x + aabb_2d->_size.x, aabb_2d->_pos.y + aabb_2d->_size.y, 0.01));
}


AABB::~AABB() {

}


void AABB::set_vmin_vmax(const pt_3d & vmin, const pt_3d & vmax) {
	_vmin= vmin;
	_vmax= vmax;
	
	/*_radius= abs(_vmin.x);
	if (abs(_vmax.x)> _radius) _radius= abs(_vmax.x);
	if (abs(_vmin.y)> _radius) _radius= abs(_vmin.y);
	if (abs(_vmax.y)> _radius) _radius= abs(_vmax.y);
	if (abs(_vmin.z)> _radius) _radius= abs(_vmin.z);
	if (abs(_vmax.z)> _radius) _radius= abs(_vmax.z);*/

	_radius = glm::distance(_vmin, _vmax) * 0.5;
	_base_radius = glm::distance(pt_2d(_vmin.x, _vmin.y), pt_2d(_vmax.x, _vmax.y)) * 0.5;

	_pts[0]= pt_3d(_vmin.x, _vmin.y, _vmin.z);
	_pts[1]= pt_3d(_vmax.x, _vmin.y, _vmin.z);
	_pts[2]= pt_3d(_vmin.x, _vmax.y, _vmin.z);
	_pts[3]= pt_3d(_vmax.x, _vmax.y, _vmin.z);
	_pts[4]= pt_3d(_vmin.x, _vmin.y, _vmax.z);
	_pts[5]= pt_3d(_vmax.x, _vmin.y, _vmax.z);
	_pts[6]= pt_3d(_vmin.x, _vmax.y, _vmax.z);
	_pts[7]= pt_3d(_vmax.x, _vmax.y, _vmax.z);
}


std::vector<std::vector<uint> > AABB::triangles_idxs() {
	std::vector<std::vector<uint> > idx = {
		{0, 4, 2}, {2, 4, 6}, // x-
		{1, 3, 7}, {1, 7, 5}, // x+
		{0, 1, 5}, {0, 5, 4}, // y-
		{3, 2, 6}, {3, 6, 7}, // y+
		{0, 2, 3}, {0, 3, 1}, // z-
		{5, 7, 6}, {5, 6, 4} // z+
	};
	return idx;
}


std::vector<pt_3d> AABB::segments() {
	return std::vector<pt_3d> {
		_pts[0], _pts[1], _pts[1], _pts[3], _pts[3], _pts[2], _pts[2], _pts[0], // bottom
		_pts[4], _pts[5], _pts[5], _pts[7], _pts[7], _pts[6], _pts[6], _pts[4], // top
		_pts[0], _pts[1], _pts[1], _pts[5], _pts[5], _pts[4], _pts[4], _pts[0], // left
		_pts[2], _pts[6], _pts[6], _pts[7], _pts[7], _pts[3], _pts[3], _pts[2], // right
		_pts[1], _pts[3], _pts[3], _pts[7], _pts[7], _pts[5], _pts[5], _pts[1], // front
		_pts[0], _pts[2], _pts[2], _pts[6], _pts[6], _pts[4], _pts[4], _pts[0], // back
	};
}


void AABB::translate(pt_3d v) {
	set_vmin_vmax(_vmin+ v, _vmax+ v);
}


void AABB::scale(number x) {
	pt_3d c = center();
	
	set_vmin_vmax(x* (_vmin - c) + c, x* (_vmax - c) + c);
}


pt_3d AABB::center() {
	return 0.5 * (_vmin + _vmax);
}


pt_3d AABB::bottom_center() {
	return pt_3d(0.5 * (_vmin.x + _vmax.x), 0.5 * (_vmin.y + _vmax.y), _vmin.z);
}


pt_3d AABB::size() {
	return _vmax - _vmin;
}


AABB_2D * AABB::aabb2d() {
	return new AABB_2D(pt_2d(_vmin.x, _vmin.y), pt_2d(_vmax.x - _vmin.x, _vmax.y - _vmin.y));
}


void AABB::set_z(number z) {
	set_vmin_vmax(pt_3d(_vmin.x, _vmin.y, z), pt_3d(_vmax.x, _vmax.y, _vmax.z - _vmin.z + z));
}


std::ostream & operator << (std::ostream & os, const AABB & aabb) {
	os << "vmin=" << glm::to_string(aabb._vmin) << " ; vmax=" << glm::to_string(aabb._vmax);
	return os;
}


// ------------------------------------------------------------------------------------------------------
BBox::BBox() : _vmin(pt_3d(0.0)), _vmax(pt_3d(0.0)), _radius(0.0) {
	_aabb= new AABB();
}


BBox::BBox(const pt_3d & vmin, const pt_3d & vmax, const mat_4d & model2world) : _vmin(vmin), _vmax(vmax), _model2world(model2world) {
	_aabb= new AABB();
	set_model2world(_model2world);
	update_radius();
}


BBox::BBox(AABB * aabb) : _vmin(aabb->_vmin), _vmax(aabb->_vmax), _model2world(mat_4d(1.0)) {
	_aabb= new AABB();
	set_model2world(_model2world);
	update_radius();
}


BBox::~BBox() {
	delete _aabb;
}


void BBox::update_radius() {
	_radius= abs(_vmin.x);
	if (abs(_vmax.x)> _radius) _radius= abs(_vmax.x);
	if (abs(_vmin.y)> _radius) _radius= abs(_vmin.y);
	if (abs(_vmax.y)> _radius) _radius= abs(_vmax.y);
	if (abs(_vmin.z)> _radius) _radius= abs(_vmin.z);
	if (abs(_vmax.z)> _radius) _radius= abs(_vmax.z);
}


void BBox::set_aabb(AABB * aabb) {
	_vmin = aabb->_vmin;
	_vmax = aabb->_vmax;
	_model2world = mat_4d(1.0);
	update_radius();
	set_model2world(_model2world);
}


void BBox::set_model2world(const mat_4d & model2world) {
	_model2world= model2world;

	_pts[0]= pt_3d(_model2world* pt_4d(_vmin.x, _vmin.y, _vmin.z, 1.0));
	_pts[1]= pt_3d(_model2world* pt_4d(_vmax.x, _vmin.y, _vmin.z, 1.0));
	_pts[2]= pt_3d(_model2world* pt_4d(_vmin.x, _vmax.y, _vmin.z, 1.0));
	_pts[3]= pt_3d(_model2world* pt_4d(_vmax.x, _vmax.y, _vmin.z, 1.0));
	_pts[4]= pt_3d(_model2world* pt_4d(_vmin.x, _vmin.y, _vmax.z, 1.0));
	_pts[5]= pt_3d(_model2world* pt_4d(_vmax.x, _vmin.y, _vmax.z, 1.0));
	_pts[6]= pt_3d(_model2world* pt_4d(_vmin.x, _vmax.y, _vmax.z, 1.0));
	_pts[7]= pt_3d(_model2world* pt_4d(_vmax.x, _vmax.y, _vmax.z, 1.0));

	pt_3d vmin(_pts[0]);
	pt_3d vmax(_pts[0]);
	for (uint i=1; i<8; ++i) {
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


std::vector<std::vector<uint> > BBox::triangles_idxs() {
	std::vector<std::vector<uint> > idx= {
		{0, 4, 2}, {2, 4, 6}, // x-
		{1, 3, 7}, {1, 7, 5}, // x+
		{0, 1, 5}, {0, 5, 4}, // y-
		{3, 2, 6}, {3, 6, 7}, // y+
		{0, 2, 3}, {0, 3, 1}, // z-
		{5, 7, 6}, {5, 6, 4} // z+
	};
	return idx;
}


std::vector<pt_3d> BBox::segments() {
	return std::vector<pt_3d> {
		_pts[0], _pts[1], _pts[1], _pts[3], _pts[3], _pts[2], _pts[2], _pts[0], // bottom
		_pts[4], _pts[5], _pts[5], _pts[7], _pts[7], _pts[6], _pts[6], _pts[4], // top
		_pts[0], _pts[1], _pts[1], _pts[5], _pts[5], _pts[4], _pts[4], _pts[0], // left
		_pts[2], _pts[6], _pts[6], _pts[7], _pts[7], _pts[3], _pts[3], _pts[2], // right
		_pts[1], _pts[3], _pts[3], _pts[7], _pts[7], _pts[5], _pts[5], _pts[1], // front
		_pts[0], _pts[2], _pts[2], _pts[6], _pts[6], _pts[4], _pts[4], _pts[0], // back
	};
}


BBox_2D * BBox::bbox2d() {
	number width = _vmax.y - _vmin.y;
	pt_2d pt1 = 0.5 * (pt_2d(_pts[0]) + pt_2d(_pts[2]));
	pt_2d pt2 = 0.5 * (pt_2d(_pts[1]) + pt_2d(_pts[3]));
	//std::cout << width << "\n";
	//std::cout << glm_to_string(_pts[0]) << " ; " << glm_to_string(_pts[1]) << " ; " << glm_to_string(_pts[2]) << " ; " << glm_to_string(_pts[3]) << "\n";
	//std::cout << glm_to_string(pt1) << " ; " << glm_to_string(pt2) << "\n";
	return new BBox_2D(width, pt1, pt2);
}


std::ostream & operator << (std::ostream & os, const BBox & bbox) {
	os << "aabb = " << *bbox._aabb;
	os << " ; vmin=" << glm::to_string(bbox._vmin) << " ; vmax=" << glm::to_string(bbox._vmax);
	return os;
}


// ---------------------------------------------------------------------------------------------------------------------
InstancePosRot::InstancePosRot() :
	_position(pt_3d(0.0)), _rotation(quat(1.0, 0.0, 0.0, 0.0)), _scale(pt_3d(1.0)), _model2world(mat_4d(1.0)),
	_active(false), _dist2(0.0), _selected(false)
{
	_bbox= new BBox();
	//_emprise= new AABB_2D();
}


InstancePosRot::InstancePosRot(const pt_3d & position, const quat & rotation, const pt_3d & scale) :
	_position(position), _rotation(rotation), _scale(scale), _active(false), _dist2(0.0), _selected(false)
{
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox= new BBox();
	//_emprise= new AABB_2D();
}


InstancePosRot::InstancePosRot(const pt_3d & position, const quat & rotation, const pt_3d & scale, AABB * aabb) : 
	_position(position), _rotation(rotation), _scale(scale), _active(false), _dist2(0.0), _selected(false)
{
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox= new BBox(aabb->_vmin, aabb->_vmax, _model2world);
	//_emprise= new AABB_2D(pt_2d(_bbox->_aabb->_vmin), pt_2d(_bbox->_aabb->_vmax- _bbox->_aabb->_vmin));
}


InstancePosRot::~InstancePosRot() {
	delete _bbox;
	//delete _emprise;
}


void InstancePosRot::set_pos_rot_scale(const pt_3d & position, const quat & rotation, const pt_3d & scale) {
	_position= position;
	_rotation= rotation;
	_scale= scale;
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox->set_model2world(_model2world);
	//_emprise->_pos= pt_2d(_bbox->_aabb->_vmin);
	//_emprise->_size= pt_2d(_bbox->_aabb->_vmax- _bbox->_aabb->_vmin);
}


// lent, mieux vaut utiliser l'autre
void InstancePosRot::set_pos_rot_scale(const mat_4d & mat) {
	pt_3d skew;
	pt_4d perspective;
	glm::decompose(mat, _scale, _rotation, _position, skew, perspective);
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox->set_model2world(_model2world);
}


void InstancePosRot::update_dist2(pt_3d view_eye) {
	// glm::distance2 est lent !
	//_dist2= glm::distance2(_position, view_eye);
	_dist2= (_position.x- view_eye.x)* (_position.x- view_eye.x)+ (_position.y- view_eye.y)* (_position.y- view_eye.y)+ (_position.z- view_eye.z)* (_position.z- view_eye.z);
}


